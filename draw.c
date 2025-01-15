#include "draw.h"
#include <stdio.h>

//struct Draw *draw_init(cairo_surface_t *surface){
cairo_t *draw_init(cairo_surface_t *surface){
	//struct Draw *draw;
	cairo_t *cr;
    cr = cairo_create(surface);
	printf("created surface in draw_init\n");

	// Move origin to bottom left corner of screen!
	cairo_translate(cr, 0, SCREEN_HEIGHT);
	cairo_scale(cr, 1, -1); // this makes the text mirrored too :$
	
	return cr;
}
