// #define GL_SILENCE_DEPRECATION
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


// Game state variables
float planeX = 0.0f, planeY = 0.0f;
float planeSpeed = 0.1f;
bool keys[256] = { false };
float pitch = 0.0f; // up/down
float roll = 0.0f;  // left/right
int gameTime = 0;
bool gameRunning = false;  // Changed to false for start screen
bool gameOver = false;
enum GameState { START_SCREEN, CONTROL_SELECT, PLAYING, GAME_OVER };
enum GameState currentState = START_SCREEN;  // Added game state enum
float obstacleRotation = 0.0f;  // Rotation angle for obstacles


// Mouse control variables
int mouseX = 400, mouseY = 300;    // Store current mouse position
float mouseControlSpeed = 0.01f;   // Mouse sensitivity
bool mouseControl = true;          // Toggle for mouse control
int windowWidth = 800, windowHeight = 600;


typedef struct {
   float x, y, z;
   float radius;
   bool passed;
   float rotation;
} Obstacle;


#define MAX_OBSTACLES 10
Obstacle obstacles[MAX_OBSTACLES];


typedef struct {
   float x, y, z;
   float width, height;
   float r, g, b;
} Building;


#define MAX_BUILDINGS 40
Building buildings[MAX_BUILDINGS];


void initGame() {
   planeX = 0.0f;
   planeY = 0.0f;
   gameTime = 0;
   gameRunning = true;
   gameOver = false;
   currentState = PLAYING;


   for (int i = 0; i < MAX_OBSTACLES; i++) {
       obstacles[i].z = -20.0f - (i * 15.0f);
       obstacles[i].x = ((rand() % 100) / 100.0f) * 5.0f - 2.5f;
       obstacles[i].y = ((rand() % 100) / 100.0f) * 3.0f - 1.5f;
       obstacles[i].rotation = rand() % 360;
       obstacles[i].radius = 1.0f;
       obstacles[i].passed = false;
   }


   for (int i = 0; i < MAX_BUILDINGS; i++) {
       buildings[i].z = -100.0f + (i % 20) * 10.0f;
       buildings[i].width = 2.0f + (rand() % 3);
       buildings[i].height = 3.0f + (rand() % 5);


       if (i < MAX_BUILDINGS / 2) {
           buildings[i].x = -7.0f - (rand() % 5);
       } else {
           buildings[i].x = 7.0f + (rand() % 5);
       }


       buildings[i].r = (rand() % 100) / 100.0f;
       buildings[i].g = (rand() % 100) / 100.0f;
       buildings[i].b = (rand() % 100) / 100.0f;
       buildings[i].y = 0.0f;
   }
  
   // If using mouse control, center the mouse and hide cursor
   if (mouseControl) {
       mouseX = windowWidth / 2;
       mouseY = windowHeight / 2;
       glutWarpPointer(mouseX, mouseY);
       glutSetCursor(GLUT_CURSOR_NONE);
   } else {
       // Show cursor if using keyboard
       glutSetCursor(GLUT_CURSOR_INHERIT);
   }
}


