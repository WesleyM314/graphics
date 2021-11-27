/*
 * proj4.c
 *
 * An OpenGL source code proj4.
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
#include "proj4.h"

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))
#define DEBUG 1
#define VERTS_PER_CUBE 132

// Pipeline transformation matrices
mat4 ctm;
mat4 model_view;
mat4 projection;
// TODO add array of matrices for individual cubes

// Memory locations of pipeline values
GLuint ctm_location;
GLuint model_view_location;
GLuint projection_location;
GLuint draw_cube_location;
GLuint cube_transform_location;

// Arrays for vertices and colors
vec4 *vertices;
vec4 *colors;
vec2 *tex_coords;

// Vertex quantities
int num_vertices = 0;
int num_colors = 0;
int num_tex_coords = 0;

// Flag for file including colors
GLboolean has_colors = GL_FALSE;

// Texture dimensions
int texw, texh;

// Perspective projection variables
GLfloat left, right, top, bottom, near, far;

// Position vectors related to look_at
vec4 eye = (vec4){0, -0, 3, 1};
vec4 at = (vec4){0, 0, 0, 1}; // Look at origin
vec4 up = (vec4){0, 1, 0, 0}; // Up is y axis

// Flags for movement and animation
GLboolean right_flag = GL_FALSE;
GLboolean left_flag = GL_FALSE;
GLboolean up_flag = GL_FALSE;
GLboolean down_flag = GL_FALSE;

// Transformation matrices for movement
// and animations
mat4 movement_tr;

// Colors
vec4 green = (vec4){0.22745098, 0.478431373, 0.278431373, 1};
vec4 red = (vec4){0.588235294, 0.270588235, 0.274509804, 1};
vec4 blue = (vec4){47.0 / 255, 86.0 / 255, 207.0 / 255, 1};
vec4 orange = (vec4){235.0 / 255, 159.0 / 255, 37.0 / 255, 1};
vec4 yellow = (vec4){1.0, 1.0, 0, 1};
vec4 white = (vec4){1, 1, 1, 1};
vec4 black = (vec4){0, 0, 0, 1};

// Color orders for each small cube to make
// the whole Rubik's cube. Cubes start with
// the back left cube on the top layer, back
// middle, back right, middle left, middle middle,
// etc. The same order is done on the middle level,
// and finally the bottom level
Color color_orders[27][6] = {
    // Level 1
    {BLACK, BLACK, BLUE, ORANGE, WHITE, BLACK},
    {BLACK, BLACK, BLUE, BLACK, WHITE, BLACK},
    {BLACK, RED, BLUE, BLACK, WHITE, BLACK},

    {BLACK, BLACK, BLACK, ORANGE, WHITE, BLACK},
    {BLACK, BLACK, BLACK, BLACK, WHITE, BLACK},
    {BLACK, RED, BLACK, BLACK, WHITE, BLACK},

    {GREEN, BLACK, BLACK, ORANGE, WHITE, BLACK},
    {GREEN, BLACK, BLACK, BLACK, WHITE, BLACK},
    {GREEN, RED, BLACK, BLACK, WHITE, BLACK},

    // Level 2
    {BLACK, BLACK, BLUE, ORANGE, BLACK, BLACK},
    {BLACK, BLACK, BLUE, BLACK, BLACK, BLACK},
    {BLACK, RED, BLUE, BLACK, BLACK, BLACK},

    {BLACK, BLACK, BLACK, ORANGE, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, RED, BLACK, BLACK, BLACK, BLACK},

    {GREEN, BLACK, BLACK, ORANGE, BLACK, BLACK},
    {GREEN, BLACK, BLACK, BLACK, BLACK, BLACK},
    {GREEN, RED, BLACK, BLACK, BLACK, BLACK},

    // Level 3
    {BLACK, BLACK, BLUE, ORANGE, BLACK, YELLOW},
    {BLACK, BLACK, BLUE, BLACK, BLACK, YELLOW},
    {BLACK, RED, BLUE, BLACK, BLACK, YELLOW},

    {BLACK, BLACK, BLACK, ORANGE, BLACK, YELLOW},
    {BLACK, BLACK, BLACK, BLACK, BLACK, YELLOW},
    {BLACK, RED, BLACK, BLACK, BLACK, YELLOW},

    {GREEN, BLACK, BLACK, ORANGE, BLACK, YELLOW},
    {GREEN, BLACK, BLACK, BLACK, BLACK, YELLOW},
    {GREEN, RED, BLACK, BLACK, BLACK, YELLOW},
};

// Array to hold translation matrices
// for each individual small cube
mat4 cube_transforms[27];

// Matrices to keep track of which blocks are
// on which faces. Indeces follow the same
// pattern as the color patterns:
// 0 is top back left, 2 is top back right,
// 8 is top front right, 26 is bottom front right
// Access as [layer][row][col]
int orientation[3][3][3] = {
    {
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 8},
    },
    {
        {9, 10, 11},
        {12, 13, 14},
        {15, 16, 17},
    },
    {
        {18, 19, 20},
        {21, 22, 23},
        {24, 25, 26},
    },
};

/**
 * Idle animation
 */
