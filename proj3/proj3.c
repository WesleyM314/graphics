/*
 * proj3.c
 *
 * An OpenGL source code proj3.
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "../lib/initShader.h"
#include "../lib/lib.h"
#include "proj3.h"

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))
#define DEBUG_FILE_INPUT 0
#define DEBUG 0

#define MOVE_SPEED 10
#define EYE_LEVEL 1.01 // y offset to simulate eye level
#define GROUND_PADDING 0.2

// Available modes
typedef enum
{
    MAP,
    WALK
} mode;

// Pipeline transformation matrices
mat4 ctm;
mat4 model_view;
mat4 projection;
mat4 arrow_tr;

// Memory locations of pipeline values
GLuint ctm_location;
GLuint model_view_location;
GLuint projection_location;
GLuint colorflag_location;
GLuint draw_arrow_location;
GLuint arrow_tr_location;

// Arrays for vertices and colors
vec4 *vertices;
vec4 *colors;
vec2 *tex_coords;

// Keep track of vertex quantities 
int num_vertices = 0;
int num_colors = 0;
int num_tex_coords = 0;

// Flag for file including colors
GLboolean has_colors = GL_FALSE;

// Texture dimensions
int texw, texh;

// Perspective projection variables
GLfloat left, right, top, bottom, near, far;

// Elevation of eyeline above y=0 plane
GLfloat base_eye_level = 0.5;

// Position vectors related to look_at
vec4 eye = (vec4){0, EYE_LEVEL, 0, 1}; // Start at origin
vec4 at = (vec4){0, EYE_LEVEL, -1, 1}; // Look towards -z
vec4 up = (vec4){0, 1, 0, 0};          // Up stays the same

// Find bounds of points
GLfloat minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;

// Current mode
mode curMode = WALK;

// Flags for movement
GLboolean forward_flag = GL_FALSE;
GLboolean back_flag = GL_FALSE;
GLboolean left_flag = GL_FALSE;
GLboolean right_flag = GL_FALSE;
GLboolean turnleft_flag = GL_FALSE;
GLboolean turnright_flag = GL_FALSE;
GLboolean lookup_flag = GL_FALSE;
GLboolean lookdown_flag = GL_FALSE;

// Flags for animations
GLboolean to_map_flag = GL_FALSE;
GLboolean from_map_flag = GL_FALSE;

// Distance from "ground" used in animation
GLfloat anim_d = 0;

// Keep track of up/down camera angle
GLfloat camera_theta = 0;

// Used in animation calculations
mat4 rx = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};
mat4 ry = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};
mat4 rz = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

/**
 * Idle animation
 */