void drawPlane() {
   glPushMatrix();


   // Position plane at correct spot in 3D world
   glTranslatef(planeX, planeY, -5.0f);


   // Apply tilts based on motion (pitch = up/down, roll = side tilt)
   glRotatef(roll, 0, 0, 1);   // Tilt left/right
   glRotatef(pitch, 1, 0, 0);  // Tilt up/down


   // Point the nose of the plane to the right (side view)
   glRotatef(90, 0, 1, 0); 


   // Scale up the plane for better visibility
   glScalef(0.8f, 0.8f, 0.8f);
   glColor3f(1.0f, 1.0f, 1.0f); // Dark gray paper plane


   // --- Plane body parts same as before ---
   // Nose
   glBegin(GL_TRIANGLES);
       glVertex3f(1.0f, 0.0f, 0.0f);
       glVertex3f(-1.0f, 0.2f, 0.5f);
       glVertex3f(-1.0f, 0.2f, -0.5f);
   glEnd();


   // Bottom
   glBegin(GL_TRIANGLES);
       glVertex3f(1.0f, 0.0f, 0.0f);
       glVertex3f(-1.0f, -0.2f, -0.5f);
       glVertex3f(-1.0f, -0.2f, 0.5f);
   glEnd();


   // Wings
   glBegin(GL_QUADS);
       // Left
       glVertex3f(-1.0f, 0.2f, 0.5f);
       glVertex3f(-1.8f, -0.1f, 1.5f);
       glVertex3f(-1.8f, -0.2f, 1.5f);
       glVertex3f(-1.0f, -0.2f, 0.5f);
       // Right
       glVertex3f(-1.0f, 0.2f, -0.5f);
       glVertex3f(-1.8f, -0.1f, -1.5f);
       glVertex3f(-1.8f, -0.2f, -1.5f);
       glVertex3f(-1.0f, -0.2f, -0.5f);
   glEnd();


   // Center crease
   glBegin(GL_TRIANGLES);
       glVertex3f(1.0f, 0.0f, 0.0f);
       glVertex3f(-1.0f, 0.2f, 0.0f);
       glVertex3f(-1.0f, -0.2f, 0.0f);
   glEnd();


   // Tail fin
   glBegin(GL_TRIANGLES);
       glVertex3f(-1.0f, 0.2f, 0.0f);
       glVertex3f(-1.3f, 0.6f, 0.0f);
       glVertex3f(-1.3f, 0.2f, 0.0f);
   glEnd();


   glPopMatrix();
}


void drawStartScreen() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, 800, 0.0, 600, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
  
   // Draw title
   glColor3f(0.8f, 0.0f, 1.0f);
   char titleStr[] = "3D Plane Game";
   glRasterPos2i(300, 460);
   for (char* c = titleStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
   }


   // Draw instructions in neon pink
   glColor3f(1.0f, 0.2f, 0.8f);
   char startStr[] = "Press B to Start";
   glRasterPos2i(310, 410);
   for (char* c = startStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
   }
  
   // Draw plane
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
  
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 200.0f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
  
   // Draw a larger plane for the start screen
   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -10.0f);
   glRotatef(150, 5, -15, 0); // Rotate slightly to show more of the plane
   drawPlane();
   glPopMatrix();
}


void drawControlSelect() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, 800, 0.0, 600, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
  
   // Draw title with a slight glow effect
   glColor3f(0.2f, 0.6f, 1.0f);
   char titleStr[] = "Select Control Method";
   glRasterPos2i(290, 400);
   for (char* c = titleStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
   }


   // Option for mouse control
   glColor3f(0.8f, 0.8f, 1.0f);
   char mouseStr[] = "Press M for Mouse Control";
   glRasterPos2i(300, 320);
   for (char* c = mouseStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
   }
  
   // Draw mouse icon
   glColor3f(0.7f, 0.7f, 0.9f);
   glBegin(GL_QUADS);
       glVertex2f(270, 315);
       glVertex2f(270, 335);
       glVertex2f(290, 335);
       glVertex2f(290, 315);
   glEnd();
  
   // Option for keyboard control
   glColor3f(0.8f, 0.8f, 1.0f);
   char keyStr[] = "Press K for Keyboard Control";
   glRasterPos2i(300, 270);
   for (char* c = keyStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
   }
  
   // Draw keyboard icon
   glColor3f(0.7f, 0.7f, 0.9f);
   glBegin(GL_LINE_LOOP);
       glVertex2f(270, 265);
       glVertex2f(270, 285);
       glVertex2f(290, 285);
       glVertex2f(290, 265);
   glEnd();
  
   // Add "W", "A", "S", "D" keys
   glRasterPos2i(277, 277);
   glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'W');
  
   glRasterPos2i(272, 267);
   glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'A');
  
   glRasterPos2i(277, 267);
   glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'S');
  
   glRasterPos2i(283, 267);
   glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'D');


   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}