void idle(void)
{
    movement_tr = identity();
    mat4 t1;

    // Up
    if (up_flag)
    {
        // Make sure eye doesn't reach y axis
        if (angleBetween(&eye, &up) > 0.015)
        {
            // Rotate eye about y to x = 0
            vec4 z = v4(0, 0, 1, 0);
            vec4 cur_x = v4(eye.x, 0, eye.z, 0);
            GLfloat theta_x = angleBetween(&cur_x, &z);

            // Rotate clockwise or counterclockwise
            theta_x = (eye.x > 0) ? -theta_x : theta_x;

            t1 = y_rotate(theta_x);
            movement_tr = multMat(&t1, &movement_tr);

            // Rotate up a little
            t1 = x_rotate(-0.01);
            movement_tr = multMat(&t1, &movement_tr);

            // Rotate back around y
            t1 = y_rotate(-theta_x);
            movement_tr = multMat(&t1, &movement_tr);
        }
    }
    // Down
    if (down_flag)
    {
        // Make sure eye doesn't reach y axis
        if (angleBetween(&eye, &up) < M_PI - 0.015)
        {
            // Rotate eye about y to x = 0
            vec4 z = v4(0, 0, 1, 0);
            vec4 cur_x = v4(eye.x, 0, eye.z, 0);
            GLfloat theta_x = angleBetween(&cur_x, &z);

            // Rotate clockwise or counterclockwise
            theta_x = (eye.x > 0) ? -theta_x : theta_x;

            t1 = y_rotate(theta_x);
            movement_tr = multMat(&t1, &movement_tr);

            // Rotate down a little
            t1 = x_rotate(0.01);
            movement_tr = multMat(&t1, &movement_tr);

            // Rotate back around y
            t1 = y_rotate(-theta_x);
            movement_tr = multMat(&t1, &movement_tr);
        }
    }
    // Right
    if (right_flag)
    {
        t1 = y_rotate(0.01);
        movement_tr = multMat(&t1, &movement_tr);
    }
    // Left
    if (left_flag)
    {
        t1 = y_rotate(-0.01);
        movement_tr = multMat(&t1, &movement_tr);
    }

    eye = multMatVec(&movement_tr, &eye);
    model_view = look_at(eye, at, up);
    glutPostRedisplay();
}

