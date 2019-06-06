
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
//#include <GL/glew.h>

#include <iostream>
#include <iomanip>
#include <opencv2/core.hpp>    // include OpenCV core headers
#include <opencv2/imgproc.hpp> // include image processing headers
#include <opencv2/highgui.hpp> // include GUI-related headers


#include "PoseEstimation.h"
#include "MarkerTracker.h"
#include "DrawPrimitives.h"
#define PI 3.1415926535
vector < vector < int > > maze;

// Added in Exercise 9 - Start *****************************************************************

struct Position { double x,y,z; };

bool debugmode = false;
bool balldebug = false;

// The vector of plane
float xangle = 0.f;
float yangle = 0.f;
float zangle = 0.f;
int plane_flag[3] = {0, 0, 0};	// flag of x, y, z with 0: stop 1: increase -1: decrease
float angle_unit = 30.f / 60.0f; // angle change with one keyboard press

// The position of ball
float ballpos[3] = { -0.0125f, 0.0125f, 0.001f };
float ball_speed[3] = { .0f, .0f, .0f };

int ball_flag[3] = {0, 0, 0};	// flag of x, y, z with 0: stop 1: increase -1: decrease
float ball_speed_unit = 0.0001f;
float ball_radius = 0.0004f;

float resultTransposedMatrix[16];
float snowmanLookVector[4];
int towards = 0x0690;
int towardsList[2] = {0x0690, 0x0690};
int towardscounter = 0;
// Position ballpos;
int ballspeed = 100;
// Added in Exercise 9 - End *****************************************************************

//camera settings
const int camera_width  = 1280;
const int camera_height = 720;
unsigned char bkgnd[camera_width*camera_height*3];

void InitializeVideoStream( cv::VideoCapture &camera ) {
   if( camera.isOpened() )
       camera.release();

   camera.open(0); // open the default camera
   if ( camera.isOpened()==false ) {
       std::cout << "No webcam found, using a video file" << std::endl;
       camera.open("MarkerMovie.mpg");
       if ( camera.isOpened()==false ) {
           std::cout << "No video file found. Exiting."      << std::endl;
           exit(0);
       }
   }
}

// Added in Exercise 9 - Start *****************************************************************
void multMatrix(float mat[16], float vec[4])
{
   for(int i=0; i<4; i++)
   {
       snowmanLookVector[i] = 0;
       for(int j=0; j<4; j++)
             snowmanLookVector[i] += mat[4*i + j] * vec[j];
   }
}
// Added in Exercise 9 - End *****************************************************************

/* program & OpenGL initialization */
void initGL(int argc, char *argv[])
{
// initialize the GL library
// Added in Exercise 8 - End *****************************************************************
   // pixel storage/packing stuff
   glPixelStorei(GL_PACK_ALIGNMENT, 1);// for glReadPixels
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for glTexImage2D
   glPixelZoom(1.0, -1.0);
// Added in Exercise 8 - End *****************************************************************
   // enable and set colors
   glEnable( GL_COLOR_MATERIAL );
   glClearColor( 0, 0, 0, 1.0 );

   // enable and set depth parameters
   glEnable( GL_DEPTH_TEST );
   glClearDepth( 1.0 );

   // light parameters
   GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
   GLfloat light_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
   GLfloat light_dif[] = { 0.7f, 0.7f, 0.7f, 1.0f };

   // enable lighting
   glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
   glLightfv( GL_LIGHT0, GL_AMBIENT,  light_amb );
   glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_dif );
   glEnable( GL_LIGHTING );
   glEnable( GL_LIGHT0 );
}

void computeAnglesFromMatrix()
{
 
    float sy = sqrt(resultTransposedMatrix[0] * resultTransposedMatrix[0] +  resultTransposedMatrix[1] * resultTransposedMatrix[1] );
 
    bool singular = sy < 1e-6; // If
 
    if (!singular)
    {
        xangle = -atan2(resultTransposedMatrix[6] , resultTransposedMatrix[10])*180/PI;
        yangle = atan2(-resultTransposedMatrix[2], sy)*180/PI;
        zangle = atan2(resultTransposedMatrix[1], resultTransposedMatrix[0])*180/PI;
        
    }
    else
    {
        xangle = -atan2(-resultTransposedMatrix[9], resultTransposedMatrix[5])*180/PI;
        yangle = atan2(-resultTransposedMatrix[2], sy)*180/PI;
        zangle = 0.0;
    }
}

void checkSpeedWithGravity() {
	float g = 9.8f;		// gravity constant
	float pi = acos(-1);
    printf("%f \n", yangle);
	// calculate one by one. @magicall different between plane and ball.........
	ball_speed[0] += 1.0f * sinf(yangle/180.0f*pi) * g * ball_speed_unit;
	ball_speed[1] += -1.0f * sinf(xangle/180.0f*pi) * g * ball_speed_unit;
    
}