void drawObstacles() {
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);


   GLfloat lightPos[] = {0.0f, 10.0f, 10.0f, 1.0f};
   glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


   glEnable(GL_COLOR_MATERIAL);  // Allow color to affect material
   glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);


   for (int i = 0; i < MAX_OBSTACLES; i++) {
       glPushMatrix();


       // Move to position
       glTranslatef(obstacles[i].x, obstacles[i].y, obstacles[i].z);


       // Rotate around Z-axis
       glRotatef(obstacles[i].rotation, 0.0f, 0.0f, 1.0f);


       // Add a colorful pattern for better rotation visibility
       for (int j = 0; j < 12; ++j) {
           glPushMatrix();
           glRotatef(j * 30.0f, 0.0f, 0.0f, 1.0f); // Rotate each "spoke"
           glColor3f(j % 2, (j + 1) % 2, 0.0f);    // Alternate colors
           glutSolidTorus(0.15f, obstacles[i].radius, 30, 30);
           glPopMatrix();
       }


       glPopMatrix();


       // Move obstacle forward
       obstacles[i].z += 0.2f;


       // Speed up rotation slightly
       obstacles[i].rotation += 5.0f;
       if (obstacles[i].rotation > 360.0f) obstacles[i].rotation -= 360.0f;


       // Score and reset logic
       if (!obstacles[i].passed && obstacles[i].z > 0) {
           gameTime++;
           obstacles[i].passed = true;
       }


       if (obstacles[i].z > 10.0f) {
           obstacles[i].z = -100.0f;
           obstacles[i].x = (rand() % 5) - 2.5f;
           obstacles[i].y = (rand() % 3) - 1.0f;
           obstacles[i].rotation = rand() % 360;
           obstacles[i].passed = false;
       }
   }


   glDisable(GL_COLOR_MATERIAL);
   glDisable(GL_LIGHTING);
}


void drawBuildings() {
   float groundLevel = -2.0f;


   for (int i = 0; i < MAX_BUILDINGS; i++) {
       float x = buildings[i].x;
       // float y = buildings[i].y;
       float z = buildings[i].z;
       float r = buildings[i].r;
       float g = buildings[i].g;
       float b = buildings[i].b;
       float width = buildings[i].width;
       float height = buildings[i].height;


       float y = groundLevel + (height / 2.0f);
      
       glPushMatrix();
       glTranslatef(x, y, z);
      
       // Draw the main building structure
       glEnable(GL_COLOR_MATERIAL);
       glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
       glColor3f(r, g, b);
      
       // Draw the main building as a solid block
       glPushMatrix();
       glScalef(width, height, width); // Make it cubic but keep original height
       glutSolidCube(1.0f);
       glPopMatrix();
      
       // Add windows (simple dark rectangles)
       glColor3f(0.1f, 0.1f, 0.3f); // Dark blue/black windows
      
       float windowSize = 0.1f;
       float windowSpacing = 0.25f;
       int floorsToShow = (int)(height * 2.0f); // Fewer floors to keep buildings manageable
      
       // Front face windows
       for (int floor = 0; floor < floorsToShow; floor++) {
           for (int window = 0; window < 2; window++) { // Just 2 windows per floor
               glPushMatrix();
               float winX = -width/4.0f + window * windowSpacing * 2;
               float winY = -height/2.0f + floor * windowSpacing + windowSpacing/2.0f;
               glTranslatef(winX, winY, width/2.0f + 0.01f);
               glScalef(windowSize, windowSize, 0.01f);
               glutSolidCube(1.0f);
               glPopMatrix();
           }
       }
      
       // Side face windows (only on one side to save performance)
       for (int floor = 0; floor < floorsToShow; floor++) {
           for (int window = 0; window < 2; window++) {
               glPushMatrix();
               float winZ = -width/4.0f + window * windowSpacing * 2;
               float winY = -height/2.0f + floor * windowSpacing + windowSpacing/2.0f;
               glTranslatef(width/2.0f + 0.01f, winY, winZ);
               glScalef(0.01f, windowSize, windowSize);
               glutSolidCube(1.0f);
               glPopMatrix();
           }
       }
      
       glDisable(GL_COLOR_MATERIAL);
       glPopMatrix();
      
       // Keep your original movement logic intact
       buildings[i].z += 0.2f;
       if (buildings[i].z > 20.0f) {
           buildings[i].z = -100.0f;
           buildings[i].x = (buildings[i].x < 0) ? -7.0f - (rand() % 5) : 7.0f + (rand() % 5);
           buildings[i].height = 1.0f + (rand() % 4); // Preserve your original height range
       }
   }
}


