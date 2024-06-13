#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep

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

    int x = 0; // Initial x position of the rectangle
    int y = 100; // y position of the rectangle
    int width = 200; // Width of the rectangle
    int height = 200; // Height of the rectangle

    // Main event loop
    while (1) {
        // Check for events
        while (XPending(display)) {
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                // Exit the loop on key press
                goto cleanup;
            }
        }

        // Clear the background
        cairo_set_source_rgb(cr, 1, 1, 1); // White
        cairo_paint(cr);

        // Draw the red rectangle
        cairo_set_source_rgb(cr, 1, 0, 0); // Red
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);

        // Draw some text
        cairo_set_source_rgb(cr, 0, 0, 0); // Black
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, 150, 150);
        cairo_show_text(cr, "Hello, Cairo!");

        // Flush the drawing operations
        cairo_surface_flush(surface);
        XFlush(display);

        // Update the position of the rectangle
        x += 1; // Move the rectangle to the right
		width += 1;
		if (height > 10) {
			height -= 1;
		}
        if (x > 800) {
            x = -width; // Reset the position if it goes off screen
        }

        // Wait for 0.1 seconds
        usleep(16667);
    }

cleanup:
    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XCloseDisplay(display);

    return 0;
}

