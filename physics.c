#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <math.h>
#include <time.h>
#include <assert.h>

#include "physics.h"
#include "window.h"
#include "keyboard.h"

const int frame_duration = 1000000 / FRAMERATE;
double gravity = -9.81 / FRAMERATE;
double initialVelocity;
int i, j;

struct ballArray *ba;
struct Spring *sp;
struct Pendulum *pd;


int colours[10][3] = {  {255,  0,255} ,  // magenta
			            {255,  0,  0} ,  // red
				        {255,165,  0} ,  // orange
				        {0  ,128,  0} ,  // green
				        {128,  0,128} ,  // purple
				        {255,255,  0} ,  // yellow
				        {  0,255,255} ,  // aqua
				        {  0,  0,128} ,  // navy
				        {  0,  0,  0} ,  // black
				        {255,155,  0} }; // raspberry

// TODO make static
void swap_ball(struct Ball *ball1, struct Ball *ball2)
{
	struct Ball tmp;
	tmp.vx = ball1->vx;
	tmp.vy = ball1->vy;
	ball1->vx = ball2->vx;
	ball1->vy = ball2->vy;
	ball2->vx = tmp.vx;
	ball2->vy = tmp.vy;
}

struct ballArray *physics_init_ball_array()
{
	assert(MAX_BALLS >= STARTING_BALL_COUNT);
	ba = (struct ballArray*)malloc(sizeof(struct ballArray));
    ba->size = MAX_BALLS;
	ba->count = STARTING_BALL_COUNT;

	srand(time(NULL));
	for(int i = 0; i < ba->size; i++){
		float rand_c = (float)rand() / RAND_MAX;
		int rand_col = rand() % 10;
		int rand_x = rand() % (SCREEN_WIDTH - BALL_RADIUS*2) + BALL_RADIUS;
		int rand_y = rand() % (SCREEN_WIDTH - BALL_RADIUS*2) + BALL_RADIUS;
		int rand_vel = (rand() % 10);
		int rand_ang = (rand() % 360);
		ba->ball[i].x = rand_x;
		ba->ball[i].y = rand_y;
		ba->ball[i].angle = rand_ang;  
		ba->ball[i].vx = rand_vel * cos(ba->ball[i].angle * M_PI / 180);
		ba->ball[i].vy = rand_vel * sin(ba->ball[i].angle * M_PI / 180);
		ba->ball[i].r = (float)colours[rand_col][0] / 255;
		ba->ball[i].g = (float)colours[rand_col][1] / 255;
		ba->ball[i].b = (float)colours[rand_col][2] / 255;
		ba->ball[i].m = 1;
	}
	return ba;
}

struct Pendulum *physics_init_pendulum() {
    pd = (struct Pendulum *)malloc(sizeof(struct Spring));
	pd->length = 200;
	pd->angle = M_PI_2;
	pd->x_1 = (float)SCREEN_WIDTH / 2;
	pd->y_1 = SCREEN_HEIGHT; 

	return pd;
}

struct Spring *physics_init_spring() {
    sp = (struct Spring *)malloc(sizeof(struct Spring));
	return sp;
}

void physics_add_ball()
{
	if( ba->count < ba->size) {
		ba->count += 1;
	} else { 
		return;
	}
	printf("adding ball to array: count = %d\n", ba->count);
}

void physics_remove_ball()
{

	if( ba->count == 0) {
		return;
	} else {
		ba->count -= 1;
	}
	printf("removing ball from array: count = %d\n", ba->count);
}

void physics_move_ball_up(){
// check init flag first before accessing
	ba->ball[0].y+=10;
}
void physics_move_ball_down(){
	ba->ball[0].y-=10;
}
void physics_move_ball_left(){
	ba->ball[0].x-=10;
}
void physics_move_ball_right(){
	ba->ball[0].x+=10;
}

void physics_update()
{   

	int demo_mode = keyboard_get_demo_mode();
    // ball handling
    for(i = 0; i < ba->count; i++) { 
	    // gravity
	    //ba->ball[i].vy += gravity;

	    // velocity
	    ba->ball[i].x += ba->ball[i].vx;
	    ba->ball[i].y += ba->ball[i].vy;

	    // wall reflections
 	    if (ba->ball[i].x >= SCREEN_WIDTH - BALL_RADIUS   || ba->ball[i].x <= 0 + BALL_RADIUS ) { ba->ball[i].vx *= -1; }
	    if (ba->ball[i].y >= SCREEN_HEIGHT - BALL_RADIUS  || ba->ball[i].y <= 0 + BALL_RADIUS) { ba->ball[i].vy *= -1; }

        // collision handling, uniform masses
	    for(j = 0; j < ba->count; j++){
	    	if( i == j ) {
	    		continue;
	    	}
			// TODO optimization: only diff balls that are within a range of eachother to reduce comparisons
             int diffx = SCREEN_WIDTH;  // max diff
             int diffy = SCREEN_HEIGHT; // max diff
             diffy = abs((int)ba->ball[i].x - (int)ba->ball[j].x); 
             diffx = abs((int)ba->ball[i].y - (int)ba->ball[j].y);
	    	if( diffx < BALL_DIAMETER && diffy < BALL_DIAMETER ){
				swap_ball(&ba->ball[i],  &ba->ball[j]);
            }
        }
	}
	// spring handling
	if(demo_mode == SPRING){
		if(ba->count != 1){
			ba->count = 1;
		}
		sp->x_1 = (float)SCREEN_WIDTH / 2;
		sp->y_1 = SCREEN_HEIGHT;
		sp->x_2 = ba->ball[0].x;
		sp->y_2 = ba->ball[0].y;

		float sp_length_x = fabs(sp->x_1 - sp->x_2); 
		float sp_length_y = fabs(sp->y_1 - sp->y_2); 
		sp->length = sqrt( pow(sp_length_x, 2) + pow(sp_length_y, 2) );
		//quadrant handling for cos
		if(ba->ball[0].x < SCREEN_WIDTH/2){
			sp->angle = acos(sp_length_x / sp->length) + M_PI_4;
		} else { 
			sp->angle = acos(sp_length_y / sp->length) + 3 * M_PI_4;
		}

		// spring constant k applies force F = kx where x is parallel distance along spring
	}

	// pendulum, constrained motion
	if(demo_mode == PENDULUM){
		if(ba->count != 1){
			ba->count = 1;
		}

		// update ball position by converting polar coodinates to cartesian
		ba->ball[0].x = pd->x_1 - pd->length * sin(pd->angle);
		ba->ball[0].y = pd->y_1 + pd->length * cos(pd->angle);

		// pendulum_update_physics(pd, ba);
		pd->accel = (-1 * gravity / pd->length) * sin(pd->angle);
		pd->vel += pd->accel;
		pd->vel *= 0.995; // damping
		pd->angle += pd->vel; //update the angle with the angular velocity
		printf("accel:%f, vel:%f, angle:%f, pos:%f,%f\n", pd->accel, pd->vel, pd->angle, ba->ball[0].x, ba->ball[0].y);
	}
	// Framerate control
	usleep(frame_duration);
}

void physics_change_demo_mode(int mode){
	if(mode == 1) ;
}

void physics_cleanup(struct ballArray *ba, struct Pendulum *pd, struct Spring *sp){
	free(ba);
	free(pd);
	free(sp);
}