void drawRunway() {
   float groundLevel = -2.0f; // Same as in drawBuildings()
   float runwayWidth = 5.0f;
   float runwayLength = 200.0f;
   float runwayThickness = 0.2f;
  
   // Draw the main runway (dark gray surface)
   glPushMatrix();
   glTranslatef(0.0f, groundLevel - runwayThickness/2, 0.0f);
   glScalef(runwayWidth, runwayThickness, runwayLength);
   glColor3f(0.2f, 0.2f, 0.2f); // Dark gray
   glutSolidCube(1.0f);
   glPopMatrix();
  
   // Draw runway markings (white stripes)
   glColor3f(1.0f, 1.0f, 1.0f); // White
   float stripeWidth = 0.3f;
   float stripeLength = 3.0f;
   float stripeSpacing = 5.0f;
  
   for (float z = -runwayLength/2; z < runwayLength/2; z += stripeSpacing) {
       // Center stripe
       glPushMatrix();
       glTranslatef(0.0f, groundLevel - runwayThickness/2 + 0.01f, z);
       glScalef(stripeWidth, 0.01f, stripeLength);
       glutSolidCube(1.0f);
       glPopMatrix();
   }
  
   // Draw green "grass" on both sides of the runway
   glColor3f(0.0f, 0.4f, 0.0f); // Green
   float grassWidth = 20.0f;
  
   // Left grass
   glPushMatrix();
   glTranslatef(-runwayWidth/2 - grassWidth/2, groundLevel - runwayThickness/2, 0.0f);
   glScalef(grassWidth, runwayThickness*0.9f, runwayLength);
   glutSolidCube(1.0f);
   glPopMatrix();
  
   // Right grass
   glPushMatrix();
   glTranslatef(runwayWidth/2 + grassWidth/2, groundLevel - runwayThickness/2, 0.0f);
   glScalef(grassWidth, runwayThickness*0.9f, runwayLength);
   glutSolidCube(1.0f);
   glPopMatrix();
}


void drawHUD() {
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, 800, 0.0, 600, -1.0, 1.0);


   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();


   glColor3f(1.0f, 1.0f, 1.0f);
   char timeStr[50];
   sprintf(timeStr, "TIME : %d", gameTime);
   glRasterPos2i(650, 550);
   for (char* c = timeStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
   }


   // Show different control tips based on control mode
   if (mouseControl) {
       char controlStr[] = "MOVE: Mouse, RESTART: [R], MAIN MENU: [M], TOGGLE CONTROLS: [T]";
       glRasterPos2i(10, 550);
       for (char* c = controlStr; *c != '\0'; c++) {
           glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
       }
   } else {
       char controlStr[] = "UP: [W], DOWN: [S], LEFT: [A], RIGHT: [D], TOGGLE CONTROLS: [T]";
       glRasterPos2i(10, 550);
       for (char* c = controlStr; *c != '\0'; c++) {
           glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
       }
   }


   // Display control method
   char modeStr[30];
   sprintf(modeStr, "Control: %s", mouseControl ? "Mouse" : "Keyboard");
   glRasterPos2i(650, 530);
   for (char* c = modeStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
   }


   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}


