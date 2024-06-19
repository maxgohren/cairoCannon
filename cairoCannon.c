#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <math.h>
#include <time.h>
#include <assert.h>

#define FRAMERATE 60
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define MAX_BALLS 50
#define STARTING_BALL_COUNT 2
#define BALL_RADIUS 50
#define BALL_DIAMETER 1.75*BALL_RADIUS

const int frame_duration = 1000000 / FRAMERATE;
double gravity = -9.81;
double initialVelocity;
float pendulum_angle;
int i, j;
struct Ball tmp;

enum demo_mode {
	BALL_COLLISIONS = 0,
	SPRING,
	PENDULUM
}demo_mode = 0;

int colours[10][3] = {  {255,  0,255} ,  // magenta
			            {255,  0,  0} ,  // red
				        {255,165,  0} ,  // orange
				        {0  ,128,  0} ,  // green
				        {128,  0,128} ,  // purple
				        {255,255,  0} ,  // yellow
				        {  0,255,255} ,  // aqua
				        {  0,  0,128} ,  // navy
				        {  0,  0,  0} ,  // black
				        {255,155,  0} }; // raspberry


struct Spring {
    float x_1;
    float y_1;
    float x_2;
    float y_2;
    float length;
    float angle;  // RADIANS
    float k;      // spring constant k
}spring, pendulum;

struct Ball {	
	volatile float x;
	volatile float y;
	double vx;
	double vy;
	double angle; 
	float r;
	float g;
	float b;
    int m;
};

struct ballArray {
	struct Ball ball[MAX_BALLS];
	int size;
	int count;
};

int addBall(struct ballArray *ballArray)
{
	if( ballArray->count < ballArray->size) {
		ballArray->count += 1;
	} else { 
		return 1; 
	}
	printf("adding ball to array: count = %d\n", ballArray->count);
	return 0;
}

int removeBall(struct ballArray *ballArray)
{
	if( ballArray->count == 0) {
		return 1;
	} else {
		ballArray->count -= 1;
	}
	printf("removing ball from array: count = %d\n", ballArray->count);
	return 0;
}

int initBallArray(struct ballArray *ba)
{
	srand(time(NULL));
	for(int i = 0; i < ba->size; i++){
		float rand_c = (float)rand() / RAND_MAX;
		int rand_col = rand() % 10;
		int rand_x = rand() % (SCREEN_WIDTH - BALL_RADIUS*2) + BALL_RADIUS;
		int rand_y = rand() % (SCREEN_WIDTH - BALL_RADIUS*2) + BALL_RADIUS;
		int rand_vel = (rand() % 10);
		int rand_ang = (rand() % 360);
		ba->ball[i].x = rand_x;
		ba->ball[i].y = rand_y;
		ba->ball[i].angle = rand_ang;  
		ba->ball[i].vx = rand_vel * cos(ba->ball[i].angle * M_PI / 180);
		ba->ball[i].vy = rand_vel * sin(ba->ball[i].angle * M_PI / 180);
		ba->ball[i].r = (float)colours[rand_col][0] / 255;
		ba->ball[i].g = (float)colours[rand_col][1] / 255;
		ba->ball[i].b = (float)colours[rand_col][2] / 255;
		ba->ball[i].m = 1;
	}
	return 0;
}

void drawScene(cairo_t *cr, struct ballArray *ba, struct Spring *sp, struct Spring *pd)

{
    // Clear the background
    cairo_set_source_rgb(cr, 1, 1, 1); // White
    cairo_paint(cr);

	// Draw all balls in the ball array
	for(int i = 0; i < ba->count; i++){
		//TODO set random colour for each ball each init or draw
    	cairo_set_source_rgb(cr, ba->ball[i].r, ba->ball[i].g, ba->ball[i].b);
    	cairo_arc(cr, ba->ball[i].x, ba->ball[i].y, BALL_RADIUS, 0, 2 * 3.14159);
    	cairo_fill(cr);
	}

    // draw springs
    if(demo_mode == SPRING){
        cairo_save(cr);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_translate(cr, sp->x_1, sp->y_1);
        cairo_rotate(cr, sp->angle + M_PI_4 + 0.01); // starts sticking straight up, offset ccw by pi/4 to have pi/2 rad of rotation on screen
        cairo_rectangle(cr, 0, 0, 10, sp->length - BALL_RADIUS);
        cairo_fill(cr);
        cairo_restore(cr); // stop from rotating entire screen
    }

    // draw springs
    if(demo_mode == PENDULUM){
        static int count = 0;
        cairo_save(cr);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_translate(cr, pd->x_1, pd->y_1); 
        // +PI/4 to bring into 2nd and 3rd quadrant
        // + 0.01 for some manual massaging for visuals
        cairo_rotate(cr, pd->angle + M_PI_4);
        cairo_rectangle(cr, 0, 0, 10, pd->length - BALL_RADIUS);
        cairo_fill(cr);
        cairo_restore(cr); // stop from rotating entire screen
    }
}

