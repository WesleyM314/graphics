#define WSIZE 1024

void setCurPoint(int x, int y);
void printControls();

typedef enum 
{
	GREEN,
	RED,
	BLUE,
	ORANGE,
	YELLOW,
	WHITE,
	BLACK
} Color;

typedef enum 
{
	FRONT,
	RIGHT,
	BACK,
	LEFT,
	TOP,
	BOTTOM
} Face;

typedef enum 
{
	FORWARD,
	BACKWARD
} Direction;