void idle(void)
{

    // Animate to map mode
    if (to_map_flag == GL_TRUE)
    {
        // Flag to know how long to keep looking down
        GLboolean looking_down = GL_TRUE;
        vec4 v1 = subVec(&at, &eye);

        // If looking almost straight down,
        // don't rotate or orientation is lost
        // printf("Angle between: %f\n", angleBetween(&v1, &up));
        if (angleBetween(&v1, &up) >= M_PI - 0.02)
        {
            // printf("Looking down!\n");
            rz = identity();
            looking_down = GL_FALSE;
        }

        if (looking_down == GL_TRUE)
        {
            // printf("Animating...\n");
            rx = identity();
            ry = identity();
            rz = identity();
            // Calculate rotational axis as the cross
            // product of [eye -> at] and [eye -> at + some y]
            vec4 at_offset = v4(at.x, at.y + 0.005, at.z, at.w);
            vec4 v2 = subVec(&at_offset, &eye);

            vec4 axis = crossVec(&v1, &v2);
            axis = normalize(&axis);

            // Calculate translation matrix to
            // move fixed point (eye) to origin
            mat4 t1 = translate(-eye.x, -eye.y, -eye.z);

            // Rotate a small amount each time
            rz = z_rotate(-0.015);

            // If looking almost straight down,
            // don't rotate or orientation is lost
            if (angleBetween(&v1, &up) >= M_PI - 0.02)
            {
                rz = identity();
                looking_down = GL_FALSE;
            }

            // Rotate axis to plane y = 0
            GLfloat d = sqrtf(axis.y * axis.y + axis.z * axis.z);

            if (d != 0)
            {
                rx.y = v4(0, axis.z / d, axis.y / d, 0);
                rx.z = v4(0, -axis.y / d, axis.z / d, 0);
            }

            // Rotate axis to plane x = 0
            ry.x = v4(d, 0, axis.x, 0);
            ry.z = v4(-axis.x, 0, d, 0);

            // Put together translation matrix
            mat4 m = t1;
            m = multMat(&rx, &m);
            m = multMat(&ry, &m);
            m = multMat(&rz, &m);
            // Invert t1, rx, and ry
            t1 = invMat(&t1);
            rx = invMat(&rx);
            ry = invMat(&ry);
            // Continue making m
            m = multMat(&ry, &m);
            m = multMat(&rx, &m);
            m = multMat(&t1, &m);

            // Apply m to 'at'
            at = multMatVec(&m, &at);
            model_view = look_at(eye, at, up);
            // printVec(&at);
            glutPostRedisplay();
        }
        else
        {
            // Move backwards up into the air until at a
            // height of 100
            eye.y += 1;
            at.y += 1;
            model_view = look_at(eye, at, up);
            glutPostRedisplay();
            if (eye.y >= 200)
                to_map_flag = GL_FALSE;
        }
    }

    // Animate to walk mode
    if (from_map_flag == GL_TRUE)
    {
        // Move towards ground
        if (eye.y > EYE_LEVEL + 0.01)
        {
            // printf("Moving in...\n");
            eye.y -= 1;
            at.y -= 1;
            model_view = look_at(eye, at, up);
            glutPostRedisplay();
        }
        else
        {
            // Make sure eye is at exactly eye level
            eye.y = EYE_LEVEL;
            // look up until looking straight ahead
            if (at.y < eye.y)
            {
                // printf("Looking up...\n");
                rx = identity();
                ry = identity();
                rz = identity();
                // Calculate rotational axis as the cross
                // product of [eye -> at] and [eye -> at + some y]
                vec4 at_offset = v4(at.x, at.y + 0.005, at.z, at.w);
                vec4 v1 = subVec(&at, &eye);
                vec4 v2 = subVec(&at_offset, &eye);

                vec4 axis = crossVec(&v1, &v2);
                // vec4 axis = crossVec(&at, &at_offset);
                axis = normalize(&axis);

                // Calculate translation matrix to
                // move fixed point (eye) to origin
                mat4 t1 = translate(-eye.x, -eye.y, -eye.z);

                // Rotate a small amount each time
                rz = z_rotate(0.015);

                // If angle less than threshold, don't rotate
                // if angle hits 0, orientation is lost
                if (angleBetween(&v1, &up) < 0.02)
                {
                    rz = identity();
                }

                // Rotate axis to plane y = 0
                GLfloat d = sqrtf(axis.y * axis.y + axis.z * axis.z);

                if (d != 0)
                {
                    rx.y = v4(0, axis.z / d, axis.y / d, 0);
                    rx.z = v4(0, -axis.y / d, axis.z / d, 0);
                }

                // Rotate axis to plane x = 0
                ry.x = v4(d, 0, axis.x, 0);
                ry.z = v4(-axis.x, 0, d, 0);

                // Put together translation matrix
                mat4 m = t1;
                m = multMat(&rx, &m);
                m = multMat(&ry, &m);
                m = multMat(&rz, &m);
                // Invert t1, rx, and ry
                t1 = invMat(&t1);
                rx = invMat(&rx);
                ry = invMat(&ry);
                // Continue making m
                m = multMat(&ry, &m);
                m = multMat(&rx, &m);
                m = multMat(&t1, &m);

                // Apply m to 'at'
                at = multMatVec(&m, &at);
                model_view = look_at(eye, at, up);
                glutPostRedisplay();
            }
            else
            {
                // Make sure at and eye on same level
                at.y = eye.y;
                // Stop animation
                from_map_flag = GL_FALSE;
                // Change mode to allow movement
                curMode = WALK;
                model_view = look_at(eye, at, up);
                glutPostRedisplay();
            }
        }
    }

    // Handle movement
    // Should allow for multiple
    // movement directions at once
    // Only move if in walk mode
    if (curMode == WALK)
    {
        // Forward
        if (forward_flag)
        {

            // Use look_at
            // Translate eye in direction of at
            vec4 diff = subVec(&at, &eye);
            // Normalize to unit vector for uniform movement
            diff = normalize(&diff);
            // Translate both eye and at in diff direction
            // Do NOT translate in y direction
            eye.x += diff.x / 10;
            eye.z += diff.z / 10;
            at.x += diff.x / 10;
            at.z += diff.z / 10;
        }
        // Back
        if (back_flag)
        {
            // Translate eye in direction of at
            vec4 diff = subVec(&at, &eye);
            // Normalize to unit vector for uniform movement
            diff = normalize(&diff);
            // Translate both eye and at in diff direction
            // Do NOT translate in y direction
            eye.x -= diff.x / 10;
            eye.z -= diff.z / 10;
            at.x -= diff.x / 10;
            at.z -= diff.z / 10;
        }
        // Left
        if (left_flag)
        {
            // Move along relative x axis
            vec4 z_prime = subVec(&eye, &at);
            vec4 x_prime = crossVec(&up, &z_prime);

            eye.x -= x_prime.x / 10;
            eye.z -= x_prime.z / 10;
            at.x -= x_prime.x / 10;
            at.z -= x_prime.z / 10;
        }
        // Right
        if (right_flag)
        {
            // Move along relative x axis
            vec4 z_prime = subVec(&eye, &at);
            vec4 x_prime = crossVec(&up, &z_prime);

            eye.x += x_prime.x / 10;
            eye.z += x_prime.z / 10;
            at.x += x_prime.x / 10;
            at.z += x_prime.z / 10;
        }
        // Turn left
        if (turnleft_flag)
        {
            // Translate at as if moving eye to origin,
            // rotate, then translate along same vector
            mat4 tr = translate(-eye.x, -eye.y, -eye.z);
            mat4 r = y_rotate(0.0055);
            r = multMat(&r, &tr);
            tr = translate(eye.x, eye.y, eye.z);
            r = multMat(&tr, &r);
            at = multMatVec(&r, &at);
        }
        // Turn right
        if (turnright_flag)
        {
            // Translate at as if moving eye to origin,
            // rotate, then translate along same vector
            mat4 tr = translate(-eye.x, -eye.y, -eye.z);
            mat4 r = y_rotate(-0.0055);
            r = multMat(&r, &tr);
            tr = translate(eye.x, eye.y, eye.z);
            r = multMat(&tr, &r);
            at = multMatVec(&r, &at);
        }
        // Look up
        if (lookup_flag)
        {
            rx = identity();
            ry = identity();
            rz = identity();
            // Calculate rotational axis as the cross
            // product of [eye -> at] and [eye -> at + some y]
            vec4 at_offset = v4(at.x, at.y + 0.005, at.z, at.w);
            vec4 v1 = subVec(&at, &eye);
            vec4 v2 = subVec(&at_offset, &eye);

            vec4 axis = crossVec(&v1, &v2);
            // vec4 axis = crossVec(&at, &at_offset);
            axis = normalize(&axis);

            // Calculate translation matrix to
            // move fixed point (eye) to origin
            mat4 t1 = translate(-eye.x, -eye.y, -eye.z);

            // Rotate a small amount each time
            rz = z_rotate(0.0055);

            // If angle less than threshold, don't rotate
            // if angle hits 0, orientation is lost
            if (angleBetween(&v1, &up) < 0.015)
            {
                rz = identity();
            }

            // Rotate axis to plane y = 0
            GLfloat d = sqrtf(axis.y * axis.y + axis.z * axis.z);

            if (d != 0)
            {
                rx.y = v4(0, axis.z / d, axis.y / d, 0);
                rx.z = v4(0, -axis.y / d, axis.z / d, 0);
            }

            // Rotate axis to plane x = 0
            ry.x = v4(d, 0, axis.x, 0);
            ry.z = v4(-axis.x, 0, d, 0);

            // Put together translation matrix
            mat4 m = t1;
            m = multMat(&rx, &m);
            m = multMat(&ry, &m);
            m = multMat(&rz, &m);
            // Invert t1, rx, and ry
            t1 = invMat(&t1);
            rx = invMat(&rx);
            ry = invMat(&ry);
            // Continue making m
            m = multMat(&ry, &m);
            m = multMat(&rx, &m);
            m = multMat(&t1, &m);

            // Apply m to 'at'
            at = multMatVec(&m, &at);
        }
        // Look down
        if (lookdown_flag)
        {
            rx = identity();
            ry = identity();
            rz = identity();
            // Calculate rotational axis as the cross
            // product of [eye -> at] and [eye -> at + some y]
            vec4 at_offset = v4(at.x, at.y + 0.005, at.z, at.w);
            vec4 v1 = subVec(&at, &eye);
            vec4 v2 = subVec(&at_offset, &eye);

            vec4 axis = crossVec(&v1, &v2);
            // vec4 axis = crossVec(&at, &at_offset);
            axis = normalize(&axis);

            // Calculate translation matrix to
            // move fixed point (eye) to origin
            mat4 t1 = translate(-eye.x, -eye.y, -eye.z);

            // Rotate a small amount each time
            rz = z_rotate(-0.0055);

            // If looking almost straight down,
            // don't rotate or orientation is lost
            if (angleBetween(&v1, &up) >= M_PI - 0.015)
            {
                rz = identity();
            }

            // Rotate axis to plane y = 0
            GLfloat d = sqrtf(axis.y * axis.y + axis.z * axis.z);

            if (d != 0)
            {
                rx.y = v4(0, axis.z / d, axis.y / d, 0);
                rx.z = v4(0, -axis.y / d, axis.z / d, 0);
            }

            // Rotate axis to plane x = 0
            ry.x = v4(d, 0, axis.x, 0);
            ry.z = v4(-axis.x, 0, d, 0);

            // Put together translation matrix
            mat4 m = t1;
            m = multMat(&rx, &m);
            m = multMat(&ry, &m);
            m = multMat(&rz, &m);
            // Invert t1, rx, and ry
            t1 = invMat(&t1);
            rx = invMat(&rx);
            ry = invMat(&ry);
            // Continue making m
            m = multMat(&ry, &m);
            m = multMat(&rx, &m);
            m = multMat(&t1, &m);

            // Apply m to 'at'
            at = multMatVec(&m, &at);
        }
    }

    model_view = look_at(eye, at, up);
    glutPostRedisplay();
}

