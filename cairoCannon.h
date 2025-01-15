#define FRAMERATE 60
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define MAX_BALLS 50
#define STARTING_BALL_COUNT 2
#define BALL_RADIUS 50
#define BALL_DIAMETER 1.75*BALL_RADIUS

void poll_x_event(XEvent event);

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