void init(void)
{

    // FIXME no texture used
    // // Load texture data
    // GLubyte my_texels[texw][texh][3];

    // // Load texture from file
    // char *fn = (char *)malloc(sizeof(char) * 25);
    // fn = "filename_here";
    // FILE *f = fopen(fn, "r");
    // if (f == NULL)
    // {
    //     printf("Error: couldn't open file %s\n", fn);
    //     exit(0);
    // }
    // fread(my_texels, texw * texh * 3, 1, f);
    // fclose(f);

    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "use_color"), 1);

    // // More texture stuff
    // if (!has_colors)
    // {
    //     GLuint mytex[1];
    //     glGenTextures(1, mytex);
    //     glBindTexture(GL_TEXTURE_2D, mytex[0]);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texw, texh, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //     int param;
    //     glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);
    // }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Buffer setup
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors + sizeof(vec2) * num_tex_coords, NULL, GL_STATIC_DRAW);

    // Buffer sections
    int buff_offset = 0;
    // Vertices
    glBufferSubData(GL_ARRAY_BUFFER, buff_offset, sizeof(vec4) * num_vertices, vertices);
    buff_offset += sizeof(vec4) * num_vertices;
    // Colors
    glBufferSubData(GL_ARRAY_BUFFER, buff_offset, sizeof(vec4) * num_colors, colors);
    buff_offset += sizeof(vec4) * num_colors;
    // Texture coords
    glBufferSubData(GL_ARRAY_BUFFER, buff_offset, sizeof(vec2) * num_tex_coords, tex_coords);
    buff_offset += sizeof(vec2) * num_tex_coords;

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices));

    // Texture stuff
    // if (!has_colors)
    // {
    //     GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    //     glEnableVertexAttribArray(vTexCoord);
    //     glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors));
    //     // Locate texture
    //     GLuint texture_location = glGetUniformLocation(program, "texture");
    //     glUniform1i(texture_location, 0);
    //     printf("texture_location: %i\n", texture_location);
    // }

    // Locate CTM
    ctm_location = glGetUniformLocation(program, "ctm");
    // Locate model_view
    model_view_location = glGetUniformLocation(program, "model_view");
    // Locate pojection
    projection_location = glGetUniformLocation(program, "projection");
    // Locate draw_cube
    draw_cube_location = glGetUniformLocation(program, "draw_cube");
    // Locate cube_transform
    cube_transform_location = glGetUniformLocation(program, "cube_transform");

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    // TODO reset to black
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glDepthRange(1, 0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&ctm);
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *)&model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *)&projection);

    // glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    // Draw cubes, giving each its own translation matrix
    // int offset = 0;
    glUniform1i(draw_cube_location, 1);
    for (int i = 0; i < 27; i++)
    {
        // Set cube_transform
        glUniformMatrix4fv(cube_transform_location, 1, GL_FALSE, (GLfloat *)&cube_transforms[i]);
        glDrawArrays(GL_TRIANGLES, i * VERTS_PER_CUBE, VERTS_PER_CUBE);
    }
    glUniform1i(draw_cube_location, 0);

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
    // if (button == GLUT_LEFT_BUTTON)
    // {
    //     if (state == GLUT_DOWN)
    //     {
    //         // On left button click, set cur_point
    //         // prev_point doesn't matter
    //         setCurPoint(x, y);
    //         // Stop rotation
    //         rotate_mat = identity();
    //         idle_spin = GL_FALSE;
    //     }
    //     else if (state == GLUT_UP)
    //     {
    //         idle_spin = GL_TRUE;
    //     }
    // }
    // if (button == 3)
    // {
    //     // Zoom in
    //     mat4 s = scale(1.02, 1.02, 1.02);
    //     ctm = multMat(&s, &ctm);
    //     glutPostRedisplay();
    // }
    // if (button == 4)
    // {
    //     // Zoom out
    //     mat4 s = scale(1 / 1.02, 1 / 1.02, 1 / 1.02);
    //     ctm = multMat(&s, &ctm);
    //     glutPostRedisplay();
    // }
}

void motion(int x, int y)
{
    // // Move cur_point to prev_point
    // prev_point = cur_point;

    // // Set cur_point
    // setCurPoint(x, y);

    // // return if cur_point and prev_point are the same
    // // to avoid NaN errors
    // if (equalVecs(&cur_point, &prev_point))
    // {
    //     return;
    // }

    // // Calculate angle between prev_point and cur_point
    // // Not sure why, but it seems I need to multiply by
    // // 1.5 to have the unit sphere move more closely
    // // with the mouse cursor
    // GLfloat theta = 1.5 * acosf(dotVec(&prev_point, &cur_point) / (magnitude(&prev_point) * magnitude(&cur_point)));

    // // Object will be rotated about z by theta degrees
    // rz = z_rotate(theta);

    // // Calculate rotational axis using cross product
    // // of cur_point and prev_point
    // rotate_axis = crossVec(&prev_point, &cur_point);

    // // If rotation axis is zero vector (like when moving on a
    // // diagonal off the edges of the "glass ball") just return
    // if (!rotate_axis.x && !rotate_axis.y && !rotate_axis.z)
    // {
    //     return;
    // }

    // // Normalize rotate_axis
    // rotate_axis = normalize(&rotate_axis);

    // // Use origin as fixed point
    // // Rotate axis to plane y = 0
    // GLfloat d = sqrtf(rotate_axis.y * rotate_axis.y + rotate_axis.z * rotate_axis.z);

    // if (d != 0)
    // {
    //     rx.y = (vec4){0, rotate_axis.z / d, rotate_axis.y / d, 0};
    //     rx.z = (vec4){0, -rotate_axis.y / d, rotate_axis.z / d, 0};
    // }

    // // Rotate axis to plane x = 0
    // ry.x = (vec4){d, 0, rotate_axis.x, 0};
    // ry.z = (vec4){-rotate_axis.x, 0, d, 0};

    // // Get final transformation matrix
    // rotate_mat = multMat(&ry, &rx);
    // rotate_mat = multMat(&rz, &rotate_mat);
    // // Transpose rx and ry
    // rx = transpose(&rx);
    // ry = transpose(&ry);
    // rotate_mat = multMat(&ry, &rotate_mat);
    // rotate_mat = multMat(&rx, &rotate_mat);

    // // Update ctm
    // ctm = multMat(&rotate_mat, &ctm);
    // glutPostRedisplay();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if (key == 'q')
        glutLeaveMainLoop();
    if (key == 'r')
    {
        ctm = identity();
        glutPostRedisplay();
    }
}