void updatePhysics(struct ballArray *ba, struct Spring *sp, struct Spring *pd)
{   
    // ball handling
    for(i = 0; i < ba->count; i++) { 
	    // gravity
	    ba->ball[i].vy += -0.05;
        
        // pendulum physics 
        //ba->ball[0].vx += sp->length * cos(pendulum_angle); // * gravity as well
        //ba->ball[0].vy += sp->length * sin(pendulum_angle); // * gravity as well

	    // velocity
	    ba->ball[i].x += ba->ball[i].vx;
	    ba->ball[i].y += ba->ball[i].vy;

	    // wall reflections
 	    if (ba->ball[i].x >= SCREEN_WIDTH - BALL_RADIUS   || ba->ball[i].x <= 0 + BALL_RADIUS ) { ba->ball[i].vx *= -1; }
	    if (ba->ball[i].y >= SCREEN_HEIGHT - BALL_RADIUS  || ba->ball[i].y <= 0 + BALL_RADIUS) { ba->ball[i].vy *= -1; }

         // collision handling, uniform masses
	    for(j = 0; j < ba->count; j++){
	    	if( i == j ) {
	    		continue;
	    	}
             int diffx = SCREEN_WIDTH;  // max diff
             int diffy = SCREEN_HEIGHT; // max diff
             diffy = abs((int)ba->ball[i].x - (int)ba->ball[j].x); 
             diffx = abs((int)ba->ball[i].y - (int)ba->ball[j].y);
	    	if( diffx < BALL_DIAMETER && diffy < BALL_DIAMETER ){
	    		// swap before and after
	    		tmp.vx = ba->ball[i].vx;
	    		tmp.vy = ba->ball[i].vy;
	    		ba->ball[i].vx = ba->ball[j].vx;
	    		ba->ball[i].vy = ba->ball[j].vy;
	    		ba->ball[j].vx = tmp.vx;
	    		ba->ball[j].vy = tmp.vy;
            }
        }
    }

    // spring handling
    if(demo_mode == SPRING){
        if(ba->count != 1){
            ba->count = 1;
        }
        sp->x_1 = (float)SCREEN_WIDTH / 2;
        sp->y_1 = SCREEN_HEIGHT;
        sp->x_2 = ba->ball[0].x;
        sp->y_2 = ba->ball[0].y;

        float sp_length_x = fabs(sp->x_1 - sp->x_2); 
        float sp_length_y = fabs(sp->y_1 - sp->y_2); 
        sp->length = sqrt( pow(sp_length_x, 2) + pow(sp_length_y, 2) );
        //quadrant handling for cos
        if(ba->ball[0].x < SCREEN_WIDTH/2){
            sp->angle = acos(sp_length_x / sp->length) + M_PI_4;
        } else { 
            sp->angle = acos(sp_length_y / sp->length) + 3 * M_PI_4;
        }

        // spring constant k applies force F = kx where x is parallel distance along spring
    }

    // pendulum, constrained motion
    if(demo_mode == PENDULUM){
        if(ba->count != 1){
            ba->count = 1;
        }
        pd->x_1 = (float)SCREEN_WIDTH / 2;
        pd->y_1 = SCREEN_HEIGHT;
        pd->x_2 = ba->ball[0].x;
        pd->y_2 = ba->ball[0].y;


        float pd_length_x = fabs(pd->x_1 - pd->x_2); 
        float pd_length_y = fabs(pd->y_1 - pd->y_2); 
        pd->length = sqrt( pow(pd_length_x, 2) + pow(pd_length_y, 2) );

        //quadrant handling for cos
        if(ba->ball[0].x < (float)SCREEN_WIDTH/2){
            pd->angle = acos(pd_length_x / pd->length) + M_PI_4;
            pendulum_angle = pd->angle - 3 * M_PI_4;
            printf("ball vx %.02f vy %.02f\n", 0.01 * pd->length * cos(pendulum_angle), 0.01 * pd->length * sin(pendulum_angle) );
            ba->ball[0].vx = 0.01 * pd->length * cos(pendulum_angle);
            ba->ball[0].vy = 0.01 * pd->length * sin(pendulum_angle);
        } else if(ba->ball[0].x > (float)SCREEN_WIDTH/2){
            pd->angle = acos(pd_length_y / pd->length) + 3 * M_PI_4;
            pendulum_angle = pd->angle - 3 * M_PI_4;
            printf("ball vx %.02f vy %.02f\n", 0.01 * pd->length * -cos(pendulum_angle), 0.01 * pd->length * -sin(pendulum_angle) );
            ba->ball[0].vx = 0.01 * -pd->length * cos(pendulum_angle);
            ba->ball[0].vy = 0.01 * -pd->length * sin(pendulum_angle);
        } else {
            pd->angle = acos(pd_length_y / pd->length) + 3 * M_PI_4;
            pendulum_angle = pd->angle - 3 * M_PI_4;
            printf("ball vx 0, vy 0\n");
            ba->ball[0].vy = 0;
        }
    }
    
}


