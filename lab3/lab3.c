/*
 * template.c
 *
 * An OpenGL source code template.
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../lib/initShader.h"
#include "../lib/lib.h"

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))

vec4 *vertices;
vec4 *colors;

int num_vertices = 0;
int num_colors = 0;

/**
 * Draws a cone with a circular base sitting flat on the plane y = -0.5
 * With a height of 1 and a radius of 0.5
 */
void drawCone(void)
{
    GLfloat x, z, theta, radius, height;
    vec4 a, b, c;

    // Number of triangles to use in the base (and on the sides of the cone)
    GLint numCircTriangles = 50;

    // Allocate space for vertices
    vertices = (vec4 *)malloc(sizeof(vec4) * 3 * numCircTriangles * 2);

    // Angle of each slice of the circle
    theta = 2 * M_PI / numCircTriangles;
    // Radius of base
    radius = 0.5;
    // Height of cone
    height = 1;

    // Draw cone by making two identical circles and raising the y coordinate
    // of the center of the second circle
    for (int i = 0; i < numCircTriangles; i++)
    {
        // Circle center
        a = (vec4){0.0, -0.5, 0.0, 1.0};
        // Second vertex
        x = radius * (GLfloat)cos((double)(theta * (GLfloat)i));
        z = radius * (GLfloat)sin((double)(theta * (GLfloat)i));
        b = (vec4){x, -0.5, z, 1.0};
        // Third vertex, angle offset of theta
        x = radius * (GLfloat)cos((double)((theta * (GLfloat)i) + theta));
        z = radius * (GLfloat)sin((double)((theta * (GLfloat)i) + theta));
        c = (vec4){x, -0.5, z, 1.0};

        // Add one triangle as calculated for base
        vertices[num_vertices++] = a;
        vertices[num_vertices++] = b;
        vertices[num_vertices++] = c;

        // Change y value of center vertex and add another triangle
        a.y += height;
        vertices[num_vertices++] = a;
        vertices[num_vertices++] = b;
        vertices[num_vertices++] = c;
    }
}

/**
 * Creates a random color for every triangle currently
 * in the vertices array
 */
void randColors(void)
{
    // Red, green, blue
    GLfloat r, g, b;

    // Allocate memory for color vectors
    colors = (vec4 *)malloc(sizeof(vec4) * num_vertices);

    // Seed random number generator
    srand((unsigned)time(NULL));

    // Loop through triangles
    GLint num = num_vertices / 3;
    for (int i = 0; i < num; i++)
    {
        // Get random values, scaling to range [0..1]
        r = (GLfloat)rand() / (GLfloat)RAND_MAX;
        b = (GLfloat)rand() / (GLfloat)RAND_MAX;
        g = (GLfloat)rand() / (GLfloat)RAND_MAX;

        // Add to color array 3 times to color whole triangle
        colors[num_colors++] = (vec4){r, g, b, 1.0};
        colors[num_colors++] = (vec4){r, g, b, 1.0};
        colors[num_colors++] = (vec4){r, g, b, 1.0};
    }
}

void init(void)
{
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec4) * num_colors, colors);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices));

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1, 0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if (key == 'q')
        glutLeaveMainLoop();

    //glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Template");
    glewInit();

    // testing();
    // circleTest();
    drawCone();
    randColors();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();

    return 0;
}
