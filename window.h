#include <X11/Xlib.h>
#include <cairo/cairo-xlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

struct Window {
    Display *display;
    Window root;
    Window window;
    XEvent event;
    int screen;
	cairo_surface_t *surface;
};

struct Window *window_init();