void checkCollision_i(float add_v_i){
    
    float ballpos_new_i = ballpos[1]+add_v_i;
    float ballpos_up = ballpos_new_i+ball_radius;
    float ballpos_down = ballpos_new_i-ball_radius;
    
    int j_idx = (int)(round((ballpos[0] + 0.0135f) / 0.001f));
    int i_idx_new = (int)(round((ballpos_new_i - 0.0135f) / (-0.001f)));
    
    if(add_v_i < 0){//down
        i_idx_new = (int)(round((ballpos_down - 0.0135f) / (-0.001f)));
        
        if(maze[j_idx][i_idx_new] == 1){
            float upper_bound = 0.0135f - (i_idx_new)*(0.001f) + 0.0005f;
            if(balldebug)
                cout << "upperbd " << upper_bound << ' ' << ballpos_down << endl;
            if(ballpos_down < upper_bound){
                if(balldebug)
                    cout << "i col (down dir)!" << endl;
                ball_speed[1] = -0.5f*ball_speed[1];
            }
            else{
                ballpos[1] += add_v_i;
            }
        }
        else{
            ballpos[1] += add_v_i;
        }
        
    }
    else if(add_v_i > 0){//up
        i_idx_new = (int)(round((ballpos_up - 0.0135f) / (-0.001f)));
        
        if(maze[j_idx][i_idx_new] == 1){
            float lower_bound = 0.0135f - (i_idx_new)*(0.001f) - 0.0005f;
            if(balldebug)
                cout << "lower_bound " << lower_bound << " ballpos_up " << ballpos_up << " i_idx_new " << (ballpos_up - 0.0135f) / (-0.001f) << endl;
            if(ballpos_up > lower_bound){
                if(balldebug)
                    cout << "i col (up dir)!" << endl;
                ball_speed[1] = -0.5f*ball_speed[1];
            }
            else{
                ballpos[1] += add_v_i;
            }
        }
        else{
            ballpos[1] += add_v_i;
        }
        
    }
    
    if(balldebug)
        cout << "i: j_idx " << j_idx << ' ' << ballpos[0] << ' ' << (ballpos[0] + 0.0135f) / 0.001f << " i_idx " << i_idx_new << endl;
    
}


void checkCollision_j(float add_v_j){
    
    float ballpos_new_j = ballpos[0]+add_v_j;
    float ballpos_left = ballpos_new_j-ball_radius;
    float ballpos_right = ballpos_new_j+ball_radius;
    
    int i_idx = (int)(round((ballpos[1] - 0.0135f) / (-0.001f)));
    int j_idx_new = (int)(round((ballpos_new_j + 0.0135f) / (0.001f)));
    
    if(add_v_j < 0){//left
        j_idx_new = (int)(round((ballpos_left + 0.0135f) / (0.001f)));
        
        if(maze[j_idx_new][i_idx] == 1 || j_idx_new == 0){
            float right_bound = -0.0135f + (j_idx_new)*(0.001f) + 0.0005f;
            if(balldebug)
                cout << "right_bound " << right_bound << ' ' << ballpos_left << endl;
            if(ballpos_left < right_bound){
                if(balldebug)
                    cout << "j col (left dir)!" << endl;
                ball_speed[0] = -0.5f*ball_speed[0];
            }
            else{
                ballpos[0] += add_v_j;
            }
        }
        else{
            ballpos[0] += add_v_j;
        }
        
    }
    else if(add_v_j > 0){//right
        j_idx_new = (int)(round((ballpos_right + 0.0135f) / (0.001f)));
        
        if(maze[j_idx_new][i_idx] == 1){
            float left_bound = -0.0135f + (j_idx_new)*(0.001f) - 0.0005f;
            if(balldebug)
                cout << "left_bound " << left_bound << ' ' << ballpos_right << endl;
            if(ballpos_right > left_bound){
                if(balldebug)
                    cout << "j col (right dir)!" << endl;
                ball_speed[0] = -0.5f*ball_speed[0];
            }
            else{
                ballpos[0] += add_v_j;
            }
        }
        else{
            ballpos[0] += add_v_j;
        }
    }
    
    if(balldebug)
        cout << "j "   << ballpos_new_j << ' ' << j_idx_new << ' '<< i_idx << endl;
}


void ball_movement() {
    float framerate = 60.0f;
    
    // manipulate the speed by keyboard
    ball_speed[0] += ball_flag[0] * ball_speed_unit;
    ball_speed[1] += ball_flag[1] * ball_speed_unit;
    
    checkSpeedWithGravity();
    
    if(balldebug)
        cout << "1-------------------------------------\n";
    if(balldebug)
        cout << "-j-----------------------------------\n";
    checkCollision_j(0.5f * ball_speed[0]/ framerate);
    if(balldebug)
        cout << "-i------------------------------------\n";
    checkCollision_i(0.5f * ball_speed[1]/ framerate);
    
    //    ballpos[2] += 0.5f * ball_speed[2] / framerate;
    if(balldebug)
        cout << "2-------------------------------------\n"; 
}


