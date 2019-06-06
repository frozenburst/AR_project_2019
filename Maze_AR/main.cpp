
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
vector < vector < int > > maze;

// Added in Exercise 9 - Start *****************************************************************

struct Position { double x,y,z; };

bool debugmode = false;
bool balldebug = false;

float resultTransposedMatrix[16];
float snowmanLookVector[4];
int towards = 0x0690;
int towardsList[2] = {0x0690, 0x0690};
int towardscounter = 0;
Position ballpos;
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

void moveBall(float mat[16])
{
   float vector[3];
   vector[0] = mat[3]  - (float)ballpos.x;
   vector[1] = mat[7]  - (float)ballpos.y;
   vector[2] = mat[11] - (float)ballpos.z;

   float length = sqrt( vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2] );
   if(balldebug) std::cout << length << std::endl;
   if( length < 0.01)
   {
       towards = towardsList[(towardscounter++)%2];
       if(balldebug) std::cout << "target changed to marker " << towards << std::endl;
       ballspeed = 60 + 80 * rand()/RAND_MAX;
       return;
   }
   ballpos.x += vector[0] / (ballspeed * length);
   ballpos.y += vector[1] / (ballspeed * length);
   ballpos.z += vector[2] / (ballspeed * length);

}


void rotateToMarker(float thisMarker[16], float lookAtMarker[16], int markernumber)
{
   float vector[3];
   vector[0] = lookAtMarker[3] - thisMarker[3];
   vector[1] = lookAtMarker[7] - thisMarker[7];
   vector[2] = lookAtMarker[11] - thisMarker[11];

   if(towards == markernumber) moveBall(lookAtMarker);

   //normalize vector
   float help = sqrt( vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2] );
   vector[0] /= help;
   vector[1] /= help;
   vector[2] /= help;

   if(debugmode) std::cout << "Vector: " << vector[0] << ", " << vector[1] << ", " << vector[2] << std::endl;

   float defaultLook[4] = {1,0,0,0};
   multMatrix(thisMarker, defaultLook);

   //normalize snowmanLookVector
   help = sqrt( snowmanLookVector[0]*snowmanLookVector[0] + snowmanLookVector[1]*snowmanLookVector[1] + snowmanLookVector[2]*snowmanLookVector[2] );
   snowmanLookVector[0] /= help;
   snowmanLookVector[1] /= help;
   snowmanLookVector[2] /= help;

   if(debugmode) std::cout << "SnowmanLookVector: " << snowmanLookVector[0] << ", " << snowmanLookVector[1] << ", " << snowmanLookVector[2] << std::endl;

   float angle = ((float) (180.0 / M_PI)) * acos( vector[0] * snowmanLookVector[0] + vector[1] * snowmanLookVector[1] + vector[2] * snowmanLookVector[2] );
   if((vector[0] * snowmanLookVector[1] - vector[1] * snowmanLookVector[0]) < 0 ) angle *= -1;

   if(debugmode) std::cout << "Angle: " << angle << std::endl;

   glRotatef(angle, 0, 0, 1);
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
   glRotatef(-180.f, 1.f, 0.f, 0.f);
   drawMaze(maze);
   glColor4f(0,0,0,0.5);
   glTranslatef( 0.012f, -0.0f, 0.001f );
   drawSphere( 0.0008, 10, 10 );

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
//
// #define GLFW_INCLUDE_GLU
// #define GL_SILENCE_DEPRECATION
// #include <stdio.h>
// #include <iostream>
// #include <iomanip>
// #include <opencv2/core.hpp>    // include OpenCV core headers
// #include <opencv2/imgproc.hpp> // include image processing headers
// #include <opencv2/highgui.hpp> // include GUI-related headers
// #include <GLFW/glfw3.h>
// //#include <GL/glew.h>

// #include "DrawPrimitives.h"
// vector < vector < int > > maze;

// // The vector of plane
// float xangle = 0.f;
// float yangle = 0.f;
// float zangle = 0.f;
// int plane_flag[3] = {0, 0, 0};	// flag of x, y, z with 0: stop 1: increase -1: decrease
// float angle_unit = 30.f / 60.0f; // angle change with one keyboard press

// // The position of ball
// float ballpos[3] = { -0.0125f, 0.0125f, 0.001f };
// float ball_speed[3] = { .0f, .0f, .0f };

// int ball_flag[3] = {0, 0, 0};	// flag of x, y, z with 0: stop 1: increase -1: decrease
// float ball_speed_unit = 0.0001f;
// float ball_radius = 0.0004f;

// int main_dir = -1; //-1: equal, 0: j, horizontal, 1: i, vertical

// /* program & OpenGL initialization */
// void init(int argc, char *argv[])
// {
//     // enable and set colors
//     glEnable( GL_COLOR_MATERIAL );
//     glClearColor( 0.0, 0.0, 0.0, 1.0 );

//     // enable and set depth parameters
//     glEnable( GL_DEPTH_TEST );

//     glClearDepth( 1.0 );
// }

// void checkSpeedWithGravity() {
// 	float g = 9.8f;		// gravity constant
// 	float pi = acos(-1);
    
