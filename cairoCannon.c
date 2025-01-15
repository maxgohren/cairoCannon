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

#include "cairoCannon.h"

const int frame_duration = 1000000 / FRAMERATE;
double gravity = -9.81 / FRAMERATE;
double initialVelocity;
int i, j;
struct ballArray *ba;
int exit_flag = 0;
struct Spring *spring;
struct Pendulum *pd;

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

static void swap_ball(struct Ball *ball1, struct Ball *ball2);

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

void drawScene(cairo_t *cr, struct ballArray *ba, struct Spring *sp, struct Pendulum *pd)
{
    // Clear the background
    cairo_set_source_rgb(cr, 1, 1, 1); // White
    cairo_paint(cr);


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

    // draw pendulum
    if(demo_mode == PENDULUM){
        cairo_save(cr);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_translate(cr, pd->x_1, pd->y_1); 
        // +PI/4 to bring into 2nd and 3rd quadrant
        cairo_rotate(cr, pd->angle);
        cairo_rectangle(cr, 0, 0, 10, pd->length /* - BALL_RADIUS*/);
        cairo_fill(cr);
        cairo_restore(cr); // stop from rotating entire screen
    }

	// Draw all balls in the ball array
	for(int i = 0; i < ba->count; i++){
		//TODO set random colour for each ball each init or draw
    	cairo_set_source_rgb(cr, ba->ball[i].r, ba->ball[i].g, ba->ball[i].b);
    	cairo_arc(cr, ba->ball[i].x, ba->ball[i].y, BALL_RADIUS, 0, 2 * 3.14159);
    	cairo_fill(cr);
	}
}

void updatePhysics(struct ballArray *ba, struct Spring *sp, struct Pendulum *pd)
{   
    // ball handling
    for(i = 0; i < ba->count; i++) { 
	    // gravity
	    //ba->ball[i].vy += gravity;

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
			// TODO optimization: only diff balls that are within a range of eachother to reduce comparisons
             int diffx = SCREEN_WIDTH;  // max diff
             int diffy = SCREEN_HEIGHT; // max diff
             diffy = abs((int)ba->ball[i].x - (int)ba->ball[j].x); 
             diffx = abs((int)ba->ball[i].y - (int)ba->ball[j].y);
	    	if( diffx < BALL_DIAMETER && diffy < BALL_DIAMETER ){
				swap_ball(&ba->ball[i],  &ba->ball[j]);
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

		// update ball position by converting polar coodinates to cartesian
		ba->ball[0].x = pd->x_1 - pd->length * sin(pd->angle);
		ba->ball[0].y = pd->y_1 + pd->length * cos(pd->angle);

		// pendulum_update_physics(pd, ba);
		pd->accel = (-1 * gravity / pd->length) * sin(pd->angle);
		pd->vel += pd->accel;
		pd->vel *= 0.995; // damping
		pd->angle += pd->vel; //update the angle with the angular velocity
		printf("accel:%f, vel:%f, angle:%f, pos:%f,%f\n", pd->accel, pd->vel, pd->angle, ba->ball[0].x, ba->ball[0].y);
	}
}



int main() {

    //init physics objects for all demos
    spring = (struct Spring *)malloc(sizeof(struct Spring));

    pd = (struct Pendulum *)malloc(sizeof(struct Spring));

	assert(MAX_BALLS >= STARTING_BALL_COUNT);
	ba = (struct ballArray*)malloc(sizeof(struct ballArray));
    ba->size = MAX_BALLS;
	ba->count = STARTING_BALL_COUNT;

	initBallArray(ba);

	// init_pendulum(pd, ba);
	pd->length = 200;
	pd->angle = M_PI_2;
	pd->x_1 = (float)SCREEN_WIDTH / 2;
	pd->y_1 = SCREEN_HEIGHT; 

    Display *display;
    Window root;
    Window window;
    XEvent event;
    int screen;

    // Open connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

	//init_display(display, root, window, event, screen)
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
    while (!exit_flag) {
        // Check for events
	   while (XPending(display)) {
            XNextEvent(display, &event);
			poll_x_event(event);
		}
		drawScene(cr, ba, spring, pd);
		updatePhysics(ba, spring, pd); // make diff update for each demo so we dont have to pass lots of stuff
		usleep(frame_duration);
		cairo_surface_flush(surface);
		XFlush(display);
   	}

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);
	free(ba);
    return 0;
}



void poll_x_event(XEvent event) 
{
	if (event.type == KeyPress) {
		KeySym key = XLookupKeysym(&event.xkey,0);
		switch(key) {
			case XK_space:
				initBallArray(ba);
				break;
			case XK_Escape: 
				exit_flag = 1;
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
				break;
		}
	}
}

static void swap_ball(struct Ball *ball1, struct Ball *ball2)
{
	struct Ball tmp;
	tmp.vx = ball1->vx;
	tmp.vy = ball1->vy;
	ball1->vx = ball2->vx;
	ball1->vy = ball2->vy;
	ball2->vx = tmp.vx;
	ball2->vy = tmp.vy;
}
