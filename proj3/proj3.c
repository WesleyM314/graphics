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
#define DEBUG 1

GLuint ctm_location;

vec4 *vertices;
vec4 *colors;
vec2 *tex_coords;

int num_vertices = 0;
int num_colors = 0;
int num_tex_coords = 0;
GLboolean idleSpin = GL_FALSE;
GLboolean hasColors = GL_FALSE;
int texw, texh;

mat4 ctm;

// Find bounds of points
GLfloat minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;

// Variables for mouse movements/dragging
vec4 curPoint = (vec4){0, 0, 0, 1};
vec4 prevPoint = (vec4){0, 0, 0, 1};
vec4 rotateAxis = (vec4){0, 0, 0, 1};
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
mat4 rotateMat = {
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
    if (idleSpin)
    {
        ctm = multMat(&rotateMat, &ctm);
        glutPostRedisplay();
    }
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

    // TODO
    // Load texture from file
    char *fn = (char *)malloc(sizeof(char) * 25);
    fn = "filename_here";
    FILE *f = fopen(fn, "r");
    if (f == NULL)
    {
        printf("Error: couldn't open file %s\n", fn);
        exit(0);
    }
    fread(my_texels, texw * texh * 3, 1, f);
    fclose(f);

    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    if (hasColors)
    {
        glUniform1i(glGetUniformLocation(program, "use_color"), 1);
    }
    else
    {
        glUniform1i(glGetUniformLocation(program, "use_color"), 0);
    }

    // More texture stuff
    if (!hasColors)
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

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors + sizeof(vec2) * num_tex_coords, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec4) * num_colors, colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec4) * num_colors, sizeof(vec2) * num_tex_coords, tex_coords);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vec4) * num_vertices));

    // Texture stuff
    if (!hasColors)
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

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            // On left button click, set curPoint
            // prevPoint doesn't matter
            setCurPoint(x, y);
            // Stop rotation
            rotateMat = identity();
            idleSpin = GL_FALSE;
        }
        else if (state == GLUT_UP)
        {
            idleSpin = GL_TRUE;
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

    // Set curPoint
    curPoint = v4(gl_x, gl_y, gl_z, 1.0);
}