int main() {
    Display *display;
    Window root;
    Window window;
    XEvent event;
    int screen;

    //init physics objects for all demos
    struct Spring *spring;
    spring = (struct Spring *)malloc(sizeof(struct Spring));

    struct Spring *pendulum;
    pendulum = (struct Spring *)malloc(sizeof(struct Spring));

	assert(MAX_BALLS >= STARTING_BALL_COUNT);
    struct ballArray *ba;
	ba = (struct ballArray*)malloc(sizeof(struct ballArray));
    ba->size = MAX_BALLS;
	ba->count = STARTING_BALL_COUNT;

	initBallArray(ba);

    // Open connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    // Create a simple window
    window = XCreateSimpleWindow(display, root, 1000, 10, SCREEN_WIDTH, SCREEN_HEIGHT, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));

    // Select input events
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    // Map (show) the window
    XMapWindow(display, window);

    // Create a Cairo surface and context for the Xlib window
    cairo_surface_t *surface = cairo_xlib_surface_create(display, window,
                                                         DefaultVisual(display, screen), SCREEN_WIDTH, SCREEN_HEIGHT);
    cairo_t *cr = cairo_create(surface);

	// Move origin to bottom left corner of screen!
	cairo_translate(cr, 0, SCREEN_HEIGHT);
	cairo_scale(cr, 1, -1); // this makes the text mirrored too :$

    // Main event loop
    while (1) {
        // Check for events
	   while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
				KeySym key = XLookupKeysym(&event.xkey,0);
				switch(key) {
					case XK_space:
						initBallArray(ba);
						break;
					case XK_Escape:
				   		goto cleanup; 
						break;
					case XK_Up:
						addBall(ba);
                        if (spring->angle < 2 * M_PI) spring->angle+= 0.0174533;
						break;
					case XK_Down:
						removeBall(ba);
                        if (spring->angle > 0) spring->angle-= 0.0174533;
						break;
                    case XK_h:
                        ba->ball[0].x-=10;
                        break;
                    case XK_j:
                        ba->ball[0].y-=10;
                        break;
                    case XK_k:
                        ba->ball[0].y+=10;
                        break;
                    case XK_l:
                        ba->ball[0].x+=10;
                        break;
                    case XK_1:
						demo_mode = BALL_COLLISIONS;
                        printf("now starting ball collisions demo\n");
						break;
                    case XK_2:
						demo_mode = SPRING;
                        ba->count = 1;
                        printf("now starting spring demo\n");
						break;
                    case XK_3:
						demo_mode = PENDULUM;
                        printf("now starting pendulum demo\n");
						break;
					default:
						continue;
            	}
        	}
		}

		drawScene(cr, ba, spring, pendulum);

        updatePhysics(ba, spring, pendulum); // make diff update for each demo so we dont have to pass lots of stuff

        usleep(frame_duration);

        cairo_surface_flush(surface);
        XFlush(display);

        
    }
		
cleanup:
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);
	free(ba);
    return 0;
}

