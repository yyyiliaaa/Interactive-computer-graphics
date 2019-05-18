/************************************************************
 * Handout: rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
 sample code "example3.cpp" of Chapter 4.
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
 Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
 and also to include the use of the function NormalMatrix(mv) to return the
 normal matrix (mat3) of a given model-view matrix mv (mat4).
 
 (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
 by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
 other related functions such as inverse(m) for the inverse of 3x3 matrix m are
 also added; see the file "mat-yjc-new.h".)
 
 * Extensively modified by Yi-Jen Chiang for the program structure and user
 interactions. See the function keyboard() for the keyboard actions.
 Also extensively re-structured by Yi-Jen Chiang to create and use the new
 function drawObj() so that it is easier to draw multiple objects. Now a floor
 and a rotating cube are drawn.
 
 ** Perspective view of a color cube using LookAt() and Perspective()
 
 ** Colors are assigned to each vertex and then the rasterizer interpolates
 those colors across the triangles.
 **************************************************************/
#include "Angel-yjc.h"
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

/* global definitions for constants and global image arrays */

#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];

#define    stripeImageWidth 32
GLubyte stripeImage[4*stripeImageWidth];
GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint firework;
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;    /* vertex buffer object id for axis */
GLuint shadow_buffer;   /* vertex buffer object id for shadow */
GLuint firework_buffer;

static GLuint checkerboard;
static GLuint stripe;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 15.0;
GLfloat  angle = 0.0; // rotation angle in degrees

GLfloat d;  //rotate distance
GLfloat dnext;  //next rotate distance
GLfloat AB = sqrt(5*5+0*0+7.5*7.5);
GLfloat BC = sqrt(4*4+0*0+1.5*1.5);
GLfloat CA = sqrt(1*1+0*0+9*9);


vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position
vec4 a(1,0,0,0);
vec4 b(0,1,0,0);
vec4 c(0,0,1,0);
vec4 e(0,0,0,1);
mat4 accR( a, b, c, e);

vec4    at(0.0, 0.0, 0.0, 1.0);
vec4    up(0.0, 1.0, 0.0, 0.0);

int animationFlag = 1; // 1: animation; 0: non-animation.
int rollingFlag = 0;    //1:begin rolling the sphere
int wireFlag = 0;   //  0: solid; 1: wireframe;

int segmentflag = 1;   //1:AB; 2:BC; 3:CA
int shadowFlag = 0; // 0: No shadow; 1:shadow
int lightingFlag = 1;   //0: No light; 1:Light
int shadingFlag = 1;    //0: flat; 1:smooth
int lightoption = 0;    //0:no; 1:spot light; 2:point source
int fogoption = 0;  //0:“no fog”; 1.“linear”; 2.“exponential"; 3.“exponential square”
int blendflag = 0;  //0: no blend; 1: blend

int colorFlag = 0;  //0: normal; 1: color
int floorColor = 0;
int sphereColor = 0;
int shadowColor = 1;
int texground_flag = 0;  // 0: no texture application: obj color; 1: (obj color) * (texture color)
int texsphere_flag = 0; // 0: no texture application: obj color; 1: line; 2:checkerboard
int texturepart;    //0:other; 1:floor; 2:sphere
int spaceoption = 0;    //0:object; 1:eye
int texsphere_dir = 0;  //0:vertical; 1:slanted
int latticeoption = 0;
int latticedir = 0; //0:upright; 1:tilted

int fireworkflag = 0; //0: no; 1: yes
float timesub;

char filename[20] = {};
int sphere_NumTriangles;
int sphere_NumVertices;
point4* sphere_points;
color4* sphere_colors;
point4* shadow_colors;
vec3*   normals_f;
vec3*   normals_s;


const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3   floor_normals[6];


const int axis_NumVertices = 6;
point4 axis_points[axis_NumVertices];
color4 axis_colors[axis_NumVertices];

/*----- Shader Lighting Parameters -----*/
color4 global_light_ambient(1.0, 1.0, 1.0, 1.0);
color4 directional_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 directional_light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 directional_light_specular(0.2, 0.2, 0.2, 1.0);
color4 sp_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 sp_light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 sp_light_specular(1.0, 1.0, 1.0, 1.0);
color4 fogcolor(0.7, 0.7, 0.7, 0.5);

color4 floor_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 floor_diffuse( 0.0, 1.0, 0.0, 1.0 );
color4 floor_specular( 0.0, 0.0, 0.0, 1.0 );

