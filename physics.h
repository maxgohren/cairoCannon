#ifndef PHYSICS_H
#define PHYSICS_H

#define FRAMERATE 60
#define MAX_BALLS 50
#define STARTING_BALL_COUNT 2
#define BALL_RADIUS 50
#define BALL_DIAMETER 1.75*BALL_RADIUS

struct Spring {
    float x_1;
    float y_1;
    float x_2;
    float y_2;
    float length;
    float angle;  // RADIANS
    float k;      // spring constant k
};

struct Pendulum {
    float x_1;
    float y_1;
    float x_2;
    float y_2;
	float vel;
	float accel;
    float length;
    float angle;  // RADIANS
};

struct Ball {	
	volatile float x;
	volatile float y;
	double vx;
	double vy;
	double angle; 
	float r;
	float g;
	float b;
    int m;
};

struct ballArray {
	struct Ball ball[MAX_BALLS];
	int size;
	int count;
};


struct ballArray *physics_init_ball_array();
struct Pendulum *physics_init_pendulum();
struct Spring *physics_init_spring();

void physics_add_ball();
void physics_remove_ball();


void physics_change_demo_mode(int mode);
void physics_move_ball_up();
void physics_move_ball_down();
void physics_move_ball_left();
void physics_move_ball_right();

void physics_update();

void physics_cleanup(struct ballArray *ba, struct Pendulum *pd, struct Spring *sp);

#endif // PHYSICS_H