void init(void)
{

    // Load texture data
    GLubyte my_texels[texw][texh][3];

    // Load texture from file
    FILE *f = fopen("city.data", "r");
    if (f == NULL)
    {
        printf("Error: couldn't open file city.data\n");
        exit(0);
    }
    fread(my_texels, texw * texh * 3, 1, f);
    fclose(f);

    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    if (has_colors)
    {
        glUniform1i(glGetUniformLocation(program, "use_color"), 1);
    }
    else
    {
        glUniform1i(glGetUniformLocation(program, "use_color"), 0);
    }

    // More texture stuff
    if (!has_colors)
    {
        GLuint mytex[1];
        glGenTextures(1, mytex);
        glBindTexture(GL_TEXTURE_2D, mytex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texw, texh, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        int param;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);
    }

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
    buff_offset += sizeof(vec4) * num_tex_coords;

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices));

    // Texture stuff
    if (!has_colors)
    {
        GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
        glEnableVertexAttribArray(vTexCoord);
        glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors));
        // Locate texture
        GLuint texture_location = glGetUniformLocation(program, "texture");
        glUniform1i(texture_location, 0);
        printf("texture_location: %i\n", texture_location);
    }

    // Locate CTM
    ctm_location = glGetUniformLocation(program, "ctm");
    // Locate model_view
    model_view_location = glGetUniformLocation(program, "model_view");
    // Locate pojection
    projection_location = glGetUniformLocation(program, "projection");
    // Locate colorflag
    colorflag_location = glGetUniformLocation(program, "use_color");
    // Locate draw_arrow
    draw_arrow_location = glGetUniformLocation(program, "draw_arrow");
    // Locate arrow_tr
    arrow_tr_location = glGetUniformLocation(program, "arrow_tr");

    glEnable(GL_CULL_FACE);
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
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *)&model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *)&projection);
    glUniformMatrix4fv(arrow_tr_location, 1, GL_FALSE, (GLfloat *)&arrow_tr);

    // Draw city
    glUniform1i(draw_arrow_location, 0);
    glUniform1i(colorflag_location, 0);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices - 9);
    // Draw ground
    glUniform1i(colorflag_location, 1);
    glDrawArrays(GL_TRIANGLES, num_vertices - 9, 6);
    // If in map mode, draw triangle
    if (curMode == MAP)
    {
        // Set draw_arrow
        glUniform1i(draw_arrow_location, 1);
        glDrawArrays(GL_TRIANGLES, num_vertices - 3, 3);
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    // Quit
    if (key == 'q')
        glutLeaveMainLoop();
    // Reset
    if (key == 'r')
    {
        ctm = identity();
        anim_d = base_eye_level;
        camera_theta = 0;
        eye = v4(0, EYE_LEVEL, 0, 1);
        at = v4(0, EYE_LEVEL, -1, 1);
        model_view = look_at(eye, at, up);
        to_map_flag = GL_FALSE;
        from_map_flag = GL_FALSE;
        curMode = WALK;
    }
    // Move left
    if (key == 'j')
    {
        left_flag = GL_TRUE;
    }
    // Move right
    if (key == 'l')
    {
        right_flag = GL_TRUE;
    }

    // Animate to map mode
    if (key == 'm')
    {
        // Rotate arrow about y axis to point forward,
        // then translate to eye

        mat4 r, tr;
        // Get angle between -z axis and eye->at vec
        vec4 facing = subVec(&at, &eye);
        // Remove y element of facing
        facing.y = 0.0;
        vec4 z_axis = v4(0, 0, -1, 0);
        GLfloat theta = angleBetween(&facing, &z_axis);
        // Rotate clockwise or counterclockwise
        if (facing.x > 0)
        {
            theta = -theta;
        }
#if DEBUG
        printf("Rotate arrow %f\n", theta);
        printf("Eye:\n");
        printVec(&eye);
        printf("Point of arrow:\n");
        printVec(&vertices[num_vertices - 3]);
#endif
        r = y_rotate(theta);
        // r = identity();
        // Move to eye
        tr = translate(eye.x, 0, eye.z);
        // tr = identity();
        // Combine transformations
        arrow_tr = multMat(&tr, &r);
#if DEBUG
        printf("arrow_tr\n");
        printMat(&arrow_tr);
#endif

        curMode = MAP;
        from_map_flag = GL_FALSE;
        to_map_flag = GL_TRUE;
    }

    // Animate to walk mode
    if (key == 'e')
    {
        to_map_flag = GL_FALSE;
        from_map_flag = GL_TRUE;
    }
}