color4 sphere_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 sphere_diffuse( 1.0, 0.84, 0.0, 1.0 );
color4 sphere_specular( 1.0, 0.84, 0.0, 1.0 );
float  shininess = 125.0;
//float denstiy =
float const_att =  2.0;
float linear_att = 0.01;
float quad_att = 0.001;
vec3 light_from(0.1, 0.0, -1.0);
//vec4 light_from_i;
point4 light_position(-14.0, 12.0, -3.0, 1.0);
// In World frame.
// Needs to transform it to Eye Frame
// before sending it to the shader(s).

color4 g_floor_ambient_product = global_light_ambient * floor_ambient;
color4 d_floor_ambient_product = directional_light_ambient * floor_ambient;
color4 d_floor_diffuse_product = directional_light_diffuse * floor_diffuse;
color4 d_floor_specular_product = directional_light_specular * floor_specular;
color4 s_floor_ambient_product = sp_light_ambient * floor_ambient;
color4 s_floor_diffuse_product = sp_light_diffuse * floor_diffuse;
color4 s_floor_specular_product = sp_light_specular * floor_specular;

color4 g_sphere_ambient_product = global_light_ambient * sphere_ambient;
color4 d_sphere_ambient_product = directional_light_ambient * sphere_ambient;
color4 d_sphere_diffuse_product = directional_light_diffuse * sphere_diffuse;
color4 d_sphere_specular_product = directional_light_specular * sphere_specular;
color4 s_sphere_ambient_product = sp_light_ambient * sphere_ambient;
color4 s_sphere_diffuse_product = sp_light_diffuse * sphere_diffuse;
color4 s_sphere_specular_product = sp_light_specular * sphere_specular;



// Vertices of floor and axises centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( 5.0, 0.0, 8.0, 1.0),
    point4( 5.0, 0.0, -4.0, 1.0),
    point4( -5.0, 0.0, -4.0, 1.0),
    point4( -5.0, 0.0, 8.0, 1.0),
    point4( 0, 0.02, 0, 1.0),
    point4( 10, 0.02, 0, 1.0),
    point4( 0, 10, 0, 1.0),
    point4( 0, 0.02, 10, 1.0)
};
// RGBA colors
color4 vertex_colors[4] = {
    color4( 0.0, 1.0, 0.0, 1.0),  // green
    color4( 1.0, 0.0, 0.0, 1.0),  // red
    color4( 1.0, 0.0, 1.0, 1.0),  // magenta
    color4( 0.0, 0.0, 1.0, 1.0)  // blue
};
vec2 floor_texCoord[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 1.5),
    vec2(1.25, 1.5),
    
    vec2(1.25, 1.5),
    vec2(1.25, 0.0),
    vec2(0.0, 0.0),
};