void keySpecial(int key, int mousex, int mousey)
{
    // Up arrow
    if (key == GLUT_KEY_UP)
    {
        up_flag = GL_TRUE;
    }
    // Down arrow
    if (key == GLUT_KEY_DOWN)
    {
        down_flag = GL_TRUE;
    }
    // Right arrow
    if (key == GLUT_KEY_RIGHT)
    {
        right_flag = GL_TRUE;
    }
    // Left arrow
    if (key == GLUT_KEY_LEFT)
    {
        left_flag = GL_TRUE;
    }
}

void keySpecialUp(int key, int mousex, int mousey)
{
    // Up arrow
    if (key == GLUT_KEY_UP)
    {
        up_flag = GL_FALSE;
    }
    // Down arrow
    if (key == GLUT_KEY_DOWN)
    {
        down_flag = GL_FALSE;
    }
    // Right arrow
    if (key == GLUT_KEY_RIGHT)
    {
        right_flag = GL_FALSE;
    }
    // Left arrow
    if (key == GLUT_KEY_LEFT)
    {
        left_flag = GL_FALSE;
    }
}

void reshape(int width, int height)
{
    glViewport(0, 0, WSIZE, WSIZE);
}

void printControls() {}

void colorSide(Color c, v4List *list)
{
    vec4 side_color;
    switch (c)
    {
    case GREEN:
        side_color = green;
        break;
    case RED:
        side_color = red;
        break;
    case BLUE:
        side_color = blue;
        break;
    case ORANGE:
        side_color = orange;
        break;
    case YELLOW:
        side_color = yellow;
        break;
    case WHITE:
        side_color = white;
        break;
    case BLACK:
    default:
        side_color = black;
        break;
    }

    for (int i = 0; i < 6; i++)
    {
        v4ListPush(list, side_color);
    }
}

/**
 * Build and scale the Rubik's cube
 */