// 	// calculate one by one. @magicall different between plane and ball.........
// 	ball_speed[0] += 1.0f * sinf(yangle/180.0f*pi) * g * ball_speed_unit;
// 	ball_speed[1] += -1.0f * sinf(xangle/180.0f*pi) * g * ball_speed_unit;
    
// }

// void checkCollision_i(float add_v_i){
    
//     float ballpos_new_i = ballpos[1]+add_v_i;
//     float ballpos_up = ballpos_new_i+ball_radius;
//     float ballpos_down = ballpos_new_i-ball_radius;
    
//     cout << ballpos_up << ' ' << ballpos_down  << endl;
    
    
//     int j_idx = (ballpos[0] + 0.0135f) / 0.001f;
//     int i_idx_new = 0;
    
//     if(add_v_i < 0){//down
//         i_idx_new = (ballpos_down - 0.0135f) / (-0.001f);
        
//         if(maze[j_idx][i_idx_new+1] == 1){
//             float upper_bound = 0.0135f - (i_idx_new+1)*(0.001f) + 0.0005f;
//             cout << "upperbd " << upper_bound << ' ' << ballpos_down << endl;
//             if(ballpos_down < upper_bound){
//                 cout << "i col (down dir)!" << endl;
//                 ball_speed[1] = -0.5f*ball_speed[1];
//             }
//             else{
//                 ballpos[1] += add_v_i;
//             }
//         }
//         else{
//             ballpos[1] += add_v_i;
//         }
        
//     }
//     else if(add_v_i > 0){//up
//         i_idx_new = (ballpos_up - 0.0135f) / (-0.001f);
        
//         if(maze[j_idx][i_idx_new] == 1){
//             float lower_bound = 0.0135f - (i_idx_new)*(0.001f) - 0.0005f;
//             cout << "lower_bound " << lower_bound << ' ' << ballpos_up << endl;
//             if(ballpos_up > lower_bound){
//                 cout << "i col (up dir)!" << endl;
//                 ball_speed[1] = -0.5f*ball_speed[1];
//             }
//             else{
//                 ballpos[1] += add_v_i;
//             }
//         }
//         else{
//             ballpos[1] += add_v_i;
//         }
        
//     }

//     cout << "i " << j_idx << ' ' << ballpos_new_i << ' ' << i_idx_new << endl;

// }


// void checkCollision_j(float add_v_j){
    
//     float ballpos_new_j = ballpos[0]+add_v_j;
//     float ballpos_left = ballpos_new_j-ball_radius;
//     float ballpos_right = ballpos_new_j+ball_radius;
    
//     cout << ballpos_left << ' ' << ballpos_right << endl;
    
    
//     int i_idx = (ballpos[1] - 0.0135f) / (-0.001f);
//     int j_idx_new = (ballpos_new_j + 0.0135f) / (0.001f);
    
//     if(add_v_j < 0){//left
//         j_idx_new = (ballpos_left + 0.0135f) / (0.001f);
        
//         if(maze[j_idx_new][i_idx] == 1 || j_idx_new == 0){
//             float right_bound = -0.0135f + (j_idx_new)*(0.001f) + 0.0005f;
//             cout << "right_bound " << right_bound << ' ' << ballpos_left << endl;
//             if(ballpos_left < right_bound){
//                 cout << "j col (left dir)!" << endl;
//                 ball_speed[0] = -0.5f*ball_speed[0];
//             }
//             else{
//                 ballpos[0] += add_v_j;
//             }
//         }
//         else{
//             ballpos[0] += add_v_j;
//         }
        
//     }
//     else if(add_v_j > 0){//right
//         j_idx_new = (ballpos_right + 0.0135f) / (0.001f);
        
//         if(maze[j_idx_new+1][i_idx] == 1){
//             float left_bound = -0.0135f + (j_idx_new+1)*(0.001f) - 0.0005f;
//             cout << "left_bound " << left_bound << ' ' << ballpos_right << endl;
//             if(ballpos_right > left_bound){
//                 cout << "j col (right dir)!" << endl;
//                 ball_speed[0] = -0.5f*ball_speed[0];
//             }
//             else{
//                 ballpos[0] += add_v_j;
//             }
//         }
//         else{
//             ballpos[0] += add_v_j;
//         }
//     }
    
//     cout << "j "   << ballpos_new_j << ' ' << j_idx_new << ' '<< i_idx << endl;
    
// }


// void ball_movement() {
// 	float framerate = 60.0f;

// 	// manipulate the speed by keyboard
// 	ball_speed[0] += ball_flag[0] * ball_speed_unit;
// 	ball_speed[1] += ball_flag[1] * ball_speed_unit;

// 	checkSpeedWithGravity();
    
//     checkCollision_j(0.5f * ball_speed[0]/ framerate);
//     checkCollision_i(0.5f * ball_speed[1]/ framerate);

// //    ballpos[2] += 0.5f * ball_speed[2] / framerate;


// }

// void display(GLFWwindow* window) {

//     float ratio;
//     int width, height;