bool checkCollision() {
   for (int i = 0; i < MAX_OBSTACLES; i++) {
       if (obstacles[i].z > -7.0f && obstacles[i].z < -3.0f) {
           float dx = planeX - obstacles[i].x;
           float dy = planeY - obstacles[i].y;
           float distance = sqrt(dx * dx + dy * dy);


           if (distance > obstacles[i].radius - 0.3f &&
               distance < obstacles[i].radius + 0.3f) {
               continue;
           } else if (distance < obstacles[i].radius + 0.5f) {
               return true;
           }
       }
   }
   return false;
}


void keyPressed(unsigned char key, int x, int y) {
   keys[key] = true;
  
   // Handle start screen
   if ((key == 'b' || key == 'B') && currentState == START_SCREEN) {
       currentState = CONTROL_SELECT; // Show control select screen instead of directly starting game
   }
  
   // Handle control selection
   if (currentState == CONTROL_SELECT) {
       if (key == 'm' || key == 'M') {
           mouseControl = true;
           initGame();
       }
       else if (key == 'k' || key == 'K') {
           mouseControl = false;
           initGame();
       }
   }
  
   // Handle game over screen
   if ((key == 'r' || key == 'R') && currentState == GAME_OVER) {
       initGame();
   }
  
   // Return to main menu
   if (key == 'g' || key == 'G' && currentState != CONTROL_SELECT) {
       currentState = START_SCREEN;
       gameRunning = false;
   }
  
   // Toggle mouse/keyboard control during gameplay
   if ((key == 't' || key == 'T') && currentState == PLAYING) {
       mouseControl = !mouseControl;
      
       // If turning mouse control on, center the mouse and hide cursor
       if (mouseControl) {
           mouseX = windowWidth / 2;
           mouseY = windowHeight / 2;
           glutWarpPointer(mouseX, mouseY);
           glutSetCursor(GLUT_CURSOR_NONE);
       } else {
           // Show cursor for keyboard control
           glutSetCursor(GLUT_CURSOR_INHERIT);
       }
   }
}


void keyReleased(unsigned char key, int x, int y) {
   keys[key] = false;
}

void mouseMotion(int x, int y) {
   if (mouseControl && currentState == PLAYING) {
       // Calculate delta from center of screen
       int deltaX = x - mouseX;
       int deltaY = y - mouseY;
      
       // Update plane position based on mouse movement
       planeX += deltaX * mouseControlSpeed;
       planeY -= deltaY * mouseControlSpeed;  // Reversed because y-coordinate is inverted
      
       // Recenter the mouse cursor to avoid hitting screen boundaries
       if (x != windowWidth/2 || y != windowHeight/2) {
           mouseX = windowWidth/2;
           mouseY = windowHeight/2;
           glutWarpPointer(mouseX, mouseY);
       }
      
       // Update roll and pitch based on mouse movement for visual effect
       roll = -deltaX * 0.25f;
       pitch = deltaY * 0.25f;
   }
}