void buildSmallCube(v4List *verts, v4List *color, Color color_order[6])
{
    // Build a small cube first
    // Use a list to hold verts for now
    // v4List verts, color;
    v4ListNew(verts);
    v4ListNew(color);

    // Create color faces
    // Reference face
    vec4 face_ref[] = {
        v4(-0.2, -0.2, 0.25, 1),
        v4(0.2, -0.2, 0.25, 1),
        v4(0.2, 0.2, 0.25, 1),
        v4(-0.2, -0.2, 0.25, 1),
        v4(0.2, 0.2, 0.25, 1),
        v4(-0.2, 0.2, 0.25, 1),
    };

    mat4 tr;
    GLfloat theta = 0.0;
    tr = y_rotate(theta);

    // Make 4 faces about y axis
    for (int i = 0; i < 4; i++)
    {
        // Copy ref to verts
        for (int j = 0; j < 6; j++)
        {
            v4ListPush(verts, multMatVec(&tr, &face_ref[j]));
        }
        // Increment theta by 90 degrees
        theta += M_PI / 2;
        // Update tr
        tr = y_rotate(theta);
    }

    // Make top face
    tr = x_rotate(-M_PI / 2);
    for (int i = 0; i < 6; i++)
    {
        v4ListPush(verts, multMatVec(&tr, &face_ref[i]));
    }

    // Make bottom face
    tr = x_rotate(M_PI / 2);
    for (int i = 0; i < 6; i++)
    {
        v4ListPush(verts, multMatVec(&tr, &face_ref[i]));
    }

    // Sloped black edges
    // Edge refs to be rotated
    v4List edge_ref;
    v4ListNew(&edge_ref);

    // Corner triangles
    // Top  left
    v4ListPush(&edge_ref, v4(-0.2, 0.25, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.25, 0.2, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.2, 0.2, 0.25, 1));
    // Bottom left
    v4ListPush(&edge_ref, v4(-0.2, -0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(-0.25, -0.2, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.2, -0.25, 0.2, 1));

    // Straight edges
    // Top
    v4ListPush(&edge_ref, v4(-0.2, 0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(0.2, 0.25, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.2, 0.25, 0.2, 1));

    v4ListPush(&edge_ref, v4(-0.2, 0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(0.2, 0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(0.2, 0.25, 0.2, 1));

    // Bottom
    v4ListPush(&edge_ref, v4(-0.2, -0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(-0.2, -0.25, 0.2, 1));
    v4ListPush(&edge_ref, v4(0.2, -0.25, 0.2, 1));

    v4ListPush(&edge_ref, v4(-0.2, -0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(0.2, -0.25, 0.2, 1));
    v4ListPush(&edge_ref, v4(0.2, -0.2, 0.25, 1));

    // Left
    v4ListPush(&edge_ref, v4(-0.2, 0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(-0.25, 0.2, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.25, -0.2, 0.2, 1));

    v4ListPush(&edge_ref, v4(-0.2, 0.2, 0.25, 1));
    v4ListPush(&edge_ref, v4(-0.25, -0.2, 0.2, 1));
    v4ListPush(&edge_ref, v4(-0.2, -0.2, 0.25, 1));

    // Copy edge_ref to verts and rotate like faces above
    theta = 0.0;
    tr = y_rotate(theta);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < edge_ref.length; j++)
        {
            v4ListPush(verts, multMatVec(&tr, &edge_ref.items[j]));
        }
        theta += M_PI / 2;
        tr = y_rotate(theta);
    }

    // Add color
    for (int i = 0; i < 6; i++)
    {
        colorSide(color_order[i], color);
    }
    // Black for edges
    for (int i = 0; i < edge_ref.length; i++)
    {
        v4ListPush(color, v4(0, 0, 0, 1));
        v4ListPush(color, v4(0, 0, 0, 1));
        v4ListPush(color, v4(0, 0, 0, 1));
        v4ListPush(color, v4(0, 0, 0, 1));
    }

    // Free edge_ref
    v4ListFree(&edge_ref);
}

void buildCube()
{
    // Lists for verts and colors
    v4List vert_list, color_list;
    v4ListNew(&vert_list);
    v4ListNew(&color_list);

    // Lists to hold small cube and colors
    v4List small_cube_verts, small_cube_colors;
    mat4 tr;

    // Loop through levels, rows, and columns
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                // Move:
                // +x by 0.5*k - 0.5
                // -y by 0.5*i + 0.5
                // +z by 0.5*j - 0.5
                tr = translate(0.5 * k - 0.5, -0.5 * i + 0.5, 0.5 * j - 0.5);

                // Get the next cube using
                // color order 9*i + 3*j + k
                buildSmallCube(&small_cube_verts, &small_cube_colors, color_orders[9 * i + 3 * j + k]);

                // Translate small cube by tr and add to vert list
                for (int l = 0; l < small_cube_verts.length; l++)
                {
                    v4ListPush(&vert_list, multMatVec(&tr, &small_cube_verts.items[l]));
                    v4ListPush(&color_list, small_cube_colors.items[l]);
                }
            }
        }
    }

    // Transfer lists
    vertices = vert_list.items;
    colors = color_list.items;
    num_vertices = vert_list.length;
    num_colors = color_list.length;
}

int main(int argc, char **argv)
{
    // TODO user menu, set texw & texh, set hasColors
    printControls();

    // Set all cube transformation matrices to identity
    for (int i = 0; i < 27; i++)
    {
        cube_transforms[i] = identity();
    }

    // Rotate view to 45 degrees
    mat4 temp = x_rotate(-M_PI / 6);
    eye = multMatVec(&temp, &eye);
    temp = y_rotate(M_PI / 4);
    eye = multMatVec(&temp, &eye);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WSIZE, WSIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Project 4");
    glewInit();

    ctm = identity();
    model_view = look_at(eye, at, up);
    projection = perspective(-0.5, 0.5, -0.5, 0.5, -0.5, -10);

    buildCube();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keySpecial);
    glutSpecialUpFunc(keySpecialUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    // glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}
