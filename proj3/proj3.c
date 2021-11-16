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

typedef enum
{
    MAP,
    WALK
} mode;

GLuint ctm_location;
GLuint model_view_location;
GLuint projection_location;
GLuint colorflag_location;
GLuint draw_arrow_location;
GLuint arrow_tr_location;

vec4 *vertices;
vec4 *colors;
vec2 *tex_coords;

int num_vertices = 0;
int num_colors = 0;
int num_tex_coords = 0;
GLboolean idle_spin = GL_FALSE;
GLboolean has_colors = GL_FALSE;
int texw, texh;

mat4 ctm;
mat4 model_view;
mat4 projection;
mat4 arrow_tr;

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
// Values to help animation
GLfloat anim_d = 0;

// Keep track of up/down camera angle
GLfloat camera_theta = 0;

// Variables for mouse movements/dragging
vec4 cur_point = (vec4){0, 0, 0, 1};
vec4 prev_point = (vec4){0, 0, 0, 1};
vec4 rotate_axis = (vec4){0, 0, 0, 1};
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
mat4 rotate_mat = {
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

/**
 * Create unit sphere
 * Mostly for testing
 */
void unitSphere(void)
{
    GLfloat theta;
    mat4 r;
    GLint numSegments = 40, numRings = 40;

    // Allocate space for vertices
    int numVerts = ((numRings)*6) * numSegments;
    vertices = (vec4 *)malloc(sizeof(vec4) * numVerts);

    // Form reference edges of segment
    vec4 *ref1 = (vec4 *)malloc(sizeof(vec4) * (numRings + 1));
    vec4 *ref2 = (vec4 *)malloc(sizeof(vec4) * (numRings + 1));
    theta = M_PI / numRings;
    r = z_rotate(-theta);
    ref1[0] = v4(0, 1, 0, 1);
    for (int i = 1; i <= numRings; i++)
    {
        ref1[i] = multMatVec(&r, &ref1[i - 1]);
    }

    // Copy ref1 to ref2 and rotate about y axis
    theta = 2 * M_PI / numSegments;
    r = y_rotate(theta);
    for (int i = 0; i <= numRings; i++)
    {
        ref2[i] = multMatVec(&r, &ref1[i]);
    }

    // Rotate ref segment about y axis, adding triangles
    for (int i = 0; i < numSegments; i++)
    {
        for (int j = 0; j <= numRings; j++)
        {
            // Top sphere cap - single triangle
            if (j == 0)
            {
                vertices[num_vertices++] = ref1[0];
                vertices[num_vertices++] = ref1[1];
                vertices[num_vertices++] = ref2[1];
            }
            // Bottom sphere cap
            else if (j == numRings)
            {
                vertices[num_vertices++] = ref1[j - 1];
                vertices[num_vertices++] = ref1[j];
                vertices[num_vertices++] = ref2[j - 1];
            }
            else
            {
                vertices[num_vertices++] = ref1[j];
                vertices[num_vertices++] = ref1[j + 1];
                vertices[num_vertices++] = ref2[j];

                vertices[num_vertices++] = ref1[j + 1];
                vertices[num_vertices++] = ref2[j + 1];
                vertices[num_vertices++] = ref2[j];
            }
        }

        // Rotate ref1 and ref2 about y axis
        for (int j = 0; j <= numRings; j++)
        {
            ref1[j] = multMatVec(&r, &ref1[j]);
            ref2[j] = multMatVec(&r, &ref1[j]);
        }
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

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            // On left button click, set cur_point
            // prev_point doesn't matter
            setCurPoint(x, y);
            // Stop rotation
            rotate_mat = identity();
            idle_spin = GL_FALSE;
        }
        else if (state == GLUT_UP)
        {
            idle_spin = GL_TRUE;
        }
    }
    if (button == 3)
    {
        // Zoom in
        mat4 s = scale(1.02, 1.02, 1.02);
        ctm = multMat(&s, &ctm);
        glutPostRedisplay();
    }
    if (button == 4)
    {
        // Zoom out
        mat4 s = scale(1 / 1.02, 1 / 1.02, 1 / 1.02);
        ctm = multMat(&s, &ctm);
        glutPostRedisplay();
    }
}

void setCurPoint(int x, int y)
{
    // Calculate current point vector
    // Convert x and y to OpenGL coordinates
    GLfloat gl_x, gl_y, gl_z, temp;
    gl_x = ((GLfloat)x - ((GLfloat)WSIZE / 2.0)) / ((GLfloat)WSIZE / 2.0);
    gl_y = -((GLfloat)y - ((GLfloat)WSIZE / 2.0)) / ((GLfloat)WSIZE / 2.0);

    // Calculate z coordinate assuming our "glass ball" is a unit sphere
    temp = 1.0 - (gl_x * gl_x + gl_y * gl_y);

    if (temp < 0)
    {
        // If temp is < 0, the mouse is off the
        // "glass ball" and z needs to be set
        // to 0 manually (at least that's how I'm
        // choosing to handle it).
        gl_z = 0;
    }
    else
    {
        gl_z = sqrt(temp);
    }

    // Set cur_point
    cur_point = v4(gl_x, gl_y, gl_z, 1.0);
}