void display(GLFWwindow* window, const cv::Mat &img_bgr, std::vector<Marker> &markers)
{
   const auto camera_image_size = sizeof(unsigned char) *img_bgr.rows*img_bgr.cols * 3;
   auto background_buffer_size = sizeof(bkgnd);
   memcpy(bkgnd, img_bgr.data, background_buffer_size);


   int width0, height0;
   glfwGetFramebufferSize(window, &width0, &height0);
//    reshape(window, width, height);

   // clear buffers
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // draw background image
   glDisable( GL_DEPTH_TEST );

   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D( 0.0, camera_width, 0.0, camera_height );

   glRasterPos2i( 0, camera_height-1 );
   //GL_BGR_EXT => GL_RGB
   glDrawPixels( camera_width, camera_height, GL_RGB, GL_UNSIGNED_BYTE,  bkgnd);

   glPopMatrix();

   glEnable(GL_DEPTH_TEST);

//    return;

   // move to marker-position
   glMatrixMode( GL_MODELVIEW );

// Added in Exercise 9 - Start *****************************************************************
   float resultMatrix_005A[16];
   float resultMatrix_0272[16];
   for(int i=0; i<markers.size(); i++){
       const int code =markers[i].code;
       if(code == 0x0690) {
           for(int j=0; j<16; j++)
               resultMatrix_005A[j] = markers[i].resultMatrix[j];
       }
    //    else if(code == 0x02690){
    //        for(int j=0; j<16; j++)
    //            resultMatrix_0272[j] = markers[i].resultMatrix[j];
    //    }
   }


   for (int x=0; x<4; ++x)
       for (int y=0; y<4; ++y)
           resultTransposedMatrix[x*4+y] = resultMatrix_005A[y*4+x];

   float scale = 0.5;
   resultTransposedMatrix[12] *= scale;  // x方向のスケール調整
   resultTransposedMatrix[13] *= scale;  // y方向のスケール調整
// Added in Exercise 9 - End *****************************************************************

//    glLoadTransposeMatrixf( resultMatrix );
   glLoadMatrixf( resultTransposedMatrix );
   computeAnglesFromMatrix();
   glRotatef(-180.f, 1.f, 0.f, 0.f);
   drawMaze(maze);
    // Sphere(ball) event
    glColor4f(0,0,1,0.5);
    ball_movement();
    glTranslatef(ballpos[0], ballpos[1], ballpos[2]);
    drawSphere( ball_radius, 10, 10 );

   int key = cv::waitKey (10);
   if (key == 27) exit(0);
   // Added in Exercise 9 - Start *****************************************************************
   else if (key == 100) debugmode = !debugmode;
   else if (key == 98) balldebug = !balldebug;
   // Added in Exercise 9 - End *****************************************************************

}

void reshape( GLFWwindow* window, int width, int height ) {

   // set a whole-window viewport
   glViewport( 0, 0, (GLsizei)width, (GLsizei)height );

   // create a perspective projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective( 30, ((GLfloat)width/(GLfloat)height), 0.01, 100 );

   // invalidate display
   //glutPostRedisplay();
}

int main(int argc, char* argv[]) {

   cv::VideoCapture cap;

   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
       return -1;


   // initialize the window system
   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(camera_width/2, camera_height/2, "Exercise 8 - Combine", NULL, NULL);
   if (!window)
   {
       glfwTerminate();
       return -1;
   }

   // Set callback functions for GLFW
   glfwSetFramebufferSizeCallback(window, reshape);

   glfwMakeContextCurrent(window);
   glfwSwapInterval( 1 );

   int window_width, window_height;
   glfwGetFramebufferSize(window, &window_width, &window_height);
   reshape(window, window_width, window_height);

   glViewport(0, 0, window_width, window_height);

   // initialize the GL library
   initGL(argc, argv);

   maze = generate_maze();
    for (int i = 0; i < maze.size(); ++i)
    {
        for (int j = 0; j < maze[0].size(); ++j)
        {
            if (maze[j][i] == 1) {
                cout << '-';
            }
            else{
                cout << ' ';
            }
        }
        cout<<endl;
    }

   // setup OpenCV
   cv::Mat img_bgr;
   InitializeVideoStream(cap);
   const double kMarkerSize = 0.03;// 0.048; // [m]
   MarkerTracker markerTracker(kMarkerSize);

   std::vector<Marker> markers;
//    float resultMatrix[16];
   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
       markers.resize(0);
       /* Capture here */
       cap >> img_bgr;

       if(img_bgr.empty()){
           std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
           InitializeVideoStream(cap);
           cv::waitKey(1000); /// Wait for one sec.
           continue;
       }

       /* Track a marker */
       markerTracker.findMarker( img_bgr, markers);///resultMatrix);

//        cv::imshow("img_bgr", img_bgr);
//        cv::waitKey(10); /// Wait for one sec.

       /* Render here */
       display(window, img_bgr, markers);

       /* Swap front and back buffers */
       glfwSwapBuffers(window);

       /* Poll for and process events */
       glfwPollEvents();
   }

   glfwTerminate();


   return 0;

}
