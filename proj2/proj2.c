/*
 * proj2.c
 *
 * An OpenGL source code proj2.
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
#include "proj2.h"

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
GLboolean usefile = GL_FALSE;
GLboolean hasColors = GL_FALSE;
int texw, texh;

mat4 ctm;

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

char *filenameInput;

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
 * Read ply file
 */
void readFile()
{
    // Prompt for ply filename
    filenameInput = (char *)malloc(sizeof(char) * 25);
    if (filenameInput == NULL)
    {
        printf("ERROR ALLOCATING MEMORY\n");
        exit(0);
    }
    printf("\nEnter .ply file name (no file extension): ");
    scanf("%s", filenameInput);

    char *filename = (char *)malloc(sizeof(char) * 25);
    strcpy(filename, filenameInput);
    strcat(filename, ".ply");

    // Attempt to open file for reading
    FILE *f;
    f = fopen(filename, "r");
    while (f == NULL)
    {
        printf("ERROR: File '%s' could not be opened\n", filename);
        printf("Enter .ply file name (no file extension): ");
        scanf("%s", filenameInput);
        strcpy(filename, filenameInput);
        strcat(filename, ".ply");
        f = fopen(filename, "r");
    }

    // Read line by line until 'end_header\n' found
    char *buff;
    size_t buffSize = 50;
    buff = (char *)malloc(sizeof(char) * buffSize);
    if (buff == NULL)
    {
        printf("Error allocating memory for buffer\n");
        exit(0);
    }

    int numFileVerts = 0;
    int faces = 0;
    int numFileTexVerts = 0;

    while (!strstr(buff, "end_header\n"))
    {
        getline(&buff, &buffSize, f);
#if DEBUG
        printf("READ LINE: %s", buff);
#endif

        // Find and extract num vertices in file
        if (strstr(buff, " vertex "))
        {
            sscanf(buff, "%*s %*s %d", &numFileVerts);
#if DEBUG
            printf("FOUND VERTEX: ");
            printf("%d\n", numFileVerts);
#endif
        }

        // Find and extract num faces in object
        if (strstr(buff, " face "))
        {
            sscanf(buff, "%*s %*s %d", &faces);
#if DEBUG
            printf("FOUND FACE: ");
            printf("%d\n", faces);
#endif
        }

        // Find and extract num texture vertices
        if (strstr(buff, " multi_texture_vertex "))
        {
            sscanf(buff, "%*s %*s %d", &numFileTexVerts);
#if DEBUG
            printf("FOUND TEX VERTEX: ");
            printf("%d\n", numFileTexVerts);
#endif
        }

        // If string "red" is found, the file
        // contains color values
        if (strstr(buff, " red"))
        {
            hasColors = GL_TRUE;
        }
    }

    // Allocate space for vertices
    // Each face has 3 vertices
    vertices = (vec4 *)malloc(sizeof(vec4) * faces * 3);
    // If has colors, allocate space. Else allocate
    // for tex vertices
    if (hasColors)
    {
        colors = (vec4 *)malloc(sizeof(vec4) * faces * 3);
    }
    else
    {
        tex_coords = (vec2 *)malloc(sizeof(vec2) * faces * 3);
    }

    // Allocate space to load vertex coordinates
    // from file
    vec4 *fileVerts = (vec4 *)malloc(sizeof(vec4) * numFileVerts);
    // If has color, allocate space to load color values
    // from file
    vec4 *fileColors;
    if (hasColors)
    {
        fileColors = (vec4 *)malloc(sizeof(vec4) * numFileVerts);
        if (fileColors == NULL)
        {
            printf("Error allocating memory for file color list\n");
            exit(0);
        }
    }

    if (fileVerts == NULL)
    {
        printf("Error allocating memory for file vertex list\n");
        exit(0);
    }
    // Load floats x, y, and z numFileVerts times
    GLfloat x, y, z;
    unsigned char r, g, b;
    for (int i = 0; i < numFileVerts; i++)
    {
        fread(&x, sizeof(x), 1, f);
        fread(&y, sizeof(y), 1, f);
        fread(&z, sizeof(z), 1, f);
        fileVerts[i] = v4(x, y, z, 1.0);

        // If has colors, load them
        if (hasColors)
        {
            fread(&r, sizeof(char), 1, f);
            fread(&g, sizeof(char), 1, f);
            fread(&b, sizeof(char), 1, f);

            float flr, flg, flb;
            flr = (GLfloat)r;
            flg = (GLfloat)g;
            flb = (GLfloat)b;
            // Scale from [0, 255] to [0.0, 1.0]
            fileColors[i] = v4(flr / 255.0, flg / 255.0, flb / 255.0, 1.0);
            if (i == 0)
            {
                printf("\nr: %d\tg: %d\tb: %d\n", r, g, b);
                printf("flr: %f\tflg: %f\tflb: %f\n", flr, flg, flb);
                printVec(&fileColors[0]);
            }
        }
    }

    // Read vertex indices and copy corresponding
    // vec4 from fileVerts to vertices
    unsigned char num;
    int index;
    for (int i = 0; i < faces; i++)
    {
        // Read num
        fread(&num, sizeof(num), 1, f);

        if (i == 0)
        {
            printf("num = %d\n", num);
        }
        // Read num indices
        for (int j = 0; j < num; j++)
        {
            fread(&index, sizeof(index), 1, f);
            // Check index bounds
            if (index >= numFileVerts)
            {
                printf("ERROR: loaded index out of bounds\n");
                printf("Index: %c\tnumFileVerts: %d", index, numFileVerts);
                exit(0);
            }
            // Copy from fileVerts to vertices
            vertices[num_vertices++] = fileVerts[index];
            // If file has color, copy
            if (hasColors)
            {
                colors[num_colors++] = fileColors[index];
            }
        }
    }
    // If file doesn't have color, load tex data
    if (!hasColors)
    {
        // Allocate space to load tex coords
        vec2 *file_tex = (vec2 *)malloc(sizeof(vec2) * numFileTexVerts);
        if (file_tex == NULL)
        {
            printf("Error allocating memory for file tex coord list\n");
            exit(0);
        }

        // Load floats u and v numFileTexVerts times
        GLfloat u, v;
        for (int i = 0; i < numFileTexVerts; i++)
        {
            // We don't care about the leading char tx
            fread(&num, sizeof(num), 1, f);
            fread(&u, sizeof(u), 1, f);
            fread(&v, sizeof(v), 1, f);
            file_tex[i] = v2(u, v);
        }

        // Read vertex indices and copy corresponding
        // vec2 from file_tex to tex_coords
        unsigned char tx;
        unsigned int tn;

        for (int i = 0; i < faces; i++)
        {
            // Read and ignore tx and tn
            fread(&tx, sizeof(tx), 1, f);
            fread(&tn, sizeof(tn), 1, f);

            // Read num
            fread(&num, sizeof(num), 1, f);
            // Read num indices
            for (int j = 0; j < num; j++)
            {
                fread(&index, sizeof(index), 1, f);
                // Check index bounds
                if (index >= numFileTexVerts)
                {
                    printf("ERROR: loaded index out of bounds\n");
                    printf("Index: %d\tnumFileTexVerts: %d\n", index, numFileTexVerts);
                    exit(0);
                }
                // Copy from file_tex to tex_coords
                tex_coords[num_tex_coords++] = file_tex[index];
            }
        }
    }

    fclose(f);

#if DEBUG
    printf("\nNum vertices: %d\n", num_vertices);
    printf("Num tex vertices: %d\n", num_tex_coords);
    printf("Num colors: %d\n", num_colors);

    printf("\nFirst vertex:\t");
    printVec(&vertices[0]);
    printf("Last vertex:\t");
    printVec(&vertices[num_vertices - 1]);

    if (hasColors)
    {
        printf("First color:\t");
        printVec(&colors[0]);
        printf("Last color:\t");
        printVec(&colors[num_colors - 1]);
    }
    else
    {
        printf("First texture coord:\t");
        printVec2(&tex_coords[0]);
        printf("Last texture coord:\t");
        printVec2(&tex_coords[num_tex_coords - 1]);
    }

#endif
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

    // Allocate space for texture coordinates
    tex_coords = (vec2 *)malloc(sizeof(vec2) * numVerts);

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

    // Create texture coordinates
    for(int i = 0; i < num_vertices; i++)
    {
        GLfloat tex_x, tex_y;
        tex_x = (vertices[i].x + 1.0) / 2.0;
        tex_y = (vertices[i].y - 1) / -2 ;
        tex_coords[num_tex_coords++] = v2(tex_x, tex_y);
    }

#if DEBUG
    printf("First vertex: ");
    printVec(&vertices[0]);
    printf("Last vertex: ");
    printVec(&vertices[num_vertices - 1]);
#endif
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

    // Load texture from file based on
    // user input
    if (usefile && !hasColors)
    {
        char *fn = (char *)malloc(sizeof(char) * 25);
        fn = strcat(filenameInput, ".data");
        FILE *f = fopen(fn, "r");
        if (f == NULL)
        {
            printf("Error: couldn't open file %s\n", fn);
            exit(0);
        }
        fread(my_texels, texw * texh * 3, 1, f);
        fclose(f);
    }
    // Load texture for ball
    else if (!usefile)
    {
        FILE *f = fopen("8ball.data", "r");
        if(f == NULL)
        {
            printf("Error: couldn't open 8ball.data\n");
            exit(0);
        }
        fread(my_texels, texw * texh * 3, 1, f);
        fclose(f);
    }
    // char *fn = (char *)malloc(sizeof(char) * 25);
    // fn = strcat(filenameInput, ".data");
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

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WSIZE, WSIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Project 2");
    glewInit();

    ctm = identity();
    // INSERT SHAPE DRAWING FUNCTIONS HERE
    if (argc != 2)
    {
        printf("Error: wrong number of arguments\n");
        printf("Usage: proj2 ball | file\n");
        exit(0);
    }
    if (!strcmp(argv[1], "ball"))
    {
        texw = 320;
        texh = 320;
        unitSphere();
        // hasColors = GL_TRUE;
        randColors();
    }
    else if (!strcmp(argv[1], "file"))
    {
        usefile = GL_TRUE;
        texw = 1024;
        texh = 1024;
        readFile();
        if(hasColors)
        {
            // free(colors);
            // num_colors = 0;
            // randColors();
        }
    }
    // readFile();
    centerScale();
    // randColors();

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
