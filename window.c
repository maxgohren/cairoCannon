#include <stdlib.h>
#include <stdio.h>

#include "window.h"

struct Window *window_init(){
	//Display *display;
	struct Window* window = (struct Window*)malloc(sizeof(struct Window));
    if (window == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Open connection to the X server
    window->display = XOpenDisplay(NULL);
    if (window->display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

	//init_display(display, root, window, event, screen)
    window->screen = DefaultScreen(window->display);
    window->root = RootWindow(window->display, window->screen);

    // Create a simple window
    window->window = XCreateSimpleWindow(window->display, window->root, 1000, 10, SCREEN_WIDTH, SCREEN_HEIGHT, 1,
                                 BlackPixel(window->display, window->screen), WhitePixel(window->display, window->screen));

    // Select input events
    XSelectInput(window->display, window->window, ExposureMask | KeyPressMask);

    // Map (show) the window
    XMapWindow(window->display, window->window);

    // Create a Cairo surface and context for the Xlib window
    window->surface = cairo_xlib_surface_create(window->display, window->window,
                                                         DefaultVisual(window->display, window->screen), SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Return initialized Window struct
	return window;
}