//     glfwGetFramebufferSize(window, &width, &height);
//     ratio = width / (float) height;

//     glViewport(0, 0, width, height);
    
//     // clear buffers
//     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
//     //    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
//     gluPerspective( 5, ((GLfloat)width/(GLfloat)height), 0.01, 10 );
//     // move to origin
//     glMatrixMode(GL_MODELVIEW);
//     glLoadIdentity();

//     // move the object backwards
//     glTranslatef( 0.0, 0.0, -1.0 );

//     // move the object in a fancy way

//     // rotate the object
// 	xangle += plane_flag[0] * angle_unit;
// 	yangle += plane_flag[1] * angle_unit;
// 	zangle += plane_flag[2] * angle_unit;
//     glRotatef(xangle, 1.f, 0.f, 0.f);
//     glRotatef(yangle, 0.f, 1.f, 0.f);
//     glRotatef(zangle, 0.f, 0.f, 1.f);
//     drawMaze(maze);

// 	// Sphere(ball) event
//     glColor4f(0,0,1,0.5);
// 	ball_movement();
//     glTranslatef(ballpos[0], ballpos[1], ballpos[2]);
//     drawSphere( ball_radius, 10, 10 );
// }

// void reshape( GLFWwindow* window, int width, int height ) {

//     // set a whole-window viewport
//     glViewport( 0, 0, (GLsizei)width, (GLsizei)height );

//     // create a perspective projection matrix
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
//     gluPerspective( 30, ((GLfloat)width/(GLfloat)height), 0.01, 100 );


// }

// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {

//     if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
//         plane_flag[0] = -1;
//     }
//     if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
//         plane_flag[0] = 1;
//     }
//     if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && action == GLFW_RELEASE) {
//         plane_flag[0] = 0;
//     }
//     if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
//         plane_flag[1] = -1;
//     }
//     if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
//         plane_flag[1] = 1;
//     }
//     if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && action == GLFW_RELEASE) {
//         plane_flag[1] = 0;
//     }
//     if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS) {
//         plane_flag[2] = 1;
//     }
//     if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
//         plane_flag[2] = -1;
//     }
//     if ((key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) && action == GLFW_RELEASE) {
//         plane_flag[2] = 0;
//     }
// 	//ball movement by keyboard
// 	if (key == GLFW_KEY_W && action == GLFW_PRESS) ball_flag[1] = 1;
// 	if (key == GLFW_KEY_S && action == GLFW_PRESS) ball_flag[1] = -1;
// 	if ((key == GLFW_KEY_W || key == GLFW_KEY_S) && action == GLFW_RELEASE) ball_flag[1] = 0;
// 	if (key == GLFW_KEY_A && action == GLFW_PRESS) ball_flag[0] = -1;
// 	if (key == GLFW_KEY_D && action == GLFW_PRESS) ball_flag[0] = 1;
// 	if ((key == GLFW_KEY_A || key == GLFW_KEY_D) && action == GLFW_RELEASE) ball_flag[0] = 0;
// 	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
//         ball_flag[0] = 0;
//         ball_flag[1] = 0;
// 	}
// 	// end of ball movement by keyboard
//     if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
//         xangle = 0.f;
//         yangle = 0.f;
//         zangle = 0.f;
// 		ball_speed[0] = .0f;
// 		ball_speed[1] = .0f;
// 		ball_speed[2] = .0f;
// 		ball_flag[0] = 0;
// 		ball_flag[1] = 0;
// 		ball_flag[2] = 0;
// 		ballpos[0] = -0.0125f;
// 		ballpos[1] = 0.0125f;
// 		ballpos[2] = 0.001f;
//     }
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) exit(0);
// }

// int main(int argc, char* argv[]) {

//     GLFWwindow* window;

//     /* Initialize the library */
//     if (!glfwInit())
//         return -1;

//     // initialize the window system
//     /* Create a windowed mode window and its OpenGL context */
//     window = glfwCreateWindow(640, 480, "Exercise 6 - Open GL", NULL, NULL);
//     if (!window)
//     {
//         glfwTerminate();
//         return -1;
//     }
    
//     // catch keyboard event
//     glfwSetKeyCallback(window, key_callback);


//     // Set callback functions for GLFW
//     glfwSetFramebufferSizeCallback(window, reshape);

//     glfwMakeContextCurrent(window);
//     glfwSwapInterval( 1 );
//     // initialize the GL library
//     init(argc, argv);
    
//     maze = generate_maze();
//     for (int i = 0; i < maze.size(); ++i)
//     {
//         for (int j = 0; j < maze[0].size(); ++j)
//         {
//             if (maze[j][i] == 1) {
//                 cout << '-';
//             }
//             else{
//                 cout << ' ';
//             }
//         }
//         cout<<endl;
//     }
    
//     /* Loop until the user closes the window */
//     while (!glfwWindowShouldClose(window))
//     {
//         /* Render here */
//         display(window);


//         /* Swap front and back buffers */
//         glfwSwapBuffers(window);

//         /* Poll for and process events */
//         glfwPollEvents();
//     }

//     glfwTerminate();

//     return 0;
// }