// generate triangles: 1 color
void colorsphere()
{
    sphere_colors = new color4[sphere_NumVertices];
    for(int i=0; i<sphere_NumVertices; i++){
        sphere_colors[i] = color4( 1.0, 0.84, 0, 1.0);   // golden yellow;
    }
}
void shadowcolor()
{
    shadow_colors = new color4[sphere_NumVertices];
    for(int i = 0; i < sphere_NumVertices; i++){
        shadow_colors[i] = color4(0.25, 0.25, 0.25, 0.65);
    }
}
//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor(){
    //floorColor = 0;
    vec4 u = vertices[1] - vertices[0];
    vec4 v = vertices[3] - vertices[0];
    vec3 normal = normalize( cross(u, v) );
    floor_points[0] = vertices[0]; floor_normals[0] = normal; floor_colors[0] = vertex_colors[0];
    floor_points[1] = vertices[1]; floor_normals[1] = normal; floor_colors[1] = vertex_colors[0];
    floor_points[2] = vertices[2]; floor_normals[2] = normal; floor_colors[2] = vertex_colors[0];
    
    floor_points[3] = vertices[2]; floor_normals[3] = normal; floor_colors[3] = vertex_colors[0];
    floor_points[4] = vertices[3]; floor_normals[4] = normal; floor_colors[4] = vertex_colors[0];
    floor_points[5] = vertices[0]; floor_normals[5] = normal; floor_colors[5] = vertex_colors[0];
}
void axis(){
    axis_colors[0] = vertex_colors[1]; axis_points[0] = vertices[4];
    axis_colors[1] = vertex_colors[1]; axis_points[1] = vertices[5];
    
    axis_colors[2] = vertex_colors[2]; axis_points[2] = vertices[4];
    axis_colors[3] = vertex_colors[2]; axis_points[3] = vertices[6];
    
    axis_colors[4] = vertex_colors[3]; axis_points[4] = vertices[4];
    axis_colors[5] = vertex_colors[3]; axis_points[5] = vertices[7];
}
void image_set_up(void)
{
    int i, j, c;
    
    /* --- Generate checkerboard image to the image array ---*/
    for (i = 0; i < ImageHeight; i++)
        for (j = 0; j < ImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            
            if (c == 1) /* white */
            {
                c = 255;
                Image[i][j][0] = (GLubyte) c;
                Image[i][j][1] = (GLubyte) c;
                Image[i][j][2] = (GLubyte) c;
            }
            else  /* green */
            {
                Image[i][j][0] = (GLubyte) 0;
                Image[i][j][1] = (GLubyte) 150;
                Image[i][j][2] = (GLubyte) 0;
            }
            
            Image[i][j][3] = (GLubyte) 255;
        }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /*--- Generate 1D stripe image to array stripeImage[] ---*/
    for (j = 0; j < stripeImageWidth; j++) {
        /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
         When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
         */
        stripeImage[4*j] = (GLubyte)    255;
        stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
        stripeImage[4*j+2] = (GLubyte) 0;
        stripeImage[4*j+3] = (GLubyte) 255;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
//firework-----------------------------------------------------------------------------
const int Firework_NumVertices = 300;
vec3 fireworkColors[Firework_NumVertices];
vec3 fireworkVelocity[Firework_NumVertices];

void Firework()
{
    float Vx, Vy, Vz;
    float R, G, B;
    for (int i = 0; i < Firework_NumVertices; i++) {
        Vx = 2.0 * ( (rand() % 256) / 256.0 - 0.5 );
        Vy = 1.2 * 2.0 * ( (rand() % 256) / 256.0 );
        Vz = 2.0 * ( (rand() % 256) / 256.0 - 0.5 );
        fireworkVelocity[i] = vec3(Vx, Vy, Vz);
        R = (rand() % 256) / 256.0;
        G = (rand() % 256) / 256.0;
        B = (rand() % 256) / 256.0;
        fireworkColors[i] = vec3(R, G, B);
    }
}
//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    image_set_up();
    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &checkerboard);      // Generate texture obj name(s)
    
    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_2D, checkerboard); // Bind the texture to this texture unit
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
    
    //stripe
    glGenTextures(1, &stripe);      // Generate texture obj name(s)
    glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 0
    
    glBindTexture(GL_TEXTURE_1D, stripe);// Bind the texture to this texture unit
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
    
    colorsphere();
    // Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point4)*sphere_NumVertices + sizeof(color4)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point4) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices,
                    sizeof(color4) * sphere_NumVertices,
                    sphere_colors);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices,
                    sizeof(vec3) * sphere_NumVertices,
                    normals_f);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices,
                    sizeof(vec3) * sphere_NumVertices,
                    normals_s);
    
    
    // Create and initialize a vertex buffer object for shadow, to be used in display()
    shadowcolor();
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point4)*sphere_NumVertices + sizeof(color4)*sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point4) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point4) * sphere_NumVertices,
                    sizeof(color4) * sphere_NumVertices,
                    shadow_colors);
    
    floor();
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals) + sizeof(floor_texCoord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_normals), floor_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals), sizeof(floor_texCoord), floor_texCoord);
    
    axis();
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);
    
    Firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fireworkVelocity) + sizeof(fireworkColors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireworkVelocity), fireworkVelocity);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworkVelocity), sizeof(fireworkColors), fireworkColors);
    // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader53.glsl", "fshader53.glsl");
    firework = InitShader("vfirework.glsl", "ffirework.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 0.0 );
    glLineWidth(2.0);
    glPointSize(3.0);
}
//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------
void Lighting_floor(mat4 mv)
{
    glUniform4fv( glGetUniformLocation(program, "gAmbientProduct"),
                 1, g_floor_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "dAmbientProduct"),
                 1, d_floor_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "dDiffuseProduct"),
                 1, d_floor_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "dSpecularProduct"),
                 1, d_floor_specular_product );
    glUniform4fv( glGetUniformLocation(program, "sAmbientProduct"),
                 1, s_floor_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "sDiffuseProduct"),
                 1, s_floor_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "sSpecularProduct"),
                 1, s_floor_specular_product );
    point4 tp = vec4(-6.0, 0.0, -4.5, 1.0);
    point4 towardpoint = LookAt(eye, at, up) * tp;
    glUniform4fv( glGetUniformLocation(program, "towardpoint"),
                 1, towardpoint );
    
    //light_from_i = LookAt(eye, at, up) * light_from;
    // The Light Position in Eye Frame
    //vec4 light_position_eyeFrame = mv * light_position;
    glUniform3fv( glGetUniformLocation(program, "Lightfrom"),
                 1, light_from);
    point4 light_position_eyeFrame = LookAt(eye, at, up) * light_position;
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                 1, light_position_eyeFrame);
    
    
    glUniform1f(glGetUniformLocation(program, "exponent"),
                15.0);
    glUniform1f(glGetUniformLocation(program, "angle"),
                20*3.14/180.0);
    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
                const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
                linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
                quad_att);
    
    glUniform1f(glGetUniformLocation(program, "Shininess"),
                shininess );
}
void Lighting_sphere(mat4 mv)
{
    glUniform4fv( glGetUniformLocation(program, "gAmbientProduct"),
                 1, g_sphere_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "dAmbientProduct"),
                 1, d_sphere_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "dDiffuseProduct"),
                 1, d_sphere_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "dSpecularProduct"),
                 1, d_sphere_specular_product );
    glUniform4fv( glGetUniformLocation(program, "sAmbientProduct"),
                 1, s_sphere_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "sDiffuseProduct"),
                 1, s_sphere_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "sSpecularProduct"),
                 1, s_sphere_specular_product );
    point4 tp = vec4(-6.0, 0.0, -4.5, 1.0);
    point4 towardpoint = LookAt(eye, at, up) * tp;
    glUniform4fv( glGetUniformLocation(program, "towardpoint"), 1, towardpoint );
    glUniform3fv( glGetUniformLocation(program, "Lightfrom"), 1, light_from);
    point4 light_position_eyeFrame = LookAt(eye, at, up) * light_position;
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                 1, light_position_eyeFrame);
    
    glUniform1f(glGetUniformLocation(program, "exponent"), 15.0);
    glUniform1f(glGetUniformLocation(program, "angle"), 20*3.14/180);
    glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
    glUniform1f(glGetUniformLocation(program, "Shininess"), shininess );
}
void Fog()
{
    glUniform1i(glGetUniformLocation(program, "fogoption"), fogoption );
    glUniform4fv( glGetUniformLocation(program, "fogcolor"), 1, fogcolor );
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawSphere(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glUniform1i(glGetUniformLocation(program, "stripe"), 1);
    glUniform1i(glGetUniformLocation(program, "texturepart"), 2);
    glUniform1i(glGetUniformLocation(program, "spaceoption"), spaceoption);
    glUniform1i(glGetUniformLocation(program, "latticeoption"), latticeoption);
    glUniform1i(glGetUniformLocation(program, "latticedir"), latticedir);
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    if(wireFlag == 1 || lightingFlag == 0){
        if(wireFlag == 1){
            glUniform1i(glGetUniformLocation(program, "texsphere_flag"), 0);
        }
        else{
            glUniform1i(glGetUniformLocation(program, "texsphere_flag"), texsphere_flag);
            glUniform1i(glGetUniformLocation(program, "texsphere_dir"), texsphere_dir);
        }
        sphereColor = 1;
        glUniform1i(glGetUniformLocation(program, "colorFlag"), sphereColor);
        GLuint vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices) );
        // the offset is the (total) size of the previous vertex attribute array(s)
        /* Draw a sequence of geometric objs (triangles) from the vertex buffer
         (using the attributes specified in each enabled vertex attribute array) */
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glDisableVertexAttribArray(vColor);
    }
    else{
        sphereColor = 0;
        glUniform1i(glGetUniformLocation(program, "colorFlag"), sphereColor);
        glUniform1i(glGetUniformLocation(program, "texsphere_flag"), texsphere_flag);
        glUniform1i(glGetUniformLocation(program, "texsphere_dir"), texsphere_dir);
        glUniform1i(glGetUniformLocation(program, "lightoption"), lightoption);
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        if(shadingFlag == 0)
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices) );
        if(shadingFlag == 1)
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                                BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices) );
        // the offset is the (total) size of the previous vertex attribute array(s)
        
        /* Draw a sequence of geometric objs (triangles) from the vertex buffer
         (using the attributes specified in each enabled vertex attribute array) */
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glDisableVertexAttribArray(vNormal);
    }
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    
}
void drawFloor(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    glUniform1i(glGetUniformLocation(program, "texground_flag"), texground_flag);
    glUniform1i(glGetUniformLocation(program, "checkerboard"), 0);
    glUniform1i(glGetUniformLocation(program, "texturepart"), 1);
    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3)*num_vertices));
    
    if(lightingFlag == 0){
        //floorColor = 1;
        glUniform1i(glGetUniformLocation(program, "colorFlag"), 1);
        GLuint vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(sizeof(point4) * num_vertices) );
        // the offset is the (total) size of the previous vertex attribute array(s)
        /* Draw a sequence of geometric objs (triangles) from the vertex buffer
         (using the attributes specified in each enabled vertex attribute array) */
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glDisableVertexAttribArray(vColor);
    }
    if(lightingFlag == 1){
        //floorColor = 0;
        glUniform1i(glGetUniformLocation(program, "colorFlag"), 0);
        glUniform1i(glGetUniformLocation(program, "lightoption"), lightoption);
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices) );
        // the offset is the (total) size of the previous vertex attribute array(s)
        
        /* Draw a sequence of geometric objs (triangles) from the vertex buffer
         (using the attributes specified in each enabled vertex attribute array) */
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glDisableVertexAttribArray(vNormal);
    }
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vTexCoord);
}
void drawShadow(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    glUniform1i(glGetUniformLocation(program, "texturepart"), 3);
    glUniform1i(glGetUniformLocation(program, "latticeoption"), latticeoption);
    glUniform1i(glGetUniformLocation(program, "latticedir"), latticedir);
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    glUniform1i(glGetUniformLocation(program, "colorFlag"), 1);
    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point4) * num_vertices) );
    // the offset is the (total) size of the previous vertex attribute array(s)
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    glDisableVertexAttribArray(vColor);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    
}
void drawLine(GLuint buffer, int num_vertices){
    colorFlag = 1;
    glUniform1i(glGetUniformLocation(program, "texturepart"), 0);
    glUniform1i(glGetUniformLocation(program, "colorFlag"), colorFlag);
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices) );
    // the offset is the (total) size of the previous vertex attribute array(s)
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_LINES, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}
void drawFirework(GLuint buffer, int num_vertices){
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint fireworkVelocity = glGetAttribLocation(firework, "fireworkVelocity");
    glEnableVertexAttribArray(fireworkVelocity);
    glVertexAttribPointer(fireworkVelocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint fireworkColors = glGetAttribLocation(firework, "fireworkColors");
    glEnableVertexAttribArray(fireworkColors);
    glVertexAttribPointer(fireworkColors, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec3)*num_vertices));
    // the offset is the (total) size of the previous attribute array(s)
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_POINTS, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(fireworkVelocity);
    glDisableVertexAttribArray(fireworkColors);
}
//----------------------------------------------------------------------------
void display( void )
{
    
    GLuint  ModelView;  // model-view matrix uniform shader variable location
    GLuint  Projection;  // projection matrix uniform shader variable location
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glUseProgram(program); // Use the shader program
    
    ModelView = glGetUniformLocation(program, "ModelView" );
    Projection = glGetUniformLocation(program, "Projection" );
    
    /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    
    /*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    Fog();
    
    vec4    row1(12.0, 14.0, 0.0, 0.0);
    vec4    row2(0.0, 0.0, 0.0, 0.0);
    vec4    row3(0.0, -3.0, 12.0, 0.0);
    vec4    row4(0.0, -1.0, 0.0, 12.0);
    
    mat4    proj = mat4(row1, row2, row3, row4);
    mat4    fmv = LookAt(eye, at, up);
    mat4    mv = LookAt(eye, at, up);
    mat4    smv = LookAt(eye, at, up)*proj;
    
    if(rollingFlag == 0){
        mv = LookAt(eye, at, up)*Translate(3, 1, 5);
        smv = LookAt(eye, at, up)*proj*Translate(3, 1, 5);
    }
    if(rollingFlag == 1){
    vec4    ab(-5/AB, 0, -7.5/AB, 0);
    vec4    bc(4/BC, 0, -1.5/BC, 0);
    vec4    ca(1/CA, 0, 9/CA, 0);
    vec4    oy(0,1,0,0);
    vec4    trans;
    
    
    
    d = angle*2*3.14*1/360;
    dnext = d+2*3.14*1/360;
    
    if(segmentflag == 1){  //AB
        trans = vec4(3+d*ab[0], 1+d*ab[1], 5+d*ab[2], 0);
        mv = mv * Translate(trans)* Rotate(angle, -7.5, 0, 5)*accR;
        smv = smv * Translate(trans)* Rotate(angle, -7.5, 0, 5)*accR;
        if(dnext>AB){
            //cout<<"AB update"<<endl;
            accR = Rotate(angle, -7.5, 0, 5)*accR;
            angle = 0.0;
            segmentflag = 2;
        }
    }
    else if(segmentflag == 2){  //BC
        trans = vec4(-2+d*bc[0], 1+d*bc[1], -2.5+d*bc[2], 0);
        mv = mv * Translate(trans)* Rotate(angle, -1.5, 0, -4)*accR;
        smv = smv * Translate(trans)* Rotate(angle, -7.5, 0, 5)*accR;
        //cout<<dnext<<" ";
        if(dnext>BC){
            //cout<<"BC update"<<endl;
            accR = Rotate(angle, -1.5, 0, -4)*accR;
            angle = 0.0;
            segmentflag = 3;
        }
    }
    else if(segmentflag == 3){  //CA
        trans = vec4(2+d*ca[0], 1+d*ca[1], -4+d*ca[2], 0);
        mv = mv * Translate(trans)* Rotate(angle, 9, 0, -1)*accR;
        smv = smv * Translate(trans)* Rotate(angle, -7.5, 0, 5)*accR;
        if(dnext>CA){
            //cout<<"CA update"<<endl;
            accR = Rotate(angle, 9, 0, -1)*accR;
            angle = 0.0;
            segmentflag = 1;
        }
    }
    }
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    
    //draw sphere
    if(wireFlag == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if(wireFlag == 0) {
        if(lightingFlag == 1){
            Lighting_sphere(mv);
        }
        if(lightingFlag == 0){}
        // Set up the Normal Matrix from the model-view matrix
        mat3 normal_matrix = NormalMatrix(mv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                           1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    drawSphere(sphere_buffer, sphere_NumVertices);  // draw the sphere
    
    
    //draw floor
    if(lightingFlag == 1){
        Lighting_floor(fmv);
    }
    if(lightingFlag == 0){}
    
    if(shadowFlag == 1 && eye[1] > 0){
        //disable to z-buffer
        glDepthMask(GL_FALSE);
        //render floor
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, fmv); // GL_TRUE: matrix is row-major
        mat3 normal_matrix = NormalMatrix(fmv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawFloor(floor_buffer, floor_NumVertices);  // draw the floor
        // draw the shadow
        if(blendflag == 1){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, smv); // GL_TRUE: matrix is row-major
        if(wireFlag == 0)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if(wireFlag == 1)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawShadow(shadow_buffer, sphere_NumVertices);  // draw the shadow
        if(blendflag == 1)
            glDisable(GL_BLEND);
        // restore floor in z_buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_TRUE);
        //render floor
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, fmv); // GL_TRUE: matrix is row-major
        normal_matrix = NormalMatrix(fmv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawFloor(floor_buffer, floor_NumVertices);
        //enable to frame buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    else{
        //render floor
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, fmv); // GL_TRUE: matrix is row-major
        mat3 normal_matrix = NormalMatrix(fmv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawFloor(floor_buffer, floor_NumVertices);  // draw the floor
    }
    
    //draw axis
    mv = LookAt(eye, at, up);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawLine(axis_buffer, axis_NumVertices);
    
    //draw firework
    if(fireworkflag == 1){
        glUseProgram(firework); // Use the shader program
        GLuint  MV;  // model-view matrix uniform shader variable location
        GLuint  Pj;  // projection matrix uniform shader variable location
        
        MV = glGetUniformLocation(firework, "ModelView" );
        Pj = glGetUniformLocation(firework, "Projection" );
        
        /*---  Set up and pass on Projection matrix to the shader ---*/
        mat4  p = Perspective(fovy, aspect, zNear, zFar);
        glUniformMatrix4fv(Pj, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
        glUniformMatrix4fv(MV, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        float time = glutGet(GLUT_ELAPSED_TIME) - timesub;
        while (time >= 8000.0) {
            time = time - 8000;
        }
        glUniform1f( glGetUniformLocation(firework, "t"), time );
        drawFirework(firework_buffer, Firework_NumVertices);
    }
    
    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    if(rollingFlag == 0){
        glutPostRedisplay();
    };
    
    if(rollingFlag == 1){
        //angle += 0.05;
        angle += 1;    //YJC: change this value to adjust the cube rotation speed.
        glutPostRedisplay();
        
    }
}
//----------------------------------------------------------------------------
void myMouse(int button, int state, int x, int y){
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        if(rollingFlag == 0){}
        if(rollingFlag == 1){
            animationFlag = 1 -  animationFlag;
            if (animationFlag == 1) glutIdleFunc(idle);
            else                    glutIdleFunc(NULL);
            glutPostRedisplay();
        }
    }
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 033: // Escape Key
        case 'q': case 'Q':
        exit( EXIT_SUCCESS );
        break;
        
        case 'X': eye[0] += 1.0; break;
        case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
        case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
        case 'z': eye[2] -= 1.0; break;
        
        case 'b': case 'B':
            rollingFlag = 1;
        case 'v': case 'V':
            texsphere_dir = 0; break;
        case 's': case 'S':
            texsphere_dir = 1; break;
        case 'o': case 'O':
            spaceoption = 0; break;
        case 'e': case 'E':
            spaceoption = 1; break;
        case 'l': case 'L':
            latticeoption = 1 - latticeoption; break;
        case 'u': case 'U':
            latticedir = 0; break;
        case 't': case 'T':
            latticedir = 1; break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void menu(int id){
    switch (id) {
        case 1:
            eye = init_eye;
            break;
        
        case 2:
            exit(0);
            break;
        
        case 3:
            wireFlag = 1;
            
    }
    glutPostRedisplay();
}
void menu_shadow(int id){
    switch (id) {
        case 4:
            shadowFlag = 0; //No shadow
            break;
            
        case 5:
            shadowFlag = 1; //shadow
            break;
    }
    glutPostRedisplay();
}
void menu_lighting(int id){
    switch (id) {
        case 6:
            lightingFlag = 0;
            break;
            
        case 7:
            lightingFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void menu_shading(int id){
    switch (id) {
        case 8:
            shadingFlag = 0;
            break;
            
        case 9:
            shadingFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void menu_lightoption(int id){
    switch (id) {
        case 10:
            lightoption = 1;
            break;
            
        case 11:
            lightoption = 2;
            break;
    }
    glutPostRedisplay();
}
void menu_fog(int id){
    switch (id) {
        case 12:
            fogoption = 0;
            break;
        case 13:
            fogoption = 1;
            break;
        case 14:
            fogoption = 2;
            break;
        case 15:
            fogoption = 3;
            break;
    }
    glutPostRedisplay();
}
void menu_blendshadow(int id){
    switch (id) {
        case 16:
            blendflag = 0;
            break;
            
        case 17:
            blendflag = 1;
            break;
    }
    glutPostRedisplay();
}
void menu_texground(int id){
    switch (id) {
        case 18:
            texground_flag = 0;
            break;
        case 19:
            texground_flag = 1;
            break;
    }
    glutPostRedisplay();
}
void menu_texsphere(int id){
    switch (id) {
        case 20:
            texsphere_flag = 0;
            break;
        case 21:
            texsphere_flag = 1;
            break;
        case 22:
            texsphere_flag = 2;
            break;
    }
    glutPostRedisplay();
}
void menu_firework(int id){
    switch (id) {
        case 23:{
            if(fireworkflag == 1) timesub =  glutGet(GLUT_ELAPSED_TIME);
            fireworkflag = 0;
            break;
        }
            
        case 24:
            if(fireworkflag == 0) timesub =  glutGet(GLUT_ELAPSED_TIME);
            fireworkflag = 1;
            break;
    }
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void in_file(void){
    vec4 u;
    vec4 v;
    
    vec3 normal_f;
    vec3 normal_s;
    int i;
    cout<<"the name of the input file"<<endl;
    cin.getline(filename,20);
    ifstream infile;
    infile.open(filename);
    infile>>sphere_NumTriangles;
    cout << "Reading from the file" << sphere_NumTriangles<<endl;
    int three;
    sphere_NumVertices = sphere_NumTriangles*3;
    sphere_points = new point4[sphere_NumVertices];
    normals_f = new vec3[sphere_NumVertices];
    normals_s = new vec3[sphere_NumVertices];
    for(i=0; i<sphere_NumTriangles; i++){
        infile>>three;
        for(int j=0; j<three; j++){
            infile>>sphere_points[i*3+j].x>>sphere_points[i*3+j].y>>sphere_points[i*3+j].z;
            sphere_points[i*3+j].w = 1;
        }
        // generates one triangles for each face and assigns normals to the vertices
        u = sphere_points[i*3+1] - sphere_points[i*3];
        v = sphere_points[i*3+2] - sphere_points[i*3];
        normal_f = normalize( cross(u, v) );
        normals_f[i*3] = normal_f;
        normals_f[i*3+1] = normal_f;
        normals_f[i*3+2] = normal_f;
        
        normals_s[i*3] = vec3(sphere_points[i*3].x,sphere_points[i*3].y,sphere_points[i*3].z);
        normals_s[i*3+1] = vec3(sphere_points[i*3+1].x,sphere_points[i*3+1].y,sphere_points[i*3+1].z);
        normals_s[i*3+2] = vec3(sphere_points[i*3+2].x,sphere_points[i*3+2].y,sphere_points[i*3+2].z);

    }
    infile.close();
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    srand(time(0));
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Rolling Sphere");
    
#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err));
        exit(1);
    }
#endif
    
    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
    
    in_file();
    int sub_shadow;
    int sub_lighting;
    int sub_shading;
    int sub_lightoption;
    int sub_fog;
    int sub_blend;
    int sub_texground;
    int sub_texsphere;
    int sub_firework;
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMouseFunc(myMouse);
    glutKeyboardFunc(keyboard);
    
    sub_shadow = glutCreateMenu(menu_shadow);
    glutAddMenuEntry("NO", 4);
    glutAddMenuEntry("YES", 5);
    sub_lighting = glutCreateMenu(menu_lighting);
    glutAddMenuEntry("No", 6);
    glutAddMenuEntry("Yes", 7);
    sub_shading = glutCreateMenu(menu_shading);
    glutAddMenuEntry("flat shading", 8);
    glutAddMenuEntry("smooth shading", 9);
    sub_lightoption = glutCreateMenu(menu_lightoption);
    glutAddMenuEntry("spot light", 10);
    glutAddMenuEntry("point source", 11);
    sub_fog = glutCreateMenu(menu_fog);
    glutAddMenuEntry("no fog", 12);
    glutAddMenuEntry("linear", 13);
    glutAddMenuEntry("exponetial", 14);
    glutAddMenuEntry("exponetial square", 15);
    sub_blend = glutCreateMenu(menu_blendshadow);
    glutAddMenuEntry("NO", 16);
    glutAddMenuEntry("YES", 17);
    sub_texground = glutCreateMenu(menu_texground);
    glutAddMenuEntry("NO", 18);
    glutAddMenuEntry("Yes", 19);
    sub_texsphere = glutCreateMenu(menu_texsphere);
    glutAddMenuEntry("NO", 20);
    glutAddMenuEntry("Yes - Contour Lines", 21);
    glutAddMenuEntry("Yes - Checkerboard", 22);
    sub_firework = glutCreateMenu(menu_firework);
    glutAddMenuEntry("NO", 23);
    glutAddMenuEntry("Yes", 24);
    
    glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddMenuEntry("Quit", 2);
    glutAddMenuEntry("Wire Frame Sphere", 3);
    glutAddSubMenu("Shadow", sub_shadow);
    glutAddSubMenu("Enable Lighting", sub_lighting);
    glutAddSubMenu("Shading", sub_shading);
    glutAddSubMenu("Light Source", sub_lightoption);
    glutAddSubMenu("Fog Options", sub_fog);
    glutAddSubMenu("Blending Shadow", sub_blend);
    glutAddSubMenu("Texture Mapped Ground", sub_texground);
    glutAddSubMenu("Texture Mapped Sphere", sub_texsphere);
    glutAddSubMenu("Firework", sub_firework);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    
    init();
    glutMainLoop();
    return 0;
}
