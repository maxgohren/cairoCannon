#include <cairo/cairo.h>
#include <cairo-svg.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <math.h>

#define FRAMERATE 20
const float frame_duration = 1000000 / FRAMERATE;

double gravity = 9.81;

struct Ball {	
	double x;
	double y;
	double prevX;
	double prevY;
	double vx;
	double vy;
	double initialVelocity;
	double angle; 
	int collision;
};

struct Ball ball = {
					0,  // x
					600,  // y
					0,  // prevX
					0,  // prevY
					0,  // x velocity 
					0,  // y velocity
					20,  // init. velocity m/s
					75, // angle in degrees
					0};


int main() {
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

	// calculate ball velocities
	ball.vx = ball.initialVelocity * cos(ball.angle * M_PI / 180);
	ball.vy = -(ball.initialVelocity * sin(ball.angle * M_PI / 180));

    // Main event loop
    while (1) {
        // Check for events
pending:
        while (XPending(display)) {
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
				KeySym key = XLookupKeysym(&event.xkey,0);
				if(key != XK_space) {
					// exit
				   	goto cleanup; 
				} else {
					// reset
					ball.x = 0;
					ball.y = 600;
					ball.vx = ball.initialVelocity * cos(ball.angle * M_PI / 180);
					ball.vy = -(ball.initialVelocity * sin(ball.angle * M_PI / 180));
					continue;
            	}
        	}
		}
		// stop rolling
		if (ball.y == 600 && ball.x != 0 ) { ball.vx = 0; goto pending; } 

        // Clear the background
        cairo_set_source_rgb(cr, 1, 1, 1); // White
        cairo_paint(cr);

        // Draw the red circle
        cairo_set_source_rgb(cr, 1, 0, 0); // Red
        cairo_arc(cr, ball.x, ball.y, 5, 0, 2 * 3.14159);
        cairo_fill(cr);

		// wait for 1/60th of a second
        usleep(frame_duration);

		// print information to the screen about ball
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, 20);
		char posText[100];
		char velText[100];
		char grvText[100];
		snprintf( posText, sizeof(posText), "Ball Position: (%.2f,     %.2f    )", ball.x, 600 - ball.y );
		snprintf( velText, sizeof(velText), "Ball Velocity: (%.2f m/s, %.2f m/s)", ball.vx, - ball.vy );
		snprintf( grvText, sizeof(grvText), "Gravity: %.2f m/s^2", gravity);
		cairo_move_to(cr, 400, 100);
		cairo_show_text(cr,  posText);
		cairo_move_to(cr, 400, 125);
		cairo_show_text(cr,  velText);
		cairo_move_to(cr, 400, 150);
		cairo_show_text(cr,  grvText);

        // Flush the drawing operations
        cairo_surface_flush(surface);
        XFlush(display);

		// gravity and other forces
		//ball.vy += gravity / (FRAMERATE * FRAMERATE);
		ball.vy += gravity / 10;

        // Update the position of the ball 
		ball.x += ball.vx;
		ball.y += ball.vy;
		
		// bounds
        if (ball.x > 800 ) { ball.x = 800; }
        if (ball.x < 0   ) { ball.x = 0;   }
		if (ball.y > 600 ) { ball.y = 600; }
		if (ball.y < 0   ) { ball.y = 0;   }


    }

cleanup:
    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);

    return 0;
}