void motion(int x, int y)
{
    // Move cur_point to prev_point
    prev_point = cur_point;

    // Set cur_point
    setCurPoint(x, y);

    // return if cur_point and prev_point are the same
    // to avoid NaN errors
    if (equalVecs(&cur_point, &prev_point))
    {
        return;
    }

    // Calculate angle between prev_point and cur_point
    // Not sure why, but it seems I need to multiply by
    // 1.5 to have the unit sphere move more closely
    // with the mouse cursor
    GLfloat theta = 1.5 * acosf(dotVec(&prev_point, &cur_point) / (magnitude(&prev_point) * magnitude(&cur_point)));

    // Object will be rotated about z by theta degrees
    rz = z_rotate(theta);

    // Calculate rotational axis using cross product
    // of cur_point and prev_point
    rotate_axis = crossVec(&prev_point, &cur_point);

    // If rotation axis is zero vector (like when moving on a
    // diagonal off the edges of the "glass ball") just return
    if (!rotate_axis.x && !rotate_axis.y && !rotate_axis.z)
    {
        return;
    }

    // Normalize rotate_axis
    rotate_axis = normalize(&rotate_axis);

    // Use origin as fixed point
    // Rotate axis to plane y = 0
    GLfloat d = sqrtf(rotate_axis.y * rotate_axis.y + rotate_axis.z * rotate_axis.z);

    if (d != 0)
    {
        rx.y = (vec4){0, rotate_axis.z / d, rotate_axis.y / d, 0};
        rx.z = (vec4){0, -rotate_axis.y / d, rotate_axis.z / d, 0};
    }

    // Rotate axis to plane x = 0
    ry.x = (vec4){d, 0, rotate_axis.x, 0};
    ry.z = (vec4){-rotate_axis.x, 0, d, 0};

    // Get final transformation matrix
    rotate_mat = multMat(&ry, &rx);
    rotate_mat = multMat(&rz, &rotate_mat);
    // Transpose rx and ry
    rx = transpose(&rx);
    ry = transpose(&ry);
    rotate_mat = multMat(&ry, &rotate_mat);
    rotate_mat = multMat(&rx, &rotate_mat);

    // Update ctm
    ctm = multMat(&rotate_mat, &ctm);
    glutPostRedisplay();
}

/**
 * Center object and scale to fit in 
 * field of view
 */
void centerScale()
{
    // Find bounds of points
    GLfloat minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
    for (int i = 0; i < num_vertices; i++)
    {
        minx = vertices[i].x < minx ? vertices[i].x : minx;
        maxx = vertices[i].x > maxx ? vertices[i].x : maxx;

        miny = vertices[i].y < miny ? vertices[i].y : miny;
        maxy = vertices[i].y > maxy ? vertices[i].y : maxy;

        minz = vertices[i].z < minz ? vertices[i].z : minz;
        maxz = vertices[i].z > maxz ? vertices[i].z : maxz;
    }

    // Center point
    vec4 center = v4((maxx + minx) / 2, (maxy + miny) / 2, (maxz + minz) / 2, 1);

    // Find largest range, use that to scale
    GLfloat scaleFactor;

    GLfloat xrange, yrange, zrange;
    xrange = abs(maxx - minx);
    yrange = abs(maxy - miny);
    zrange = abs(maxz - minz);

    scaleFactor = xrange;
    if (yrange > scaleFactor)
        scaleFactor = yrange;
    if (zrange > scaleFactor)
        scaleFactor = zrange;

    scaleFactor = 2 / scaleFactor;

    // Translate so midpoint == origin, then scale
    mat4 t = translate(-center.x, -center.y, -center.z);
    mat4 s = scale(scaleFactor, scaleFactor, scaleFactor);
    mat4 tr = multMat(&s, &t);

    // Translate and scale all vertices
    for (int i = 0; i < num_vertices; i++)
    {
        vertices[i] = multMatVec(&tr, &vertices[i]);
    }
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
        // model_view = translate(0, -base_eye_level, 0);
        // rotate_mat = identity();
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

int main(int argc, char **argv)
{
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
    projection = perspective(-0.2, 0.2, -0.1, 0.3, -1, -210);

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(keySpecial);
    glutSpecialUpFunc(keySpecialUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    // glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}