void updatePlanePosition() {
   // Keyboard movement (still active if not using mouse)
   if (!mouseControl) {
       if (keys['w'] || keys['W']) planeY += planeSpeed;
       if (keys['s'] || keys['S']) planeY -= planeSpeed;
       if (keys['a'] || keys['A']) planeX -= planeSpeed;
       if (keys['d'] || keys['D']) planeX += planeSpeed;
      
       // Update visual tilting for keyboard mode
       if (keys['w'] || keys['W']) {
           pitch = 20.0f;    // Tilt nose up
       } else if (keys['s'] || keys['S']) {
           pitch = -20.0f;   // Tilt nose down
       } else {
           pitch = 0.0f;     // Reset pitch
       }


       if (keys['a'] || keys['A']) {
           roll = 20.0f;     // Tilt left
       } else if (keys['d'] || keys['D']) {
           roll = -20.0f;    // Tilt right
       } else {
           roll = 0.0f;      // Reset roll
       }
   }


   // Calculate visible frustum bounds at z = -5.0f
   float z = 5.0f; // distance from camera
   float fov = 45.0f;
   float aspect = 800.0f / 600.0f;


   float top = tan(fov * 0.5f * M_PI / 180.0f) * z;
   float right = top * aspect;


   // Clamp plane position within visible bounds (0.5f margin for plane size)
   if (planeX < -right + 0.5f) planeX = -right + 0.5f;
   if (planeX >  right - 0.5f) planeX =  right - 0.5f;
   if (planeY < -top + 0.5f)   planeY = -top + 0.5f;
   if (planeY >  top - 0.5f)   planeY =  top - 0.5f;
}


void drawGameOver() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0.0, 800, 0.0, 600, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();


   glColor3f(1.0f, 0.0f, 0.0f);
   char gameOverStr[] = "GAME OVER - Press R to restart";
   glRasterPos2i(300, 300);
   for (char* c = gameOverStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
   }


   char scoreStr[50];
   sprintf(scoreStr, "Final Score: %d", gameTime);
   glRasterPos2i(320, 270);
   for (char* c = scoreStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
   }


   char menuStr[] = "Press G for Main Menu";
   glRasterPos2i(320, 240);
   for (char* c = menuStr; *c != '\0'; c++) {
       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
   }


   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}


void display() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();


   switch (currentState) {
       case START_SCREEN:
           drawStartScreen();
           break;
          
       case CONTROL_SELECT:
           drawControlSelect();
           break;
          
       case PLAYING:
           updatePlanePosition();
           if (checkCollision()) {
               currentState = GAME_OVER;
               gameRunning = false;
               gameOver = true;
              
               // Show cursor on game over
               glutSetCursor(GLUT_CURSOR_INHERIT);
           }


           // Set up camera to follow plane vertically
           glMatrixMode(GL_MODELVIEW);
           glLoadIdentity();
           gluLookAt(0.0f, planeY, 5.0f,   // Camera position (moves up/down with plane)
                   0.0f, planeY, -5.0f,  // Look at plane (keep focus forward)
                   0.0f, 1.0f, 0.0f);    // Up vector


           drawPlane();
           drawObstacles();
           drawRunway();
           drawBuildings();
           drawHUD();
           break;
          
       case GAME_OVER:
           drawGameOver();
           break;
   }


   glutSwapBuffers();
}


void update(int value) {
   glutPostRedisplay();
   glutTimerFunc(16, update, 0);  // ~60 FPS
}


void reshape(int w, int h) {
   windowWidth = w;
   windowHeight = h;
  
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(800, 600);
   glutCreateWindow("Flight Through Rings - OpenGL Game");


   glutDisplayFunc(display);
   glutReshapeFunc(reshape);  // Add reshape callback
   glutPassiveMotionFunc(mouseMotion);  // Add mouse motion callback
   glutTimerFunc(25, update, 0);
   glutKeyboardFunc(keyPressed);
   glutKeyboardUpFunc(keyReleased);


   glEnable(GL_DEPTH_TEST);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


   // Lighting setup
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);


   // GLfloat light_pos[] = { 0.0f, 5.0f, 10.0f, 1.0f};
   GLfloat ambient[]  = { 1.0f, 0.0f, 0.0f};
   GLfloat diffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };


   // glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glEnable(GL_NORMALIZE);


   // Projection
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 200.0f);
   glMatrixMode(GL_MODELVIEW);


   // Initialize mouse position
   mouseX = windowWidth / 2;
   mouseY = windowHeight / 2;


   // For smoother mouse motion, hide cursor
   glutSetCursor(GLUT_CURSOR_NONE);


   glutMainLoop();
   return 0;
}