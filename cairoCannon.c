#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <math.h>
#include <time.h>
#include <assert.h>

#include "cairoCannon.h"
#include "window.h"
#include "keyboard.h"
#include "draw.h"
#include "physics.h"

int exit_flag = 0;

void drawScene(cairo_t *cr, struct ballArray *ba, struct Spring *sp, struct Pendulum *pd)
{
	int demo_mode = keyboard_get_demo_mode();

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



int main() 
{
	// Init window and draw
	struct Window *window = window_init();
	cairo_t *cr = draw_init(window->surface);

    // Init physics objects
	struct ballArray *ba = physics_init_ball_array();
	struct Pendulum *pd = physics_init_pendulum();
	struct Spring *sp = physics_init_spring();

    // Main event loop
    while (!exit_flag) {
        // Check for events
	   while (XPending(window->display)) {
            XNextEvent(window->display, &window->event);
			poll_x_event(window->event);
		}
		// physics update
		physics_update(); // make diff update for each demo so we dont have to pass lots of stuff
		// draw update
		drawScene(cr, ba, sp, pd);
		cairo_surface_flush(window->surface);
		// Window update
		XFlush(window->display);
   	}

	// draw_clean
    cairo_destroy(cr);
    cairo_surface_destroy(window->surface);
	// display_clean
    XCloseDisplay(window->display);
	// free physics objects
	physics_cleanup(ba, pd, sp);
    return 0;
}