void keyboardUp(unsigned char key, int mousex, int mousey)
{

    // Move left
    if (key == 'j')
    {
        left_flag = GL_FALSE;
    }
    // Move right
    if (key == 'l')
    {
        right_flag = GL_FALSE;
    }
}

/**
 * Special keys - downpress
 */
void keySpecial(int key, int mousex, int mousey)
{
    // Up arrow
    if (key == GLUT_KEY_UP)
    {
        forward_flag = GL_TRUE;
    }
    // Down arrow
    if (key == GLUT_KEY_DOWN)
    {
        back_flag = GL_TRUE;
    }
    // Right arrow
    if (key == GLUT_KEY_RIGHT)
    {
        turnright_flag = GL_TRUE;
    }
    // Left arrow
    if (key == GLUT_KEY_LEFT)
    {
        turnleft_flag = GL_TRUE;
    }
    // Page up
    if (key == GLUT_KEY_PAGE_UP)
    {
        lookup_flag = GL_TRUE;
    }
    // Page down
    if (key == GLUT_KEY_PAGE_DOWN)
    {
        lookdown_flag = GL_TRUE;
    }
}

void keySpecialUp(int key, int mousex, int mousey)
{
    // Up arrow
    if (key == GLUT_KEY_UP)
    {
        forward_flag = GL_FALSE;
    }
    // Down arrow
    if (key == GLUT_KEY_DOWN)
    {
        back_flag = GL_FALSE;
    }
    // Right arrow
    if (key == GLUT_KEY_RIGHT)
    {
        turnright_flag = GL_FALSE;
    }
    // Left arrow
    if (key == GLUT_KEY_LEFT)
    {
        turnleft_flag = GL_FALSE;
    }
    // Page up
    if (key == GLUT_KEY_PAGE_UP)
    {
        lookup_flag = GL_FALSE;
    }
    // Page down
    if (key == GLUT_KEY_PAGE_DOWN)
    {
        lookdown_flag = GL_FALSE;
    }
}

