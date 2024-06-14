#include <cairo/cairo.h>
#include <cairo-svg.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <math.h>

#define FRAMERATE 30
const float frame_duration = 1000000 / FRAMERATE;

double gravity = 9.81;

struct Ball {	
	double x;
	double y;
	double vx;
	double vy;
	double initialVelocity;
	double angle; 
	int collision;
};

struct ballArray {
	struct Ball ball[10];
	int size;
	int count;
};

void initBall(struct Ball *ball)
{
	ball->x = 0;
	ball->y = 600;
	ball->initialVelocity = 20;
	ball->angle = 45;  
	ball->collision = 0;
	ball->vx = ball->initialVelocity * cos(ball->angle * M_PI / 180);
	ball->vy = -(ball->initialVelocity * sin(ball->angle * M_PI / 180));
}

int addBall(struct ballArray *ballArray)
{
	if( ballArray->count < ballArray->size) {
		ballArray->count += 1;
	} else { 
		return 1; 
	}
	return 0;
}

int removeBall(struct ballArray *ballArray)
{
	if( ballArray->count == 0) {
		return 1;
	} else {
		ballArray->count -= 1;
	}
	return 0;
}

int initBallArray(struct ballArray *ba)
{
	for(int i = 0; i < ba->size; i++){
		ba->ball[i].x = 0;
		ba->ball[i].y = 600;
		ba->ball[i].initialVelocity = 20;
		ba->ball[i].angle = 45+i;  
		ba->ball[i].collision = 0;
		ba->ball[i].vx = ba->ball[i].initialVelocity * cos(ba->ball[i].angle * M_PI / 180);
		ba->ball[i].vy = -(ba->ball[i].initialVelocity * sin(ba->ball[i].angle * M_PI / 180));
	}
	return 0;
}


int main() {
    Display *display;
    Window root;
    Window window;
    XEvent event;
    int screen;

    struct Ball ballTest = { 0, 600, 0, 0, 20, 75, 0};
    struct ballArray *ba;
	//ba = (struct ballArray *)malloc(sizeof(ba));
	//ba->size = 10;
	//ba->count = 0;

	initBall(&ballTest);
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
    window = XCreateSimpleWindow(display, root, 10, 10, 800, 600, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));

    // Select input events
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    // Map (show) the window
    XMapWindow(display, window);

    // Create a Cairo surface and context for the Xlib window
    cairo_surface_t *surface = cairo_xlib_surface_create(display, window,
                                                         DefaultVisual(display, screen), 800, 600);
    cairo_t *cr = cairo_create(surface);

    // Main event loop
    while (1) {
        // Check for events
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
				KeySym key = XLookupKeysym(&event.xkey,0);
				switch(key) {
					case XK_space:
						initBall(&ballTest);
						break;
					case XK_Escape:
				   		goto seeyalater; 
						break;
					case XK_Up:
						addBall(ba);
					case XK_Down:
						removeBall(ba);
						break;
					default:
						continue;
            	}
        	}
		}

		// stop rolling
		if (ballTest.y == 600 && ballTest.x != 0 ) { initBall(&ballTest); } 

        // Clear the background
        cairo_set_source_rgb(cr, 1, 1, 1); // White
        cairo_paint(cr);

        // Draw the red circle
        cairo_set_source_rgb(cr, 1, 0, 0); // Red
        cairo_arc(cr, ballTest.x, ballTest.y, 5, 0, 2 * 3.14159);
        cairo_fill(cr);

		// wait for 1/60th of a second
        usleep(frame_duration);

		// print information to the screen about ballTest
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, 20);
		char posText[100];
		snprintf( posText, sizeof(posText), "Ball Position: (%.2f,     %.2f    )", ballTest.x, 600 - ballTest.y );
		cairo_move_to(cr, 400, 100);
		cairo_show_text(cr,  posText);

		char velText[100];
		snprintf( velText, sizeof(velText), "Ball Velocity: (%.2f m/s, %.2f m/s)", ballTest.vx, - ballTest.vy );
		cairo_move_to(cr, 400, 125);
		cairo_show_text(cr,  velText);

		char grvText[100];
		snprintf( grvText, sizeof(grvText), "Gravity: %.2f m/s^2", gravity);
		cairo_move_to(cr, 400, 150);
		cairo_show_text(cr,  grvText);

		char ballArrText[100];
		snprintf( ballArrText, sizeof(ballArrText), "Amount of balls in ball array: %d", ba->count);
		cairo_move_to(cr, 400, 175);
		cairo_show_text(cr,  ballArrText);

        // Flush the drawing operations
        cairo_surface_flush(surface);
        XFlush(display);

		// gravity and other forces
		//ballTest.vy += gravity / (FRAMERATE * FRAMERATE);
		ballTest.vy += gravity / 10;

        // Update the position of the ballTest 
		ballTest.x += ballTest.vx;
		ballTest.y += ballTest.vy;
		
		// bounds
        if (ballTest.x > 800 ) { ballTest.x = 800; }
        if (ballTest.x < 0   ) { ballTest.x = 0;   }
		if (ballTest.y > 600 ) { ballTest.y = 600; }
		if (ballTest.y < 0   ) { ballTest.y = 0;   }

	}
seeyalater:	
	// Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);
	free(ba);

    return 0;
}

