#include <X11/keysym.h>

enum demo_mode {
	BALL_COLLISIONS = 0,
	SPRING,
	PENDULUM
};

void poll_x_event(XEvent event);

int keyboard_get_demo_mode();


