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
#include "proj1.h"

#define SMOOTH 1

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))
#define WSIZE 1024

GLuint ctm_location;

vec4 *vertices;
vec4 *colors;
vec4 *normals;

int num_vertices = 0;
int num_colors = 0;
int num_normals = 0;
GLboolean pause = 0;
GLboolean idleSpin = GL_FALSE;

mat4 ctm = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

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
 * Creates my computer generated object (sphere, torus, or spring)
 */
void spring(void)
{

    GLfloat theta, x, y;
    mat4 r, t, tr;
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
    for (int i = 0; i < numCircPoints; i++)
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
    for (int i = 0; i < numCircPoints; i++)
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
 * Reads vertices from a txt file
 * and displays the object
 */
void fromFile(char *filename)
{
    FILE *f;
    int num = 0;
    f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("ERROR: File '%s' could not be opened\n", filename);
        exit(0);
    }
    fscanf(f, "%d", &num);
    printf("%d\n", num);

    // Allocate memory for vertices
    vertices = (vec4 *)malloc(sizeof(vec4) * num);

    GLfloat x, y, z, w;
    vec4 v;
    for (int i = 0; i < num; i++)
    {
        // Read line
        fscanf(f, "%f, %f, %f, %f", &x, &y, &z, &w);
        v = v4(x, y, z, w);

        // Add vertice
        vertices[num_vertices++] = v;
    }

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec4) * num_normals, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec4) * num_normals, normals);

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
}

void reshape(int width, int height)
{
    glViewport(0, 0, WSIZE, WSIZE);
}

/**
 * Calculate array of normal vectors for all
 * currently existing vertices.
 * Treats all faces as flat.
 */
void getNormals()
{
    // Array list to add normals to
    v4List norm_list;
    v4ListNew(&norm_list);

    vec4 p1, p2, p3, v1, v2, n;
#if SMOOTH
    // For smooth color, normal is the vector from origin to each vertex
    for (int i = 0; i < num_vertices; i = i + 3)
    {
        p1 = vertices[i];
        p2 = vertices[i + 1];
        p3 = vertices[i + 2];

        v4ListPush(&norm_list, normalize(&p1));
        v4ListPush(&norm_list, normalize(&p2));
        v4ListPush(&norm_list, normalize(&p3));
    }
#else
    // Iterate over vertices 3 at a time
    for (int i = 0; i < num_vertices; i = i + 3)
    {
        p1 = vertices[i];
        p2 = vertices[i + 1];
        p3 = vertices[i + 2];

        // Calculate vectors by point subtraction
        v1 = subVec(&p2, &p1);
        v2 = subVec(&p3, &p2);

        // Get normal with cross product
        n = crossVec(&v1, &v2);
        // Normalize
        n = normalize(&n);

        // Add to list three times
        v4ListPush(&norm_list, n);
        v4ListPush(&norm_list, n);
        v4ListPush(&norm_list, n);
    }
#endif

    // Transfer array list to normals
    normals = norm_list.items;
    num_normals = norm_list.length;
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WSIZE, WSIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Template");
    glewInit();

    if (argc >= 2)
    {
        if (!strcmp(argv[1], "spring"))
        {
            // Display spring
            printf("Drawing spring...\n");
            spring();
        }
        else if (!strcmp(argv[1], "file"))
        {
            // Load from file
            if (argc < 3)
            {
                printf("Missing argument\n");
                printf("Expected arguments: [spring] or [file <filename>]\n");
                exit(0);
            }
            printf("Loading from file...\n");
            fromFile(argv[2]);
        }
        else if (!strcmp(argv[1], "sphere"))
        {
            // Display unit sphere for testing
            printf("Drawing unit sphere...\n");
            unitSphere();
        }
    }
    else
    {
        printf("Expected arguments: [spring] or [file <filename>]\n");
        exit(0);
    }

    getNormals();
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
