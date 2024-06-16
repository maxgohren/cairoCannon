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

const int frame_duration = 1000000 / FRAMERATE;
double gravity = -9.81;
double initialVelocity;

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

struct Ball {	
	volatile float x;
	volatile float y;
	double vx;
	double vy;
	double angle; 
	float r;
	float g;
	float b;
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
	}
	return 0;
}

void drawScene(cairo_t *cr, struct ballArray *ba)
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

	/*	
	// print information to the screen about balls
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 20);

	char posText[100];
	snprintf( posText, sizeof(posText), "Ball Position: (%.2f,%.2f)", ballArray->ball[0].x, SCREEN_HEIGHT - ballArray->ball[0].y );
	cairo_move_to(cr, 0, 100);
	cairo_show_text(cr,  posText);

	char velText[100];
	snprintf( velText, sizeof(velText), "Ball Velocity: (%.2f m/s, %.2f m/s)", ballArray->ball[0].vx, - ballArray->ball[0].vy );
	cairo_move_to(cr, 0, -125);
	cairo_show_text(cr,  velText);

	char grvText[100];
	snprintf( grvText, sizeof(grvText), "Gravity: %.2f m/s^2", gravity);
	cairo_move_to(cr, 0, -150);
	cairo_show_text(cr,  grvText);

	char ballArrText[100];
	snprintf( ballArrText, sizeof(ballArrText), "Amount of balls in ball array: %d", ballArray->count);
	cairo_move_to(cr, 0, -175);
	cairo_show_text(cr,  ballArrText);
	*/
}



int main() {
    Display *display;
    Window root;
    Window window;
    XEvent event;
    int screen;
	int i, j;
    struct Ball tmp;

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
				   		goto seeyalater; 
						break;
					case XK_Up:
						addBall(ba);
						break;
					case XK_Down:
						removeBall(ba);
						break;
					default:
						continue;
            	}
        	}
		}

		// draw the scene
		drawScene(cr, ba);

		// wait for 1/60th of a second
        usleep(frame_duration);

        // Flush the drawing operations
        cairo_surface_flush(surface);
        XFlush(display);

		// updatePhysics()
		for(i = 0; i < ba->count; i++) {

			// gravity
			ba->ball[i].vy += -0.05;

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
				if( diffx < BALL_RADIUS && diffy < BALL_RADIUS ) {
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
    }
		
seeyalater:	// Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);
	free(ba);
    return 0;
}

