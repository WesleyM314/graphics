/*
 * proj1.c
 *
 * An OpenGL source code proj1.
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../lib/initShader.h"
#include "../lib/lib.h"

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))

GLuint ctm_location;

vec4 *vertices;
vec4 *colors;

int num_vertices = 0;
int num_colors = 0;
GLboolean pause = 1;

mat4 ctm = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

GLfloat idleRads = 0; // Radians for idle animation rotation

/**
 * Idle animation
 */
void idle(void)
{
    if (!pause)
    {
        idleRads += 0.005;
        if (idleRads > 2 * M_PI)
        {
            idleRads = 0;
        }

        // Get ctm
        ctm = x_rotate(-idleRads);
        // Redraw
        glutPostRedisplay();
    }
}

/**
 * Creates my computer generated object (sphere, torus, or spring)
 */
void myObject(void)
{

    GLfloat theta, x, y, z;
    mat4 r, t, tr;
    // TORUS
    // Number of loop segments that make
    // up the torus
    GLint numSegments = 36;
    // Number of points on each circle
    // making up loop segments
    GLint numCircPoints = 36;
    // Number of coils in spring
    GLint numCoils = 3;

    // Allocate space for vertices
    // Vertices in "shaft" + vertices in end caps
    int numVerts = numSegments * numCircPoints * numCoils * 6 + 2 * 3 * numCircPoints;
    vertices = (vec4 *)malloc(sizeof(vec4) * numVerts);

    // Make circle cross section for reference
    // Use unit circle for ease of coding, scale later
    // Translate all points + 2 on x axis so circle can
    // easily be rotated around y axis to make torus
    vec4 *refCirc1 = (vec4 *)malloc(sizeof(vec4) * numCircPoints);
    theta = 2 * M_PI / numCircPoints;
    for (int i = 0; i < numCircPoints; i++)
    {
        x = (GLfloat)cos(theta * (GLfloat)i) + 2;
        y = (GLfloat)sin(theta * (GLfloat)i);
        // Add point to refCirc
        refCirc1[i] = v4(x, y, 0, 1);
    }

    // Copy refCirc1 to give reference for other
    // edge of each segment
    vec4 *refCirc2 = (vec4 *)malloc(sizeof(vec4) * numCircPoints);
    memcpy(refCirc2, refCirc1, sizeof(vec4) * numCircPoints);

    // Rotate refCirc1 around y axis by 2 * PI / numSegments
    // to make reference for leading edge of segment
    // Add to y to make coil
    theta = 2 * M_PI / numSegments;
    r = y_rotate(theta);
    t = translate(0, 0.1, 0);
    tr = multMat(&r, &t);
    for (int i = 0; i < numCircPoints; i++)
    {
        refCirc1[i] = multMatVec(&tr, &refCirc1[i]);
    }

    // User refCirc2 to make first end cap
    for(int i = 0; i < numCircPoints; i++)
    {
        vertices[num_vertices++] = v4(2, 0, 0, 1);
        vertices[num_vertices++] = refCirc2[i];
        vertices[num_vertices++] = refCirc2[(i + 1) % numCircPoints];
    }

    for (int h = 0; h < numSegments * numCoils; h++)
    {
        // Draw triangles between refCircles to make shell
        // of segment.
        for (int i = 0; i < numCircPoints; i++)
        {
            vertices[num_vertices] = refCirc2[i];
            vertices[num_vertices + 1] = refCirc1[i];
            vertices[num_vertices + 2] = refCirc2[(i + 1) % numCircPoints];

            vertices[num_vertices + 3] = refCirc1[i];
            vertices[num_vertices + 4] = refCirc1[(i + 1) % numCircPoints];
            vertices[num_vertices + 5] = refCirc2[(i + 1) % numCircPoints];

            num_vertices += 6;
        }

        // Rotate both ref circles about y
        for (int i = 0; i < numCircPoints; i++)
        {
            refCirc1[i] = multMatVec(&tr, &refCirc1[i]);
            refCirc2[i] = multMatVec(&tr, &refCirc2[i]);
        }
    }

    // Add second end cap
    for(int i = 0; i < numCircPoints; i++)
    {
        vertices[num_vertices++] = v4(2, numSegments * numCoils * 0.1, 0, 1);
        vertices[num_vertices++] = refCirc2[(i + 1) % numCircPoints];
        vertices[num_vertices++] = refCirc2[i];
    }

    // Translate -0.25 on y
    t = translate(0, -0.5, 0);
    // Scale everything back down
    mat4 s = scale(0.1, 0.1, 0.1);
    tr = multMat(&t, &s);
    for (int i = 0; i < num_vertices; i++)
    {
        vertices[i] = multMatVec(&tr, &vertices[i]);
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

    // Locate CTM
    ctm_location = glGetUniformLocation(program, "ctm");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1, 0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&ctm);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if (key == 'q')
        glutLeaveMainLoop();
    if (key == ' ')
        pause = !pause;

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
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Template");
    glewInit();

    myObject();
    randColors();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    // glutReshapeFunc(reshape);

    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}
