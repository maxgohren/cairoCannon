#include <X11/Xlib.h>
#include <stdio.h>

#include "keyboard.h"
#include "physics.h"

extern int exit_flag;

int demo_mode = 0;

void poll_x_event(XEvent event) 
{
	if (event.type == KeyPress) {
		KeySym key = XLookupKeysym(&event.xkey,0);
		switch(key) {
			case XK_space:
				physics_init_ball_array();
				break;
			case XK_Escape: 
				exit_flag = 1;
				break;
			case XK_Up:
				physics_add_ball();
				//if (spring->angle < 2 * M_PI) spring->angle+= 0.0174533;
				break;
			case XK_Down:
				physics_remove_ball();
				//if (spring->angle > 0) spring->angle-= 0.0174533;
				break;
			case XK_h:
				physics_move_ball_left();
				break;
			case XK_j:
				physics_move_ball_down();
				break;
			case XK_k:
				physics_move_ball_up();
				break;
			case XK_l:
				physics_move_ball_right();
				break;
			case XK_1:
				demo_mode = 0;
				printf("now starting ball collisions demo\n");
				break;
			case XK_2:
				demo_mode = 1;
				printf("now starting spring demo\n");
				break;
			case XK_3:
				demo_mode = 2;
				printf("now starting pendulum demo\n");
				break;
			default:
				break;
		}
	}
}

int keyboard_get_demo_mode(){
	return demo_mode;
}
