#define WSIZE 1024

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

void setCurPoint(int x, int y);
void printControls();
void turnFace(Face face, Direction direction);
void updateOrientation(Face face, Direction direction);
void getOrientation(Face face, int arr[9]);
