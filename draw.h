#include <cairo/cairo.h>

#define SCREEN_HEIGHT 600

struct Draw {
	cairo_t *cr;
	cairo_surface_t *surface;
};

cairo_t *draw_init();

