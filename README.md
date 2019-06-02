# AR_project_2019

## Maze_AR
drawMaze() is defined DrawPrimitive.h;

Mode1: If you want to rotate an object by keyboard input, commentout upper part of main.cpp and remove comment out bottom part of main.cpp.  
Mode2: If you track QR code, commentout bottom part of main.cpp and remove comment out upper part of main.cpp.

### Mode1
xangle, yangle, zangle indicate the rotation angles of x axis, y axis, z axis respectively.  
key_callback is called when you press keyboard.  
angle_unit is angle of rotation when the keyboard is pressed.

Press the up and down keys to rotate around y axis.  
Press the left and right keys to rotate around x axis.  
Press the left and right shift keys to rotate in the horizontal plane (z axis).  
Press the entry to return to the initial posture.  
The program ends when the escape key is pressed

For ball_movement

test by keyboard with
W, S: x.pos +- speed_unit
A, D: y.pos +- speed_unit
I, K: x.pos + speed_unit_m * framerate
J, L: y.pos + speed_unit_m * framerate
speed_unit = 0.001
framerate = 60

### Mode2
We made fix that Mori-kun said in Slack.  
Please change each time there is a problem.