void reshape(int width, int height)
{
    glViewport(0, 0, WSIZE, WSIZE);
}

/**
 * Read and parse city.obj
 */
void readFile()
{
    FILE *f = fopen("city.obj", "r");
    const char delim[2] = " "; // Delimiter for tokenizing strings
    char *token;               // Token
    size_t buf_len = 1024;
    char *line = (char *)malloc(buf_len);                 // Buffer to read one line
    char **faces = (char **)malloc(sizeof(char *) * 100); // Buffer to read face values

    // Use a dynamically resizing list to load
    // values since file size is unknown
    // Raw vertices
    v4List vertTemp;
    v4ListNew(&vertTemp);

    // Vertices ordered according to faces
    v4List vertOrdered;
    v4ListNew(&vertOrdered);

    // Raw texture coords
    v2List texTemp;
    v2ListNew(&texTemp);

    // Ordered texture coords according to faces
    v2List texOrdered;
    v2ListNew(&texOrdered);

    // Read file one line at a time until EOF
    GLfloat x, y, z, s, t;
    vec4 v;
    vec2 vt;
    while (fgets(line, buf_len, f) != NULL)
    {
        // Check line type - first token
        token = strtok(line, delim);

        // Handle different line types
        // Vertex
        if (!strcmp(token, "v"))
        {
            // Read x, y, and z
            token = strtok(NULL, delim);
            sscanf(token, "%f", &x);
            token = strtok(NULL, delim);
            sscanf(token, "%f", &y);
            token = strtok(NULL, delim);
            sscanf(token, "%f", &z);
            v = v4(x, y, z, 1.0);

            // Add to vertTemp
            v4ListPush(&vertTemp, v);

            minx = x < minx ? x : minx;
            maxx = x > maxx ? x : maxx;
            miny = y < miny ? y : miny;
            maxy = y > maxy ? y : maxy;
            minz = z < minz ? z : minz;
            maxz = z > maxz ? z : maxz;
        }
        // Texture vertex
        else if (!strcmp(token, "vt"))
        {
            // Read s and t
            token = strtok(NULL, delim);
            sscanf(token, "%f", &s);
            token = strtok(NULL, delim);
            sscanf(token, "%f", &t);
            // Unflip t
            t = 1.0 - t;
            vt = v2(s, t);

            // Add to texTemp
            v2ListPush(&texTemp, vt);
        }
        // Face
        else if (!strcmp(token, "f"))
        {
            // Load vert indeces into faces and count
            int numfaces = 0;
            token = strtok(NULL, delim);
            while (token != NULL)
            {
                faces[numfaces++] = token;
                token = strtok(NULL, delim);
            }
            // If numfaces < 3 something went wrong
            if (numfaces < 3)
            {
                printf("Error: less than 3 indices on face line\n");
                exit(1);
            }
#if DEBUG_FILE_INPUT
            char tempflag = (vertOrdered.length == 0);
            0 if (tempflag)
            {
                printf("numfaces: %d\n", numfaces);
                for (int i = 0; i < numfaces; i++)
                {
                    printf("%s ", faces[i]);
                }
                printf("\n");
            }
#endif

            // Loop over faces in pattern specified in instructions
            // 1,2,3; 1,3,4; 1,4,5; etc.
            int vi, ti; // Vert index, tex index
            for (int i = 0; i < numfaces - 2; i++)
            {
                // Entry 0
                sscanf(faces[0], "%d/%d", &vi, &ti);
#if DEBUG_FILE_INPUT
                0 if (tempflag)
                    printf("vi: %d\tvt: %d\n", vi, ti);
#endif
                v4ListPush(&vertOrdered, vertTemp.items[vi - 1]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1

                // Entry 1 + i
                sscanf(faces[1 + i], "%d/%d", &vi, &ti);
#if DEBUG_FILE_INPUT
                0 if (tempflag)
                    printf("vi: %d\tvt: %d\n", vi, ti);
#endif
                v4ListPush(&vertOrdered, vertTemp.items[vi - 1]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1

                // Entry 2 + i
                sscanf(faces[2 + i], "%d/%d", &vi, &ti);
#if DEBUG_FILE_INPUT
                0 if (tempflag)
                    printf("vi: %d\tvt: %d\n\n", vi, ti);
#endif
                v4ListPush(&vertOrdered, vertTemp.items[vi - 1]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1
            }
        }
    }

#if DEBUG_FILE_INPUT
    0 printf("vertTemp capacity:\t%d\n", vertTemp.capacity);
    printf("vertTemp length:\t%d\n", vertTemp.length);
    printf("texTemp capacity:\t%d\n", texTemp.capacity);
    printf("texTemp length:\t\t%d\n\n", texTemp.length);

    printf("vertOrdered capacity:\t%d\n", vertOrdered.capacity);
    printf("vertOrdered length:\t%d\n", vertOrdered.length);
    printf("texOrdered capacity:\t%d\n", texOrdered.capacity);
    printf("texOrdered length:\t%d\n", texOrdered.length);

    printf("\nminx: %f\t", minx);
    printf("maxx: %f\t", maxx);

    printf("\n\nFirst vertex SHOULD BE:\n");
    printVec(&vertTemp.items[882]);
    printf("First vertex is:\n");
    printVec(&vertOrdered.items[0]);

#endif
    // Add ground
    v4ListPush(&vertOrdered, v4(minx - GROUND_PADDING, miny, maxz + GROUND_PADDING, 1));
    v4ListPush(&vertOrdered, v4(maxx + GROUND_PADDING, miny, maxz + GROUND_PADDING, 1));
    v4ListPush(&vertOrdered, v4(maxx + GROUND_PADDING, miny, minz - GROUND_PADDING, 1));

    v4ListPush(&vertOrdered, v4(maxx + GROUND_PADDING, miny, minz - GROUND_PADDING, 1));
    v4ListPush(&vertOrdered, v4(minx - GROUND_PADDING, miny, minz - GROUND_PADDING, 1));
    v4ListPush(&vertOrdered, v4(minx - GROUND_PADDING, miny, maxz + GROUND_PADDING, 1));

    // Translate so all x >= 0, y >= 0, z <= 0
    mat4 m1 = translate(-minx, -miny, -maxz);
    // Scale by 100
    mat4 m2 = scale(100, 100, 100);
    mat4 m3 = multMat(&m2, &m1);
    for (int i = 0; i < vertOrdered.length; i++)
    {
        vertOrdered.items[i] = multMatVec(&m3, &vertOrdered.items[i]);
    }
    // Add triangle - start with dummy values
    v4ListPush(&vertOrdered, v4(0, 0.1, 0, 1));
    v4ListPush(&vertOrdered, v4(-1, 0.1, 4, 1));
    v4ListPush(&vertOrdered, v4(1, 0.1, 4, 1));

    // Transfer vertOrdered and texOrdered to vertices and tex_coords
    vertices = vertOrdered.items;
    num_vertices += vertOrdered.length;
    tex_coords = texOrdered.items;
    num_tex_coords += texOrdered.length;

    // Fill colors with green for ground, blue for triangle
    num_colors = num_vertices;
    colors = (vec4 *)malloc(sizeof(vec4) * num_colors);
    for (int i = 0; i < num_colors - 3; i++)
    {
        colors[i] = v4(0.11, 0.647, 0.188, 1);
    }
    // Make last three colors blue
    colors[num_colors - 3] = v4(0, 0, 1, 1);
    colors[num_colors - 2] = v4(0, 0, 1, 1);
    colors[num_colors - 1] = v4(0, 0, 1, 1);

#if DEBUG_FILE_INPUT
    0 printf("Done loading from file!\n");
    printf("num_verts: %d\tnum_tex: %d\n", num_vertices, num_tex_coords);
#endif

    // Free memory of vertTemp and texTemp
    free(vertTemp.items);
    free(texTemp.items);
}

void printControls()
{
    printf("\nWelcome!\n\n");
    printf("CONTROLS\n");
    printf("------------------------------------------------\n");
    printf("Up Arrow: Walk Forward\n");
    printf("Down Arrow: Walk Backward\n");
    printf("Left Arrow: Turn Left\n");
    printf("Right Arrow: Turn Right\n");
    printf("J: Walk Left\n");
    printf("L: Walk Right\n");
    printf("M: Enter Map Mode\n");
    printf("E: Enter Walking Mode\n");
    printf("Page Up: Look Up\n");
    printf("Page Down: Look Down\n");
    printf("R: Reset\n");
    printf("Q: Quit\n");

    printf("\n\n");
}

int main(int argc, char **argv)
{
    // TODO print key commands on launch
    printControls();
    // TODO Remove unneeded code
    texw = 1024;
    texh = 1024;
    has_colors = GL_FALSE;
    anim_d = base_eye_level;
    readFile();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WSIZE, WSIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Project 3");
    glewInit();

    ctm = identity();
    model_view = look_at(eye, at, up);
    projection = perspective(-0.3, 0.3, -0.1, 0.3, -1, -210);

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(keySpecial);
    glutSpecialUpFunc(keySpecialUp);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}
