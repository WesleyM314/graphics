/*
 * template.c
 *
 * An OpenGL source code template.
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>

#include "../lib/initShader.h"
#include "../lib/lib.h"

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))

// vec4 vertices[6] =
// {{ 0.0,  0.5,  0.0, 1.0},	// top
//  {-0.5, -0.5,  0.0, 1.0},	// bottom left
//  { 0.5, -0.5,  0.0, 1.0},	// bottom right
//  { 0.5,  0.8, -0.5, 1.0},	// top
//  { 0.9,  0.0, -0.5, 1.0},	// bottom right
//  { 0.1,  0.0, -0.5, 1.0}};	// bottom left

// vec4 colors[6] =
// {{1.0, 0.0, 0.0, 1.0},	// red   (for top)
//  {0.0, 1.0, 0.0, 1.0},	// green (for bottom left)
//  {0.0, 0.0, 1.0, 1.0},	// blue  (for bottom right)
//  {0.0, 1.0, 0.0, 1.0},	// blue  (for bottom right)
//  {0.0, 1.0, 0.0, 1.0},	// blue  (for bottom right)
//  {0.0, 1.0, 0.0, 1.0}};	// blue  (for bottom right)

vec4 *vertices;
vec4 *colors;

int num_vertices;
int num_colors;

void testing(void)
{
    vertices = (vec4 *)malloc(sizeof(vec4) * 6);
    colors = (vec4 *)malloc(sizeof(vec4) * 6);
    num_vertices = 6;
    num_colors = 6;

    vertices[0] = (vec4){0.0, 0.5, 0.0, 1.0};   // top
    vertices[1] = (vec4){-0.5, -0.5, 0.0, 1.0}; // bottom left
    vertices[2] = (vec4){0.5, -0.5, 0.0, 1.0};  // bottom right
    vertices[3] = (vec4){0.5, 0.8, -0.5, 1.0};  // top
    vertices[4] = (vec4){0.9, 0.0, -0.5, 1.0};  // bottom right
    vertices[5] = (vec4){0.1, 0.0, -0.5, 1.0}; // bottom left

    colors[0] = (vec4){1.0, 0.0, 0.0, 1.0};
    colors[1] = (vec4){0.0, 1.0, 0.0, 1.0};
    colors[2] = (vec4){0.0, 0.0, 1.0, 1.0};
    colors[3] = (vec4){0.0, 1.0, 0.0, 1.0};
    colors[4] = (vec4){0.0, 1.0, 0.0, 1.0};
    colors[5] = (vec4){0.0, 1.0, 0.0, 1.0};
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

    testing();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();

    return 0;
}