void motion(int x, int y)
{
    // Move curPoint to prevPoint
    prevPoint = curPoint;

    // Set curPoint
    setCurPoint(x, y);

    // return if curPoint and prevPoint are the same
    // to avoid NaN errors
    if (equalVecs(&curPoint, &prevPoint))
    {
        return;
    }

    // Calculate angle between prevPoint and curPoint
    // Not sure why, but it seems I need to multiply by
    // 1.5 to have the unit sphere move more closely
    // with the mouse cursor
    GLfloat theta = 1.5 * acosf(dotVec(&prevPoint, &curPoint) / (magnitude(&prevPoint) * magnitude(&curPoint)));

    // Object will be rotated about z by theta degrees
    rz = z_rotate(theta);

    // Calculate rotational axis using cross product
    // of curPoint and prevPoint
    rotateAxis = crossVec(&prevPoint, &curPoint);

    // If rotation axis is zero vector (like when moving on a
    // diagonal off the edges of the "glass ball") just return
    if (!rotateAxis.x && !rotateAxis.y && !rotateAxis.z)
    {
        return;
    }

    // Normalize rotateAxis
    rotateAxis = normalize(&rotateAxis);

    // Use origin as fixed point
    // Rotate axis to plane y = 0
    GLfloat d = sqrtf(rotateAxis.y * rotateAxis.y + rotateAxis.z * rotateAxis.z);

    if (d != 0)
    {
        rx.y = (vec4){0, rotateAxis.z / d, rotateAxis.y / d, 0};
        rx.z = (vec4){0, -rotateAxis.y / d, rotateAxis.z / d, 0};
    }

    // Rotate axis to plane x = 0
    ry.x = (vec4){d, 0, rotateAxis.x, 0};
    ry.z = (vec4){-rotateAxis.x, 0, d, 0};

    // Get final transformation matrix
    rotateMat = multMat(&ry, &rx);
    rotateMat = multMat(&rz, &rotateMat);
    // Transpose rx and ry
    rx = transpose(&rx);
    ry = transpose(&ry);
    rotateMat = multMat(&ry, &rotateMat);
    rotateMat = multMat(&rx, &rotateMat);

    // Update ctm
    ctm = multMat(&rotateMat, &ctm);
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
    if (key == 'q')
        glutLeaveMainLoop();
    if (key == 'r')
    {
        ctm = identity();
        rotateMat = identity();
        glutPostRedisplay();
    }

    //glutPostRedisplay();
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

    // fgets(line, buf_len, f);
    // // Check line type - first token
    // token = strtok(line, delim);
    // printf("FIRST TOKEN = %s\n", token);
    // printf("Line after strtok: %s\n", line);
    // exit(0);
    // token = strtok(line, delim);
    // // Read other tokens in line
    // while(token != NULL)
    // {
    //     printf("token: %s\n", token);
    //     token = strtok(line, delim);
    // }
    // exit(0);

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

            // Keep track of x, y, z bounds
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

            // Loop over faces in pattern specified in instructions
            // 1,2,3; 1,3,4; 1,4,5; etc.
            int vi, ti; // Vert index, tex index
            for (int i = 0; i < numfaces - 2; i++)
            {
                // Entry 0
                sscanf(faces[0], "%d/%d", &vi, &ti);
                v4ListPush(&vertOrdered, vertTemp.items[vi]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1

                // Entry 1 + i
                sscanf(faces[1 + i], "%d/%d", &vi, &ti);
                v4ListPush(&vertOrdered, vertTemp.items[vi]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1

                // Entry 2 + i
                sscanf(faces[2 + i], "%d/%d", &vi, &ti);
                v4ListPush(&vertOrdered, vertTemp.items[vi]);
                v2ListPush(&texOrdered, texTemp.items[ti - 1]); // Texture index starts at 1
            }
        }
    }

#if DEBUG
    printf("vertTemp capacity:\t%d\n", vertTemp.capacity);
    printf("vertTemp length:\t%d\n", vertTemp.length);
    printf("texTemp capacity:\t%d\n", texTemp.capacity);
    printf("texTemp length:\t\t%d\n\n", texTemp.length);

    printf("vertOrdered capacity:\t%d\n", vertOrdered.capacity);
    printf("vertOrdered length:\t%d\n", vertOrdered.length);
    printf("texOrdered capacity:\t%d\n", texOrdered.capacity);
    printf("texOrdered length:\t%d\n", texOrdered.length);

#endif

    // Transfer vertOrdered and texOrdered to vertices and tex_coords
    vertices = vertOrdered.items;
    tex_coords = texOrdered.items;

    // Free memory of vertTemp and texTemp
    free(vertTemp.items);
    free(texTemp.items);
}

int main(int argc, char **argv)
{
    // int foo, bar;
    // char str[] = "1/2";
    // printf("String: %s\n", str);
    // sscanf(str, "%d/%d", &foo, &bar);
    // printf("foo: %d\tbar: %d\n", foo, bar);
    // exit(0);
    // TODO user menu, set texw & texh, set hasColors
    texw = 1024;
    texh = 1024;
    hasColors = GL_FALSE;
    readFile();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WSIZE, WSIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Project 3");
    glewInit();

    ctm = identity();
    // INSERT SHAPE DRAWING FUNCTIONS HERE
    unitSphere(); // REPLACE ME
    randColors();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    // glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}

// Functions for dynamic floatList

// // Init a list
// void listNew(floatList *l)
// {
//     l->capacity = 512;
//     l->items = (GLfloat *)malloc(sizeof(GLfloat) * l->capacity);
//     l->total = 0;
// }

// // Resize an array list
// void listResize(floatList *list, int capacity)
// {
//     if (list)
//     {
//         GLfloat *items = (GLfloat *)realloc(list->items, sizeof(GLfloat) * capacity);
//         if (items)
//         {
//             list->items = items;
//             list->capacity = capacity;
//             return;
//         }
//         printf("Error allocating memory for array list\n");
//         exit(1);
//     }
// }

// // Add to an array list
// void listPush(floatList *list, GLfloat item)
// {
//     if (list)
//     {
//         // Check if list is full
//         if (list->capacity == list->total)
//         {
//             // Double in size
//             listResize(list, list->capacity * 2);
//         }
//         // Add to end of list
//         list->items[list->total] = item;
//         list->total += 1;
//     }
// }
