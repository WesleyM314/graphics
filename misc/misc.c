// Things I've written that I don't need for the actual labs but might come in handy

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <stdio.h>

#include "../lib/initShader.h"
#include "../lib/lib.h"

vec4 *vertices;
vec4 *colors;

int num_vertices = 0;
int num_colors = 0;

/**
 * Draws a circle facing the screen
 */
void circleTest(void)
{
    GLfloat x, y, theta, radius;

    int num_triangles = 100;
    vertices = (vec4 *)malloc(sizeof(vec4) * 3 * num_triangles);
    colors = (vec4 *)malloc(sizeof(vec4) * 3 * num_triangles);

    // The center angle of each triangle is 2pi/num_triangles
    theta = 2 * M_PI / num_triangles;
    // Outer radius of circle
    radius = 0.5;

    // Create circle using right hand rule, going counter-clockwise
    for (int i = 0; i < num_triangles; i++)
    {
        // Circle center
        vertices[num_vertices] = (vec4){0, 0, 0, 1};

        // Second point, starts on x axis
        x = radius * (GLfloat)cos((double)(theta * (GLfloat)i));
        y = radius * (GLfloat)sin((double)(theta * (GLfloat)i));
        vertices[num_vertices + 1] = (vec4){x, y, 0, 1};

        // Third point, angle offset of theta
        x = radius * (GLfloat)cos((double)((theta * (GLfloat)i) + theta));
        y = radius * (GLfloat)sin((double)((theta * (GLfloat)i) + theta));
        vertices[num_vertices + 2] = (vec4){x, y, 0, 1};

        // Increment num_vertices by 3
        num_vertices += 3;

        // Add colors
        colors[num_colors] = (vec4){0, 1, 0, 1};
        colors[num_colors + 1] = (vec4){0, .3, 1, 1};
        colors[num_colors + 2] = (vec4){0, .3, 1, 1};
        num_colors += 3;
    }
}