/*******************************************************************
	       Multi-Part Model Construction and Manipulation
		   Rahul Bilimoria
		   500569144
********************************************************************/

#define _USE_MATH_DEFINES
#define DTR (M_PI/180)

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

void initOpenGL(int w, int h);
void display(void);
void drawSubmarine();
void drawBase();
void drawSail();
void drawSailPlanes();
void drawRudder();
void drawPropeller();
void rotatePropellers(int n);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
VECTOR3D ScreenToWorld(int x, int y);

static int currentButton;
static unsigned char currentKey;
float rotated = 0.0;
float translatedX = 0.0;
float newX = 0.0;
float moveX = 0.0;
float translatedY = 0.0;
float translatedZ = 0.0;
float newZ = 0.0;
float moveZ = 0.0;
float angle = 0.0;
int speed = 1;
boolean move = false;
int delay = 10;

GLfloat light_position0[] = {-6.0,  12.0, 0.0,1.0};
GLfloat light_position1[] = { 6.0,  12.0, 0.0,1.0};
GLfloat light_diffuse[]   = {1.0, 1.0, 1.0, 1.0};
GLfloat light_specular[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat light_ambient[]   = {0.2, 0.2, 0.2, 1.0};


// Set up lighting/shading and material properties for submarine - upcoming lecture - just copy for now
GLfloat submarine_mat_ambient[] = { 0.25, 0.25, 0.25, 1.0 };
GLfloat submarine_mat_specular[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat submarine_mat_diffuse[] = { 0.774597, 0.774597, 0.774597, 1.0 };
GLfloat submarine_mat_shininess[] = { 76.8 };

GLfloat propeller_mat_ambient[] = { 0.329412, 0.223529, 0.027451, 1.0 };
GLfloat propeller_mat_specular[] = { 0.780392, 0.568627, 0.113725, 1.0 };
GLfloat propeller_mat_diffuse[] = { 992157, 0.941176, 0.807843, 1.0 };
GLfloat propeller_mat_shininess[] = { 27.8974 };

QuadMesh *groundMesh = NULL;


struct BoundingBox
{
  VECTOR3D min;
  VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

int main(int argc, char **argv)
{
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Scene Modeller");

  initOpenGL(500,500);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotionHandler);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(functionKeys);
  glutTimerFunc(delay, rotatePropellers, 0);

  glutMainLoop();
  return 0;
}



// Setup openGL */
void initOpenGL(int w, int h)
{
  // Set up viewport, projection, then change to modelview matrix mode - 
  // display function will then set up camera and modeling transforms
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0,1.0,0.2,80.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set up and enable lighting
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  
  glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_LIGHT1);
  
  // Other OpenGL setup
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.6, 0.6, 0.6, 0.0);  
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  // This one is important - renormalize normal vectors 
  glEnable(GL_NORMALIZE);
  
  //Nice perspective.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT , GL_NICEST);
  
  // Set up ground quad mesh
  VECTOR3D origin  = VECTOR3D(-8.0f,0.0f,8.0f);
  VECTOR3D dir1v   = VECTOR3D(1.0f, 0.0f, 0.0f);
  VECTOR3D dir2v   = VECTOR3D(0.0f, 0.0f,-1.0f);
  groundMesh = new QuadMesh(meshSize, 16.0);
  groundMesh->InitMesh(meshSize, origin, 16.0, 16.0, dir1v, dir2v);
  
  VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
  VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
  VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
  float shininess = 0.2;
  groundMesh->SetMaterial(ambient,diffuse,specular,shininess);

  // Set up the bounding box of the scene
  // Currently unused. You could set up bounding boxes for your objects eventually.
  BBox.min.Set(-8.0f, 0.0, -8.0);
  BBox.max.Set( 8.0f, 6.0,  8.0);
}



void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glLoadIdentity();
  
  // Set up the camera
  gluLookAt(0.0,15.0,22.0,0.0,0.0,0.0,0.0,1.0,0.0);
  
  // Draw Submarine

  // Draw ground
  groundMesh->DrawMesh(meshSize);

  // Set submarine material properties
  glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

  // Apply transformations to move submarine
  glTranslatef(translatedX, translatedY, translatedZ);
  glRotatef(rotated, 0.0, 1.0, 0.0);

  // ...
  // Apply transformations to construct submarine

  drawSubmarine();

  glutSwapBuffers();
}

void drawSubmarine() {
	drawBase();
	drawSail();
	drawSailPlanes();
	drawRudder();
	drawPropeller();
}

void drawBase() {

	glPushMatrix();

	glTranslatef(0.0, 3.0, 0.0);
	glRotatef(45.0, 0.0, 0.0, 0.0);
	glScalef(5.0, 1.0, 1.0);
	glutSolidSphere(1.5, 20.0, 20.0);

	glPopMatrix();
}

void drawSail() {

	glPushMatrix();
	glTranslatef(-0.6, 3.8, 0.0);
	glScalef(2.0, 2.0, 0.2);
	glutSolidCube(1.0);
	glPopMatrix();

}

void drawSailPlanes() {

	glPushMatrix();
	glTranslatef(-0.5, 4.4, 0.3);
	glRotatef(-15.0, 1.0, 0.0, 0.0);
	glScalef(1.5, 0.1, 0.6);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.5, 4.4, -0.3);
	glRotatef(15.0, 1.0, 0.0, 0.0);
	glScalef(1.5, 0.1, 0.6);
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawRudder() {

	glPushMatrix();
	glTranslatef(5.0, 3.1, 0.0);
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glutSolidCone(0.75, 2.0, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.6, 3.8, 0.0);
	glRotatef(15.0, 0.0, 0.0, 1.0);
	glScalef(1.5, 1.5, 0.1);
	glutSolidCube(0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.6, 2.5, 0.0);
	glRotatef(-15.0, 0.0, 0.0, 1.0);
	glScalef(1.5, 1.5, 0.1);
	glutSolidCube(0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.6, 3.0, 0.8);
	glRotatef(-15, 0.0, 1.0, 0.0);
	glRotatef(15, 1.0, 0.0, 0.0);
	glScalef(1.4,0.1,1.4);
	glutSolidCube(0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.6, 3.0, -0.8);
	glRotatef(15, 0.0, 1.0, 0.0);
	glRotatef(-15, 1.0, 0.0, 0.0);
	glScalef(1.4, 0.1, 1.4);
	glutSolidCube(0.5);
	glPopMatrix();
}

void drawPropeller() {

	glMaterialfv(GL_FRONT, GL_AMBIENT, propeller_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, propeller_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, propeller_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, propeller_mat_shininess);

	glPushMatrix();
	glTranslatef(5.3, 3.08, 0.0);
	glRotatef(angle, 1.0, 0.0, 0.0);
	glScalef(0.1, 1.0, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.3, 3.08, 0.0);
	glRotatef(120.0 + angle, 1.0, 0.0, 0.0);
	glScalef(0.1, 1.0, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.3, 3.08, 0.0);
	glRotatef(-120.0 + angle, 1.0, 0.0, 0.0);
	glScalef(0.1, 1.0, 0.1);
	glutSolidCube(1);
	glPopMatrix();
}

void rotatePropellers(int n) {
	if (move) {
		translatedX -= (cos(rotated * DTR)) / (20/speed);
		if (translatedX > 10)
			translatedX = 10;
		else if (translatedX < -10)
			translatedX = -10;
		translatedZ += (sin(rotated * DTR)) / (20/speed);
		if (translatedZ > 10)
			translatedZ = 10;
		else if (translatedZ < -10) 
			translatedZ = -10;
		angle += 1 + (speed * 3);
	}
	glutPostRedisplay();
	glutTimerFunc(delay, rotatePropellers, 0);
}

// Called at initialization and whenever user resizes the window */
void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0,1.0,0.2,40.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

}

VECTOR3D pos = VECTOR3D(0,0,0);

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
  currentButton = button;

  switch(button)
  {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN)
	{
	  ;
	  
	}
	break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN)
	{
	  ; 
	}
	break;
  default:
	break;
  }
  glutPostRedisplay();
}

// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
  if (currentButton == GLUT_LEFT_BUTTON)
  {
	  ;
  }
  glutPostRedisplay();
}


VECTOR3D ScreenToWorld(int x, int y)
{
	// you will need this if you use the mouse
	return VECTOR3D(0);
}// ScreenToWorld()

/* Handles input from the keyboard, non-arrow keys */
void keyboard(unsigned char key, int x, int y)
{
  switch (key) 
  {
  case 't':

	  break;
  case 'a':
	  rotated += 5.0;
	  break;
  case 'd':
	  rotated -= 5.0;
	  break;
  case 'q':
	  speed -= 1;
	  if (speed < 1)
		  speed = 1;
	  break;
  case 'e':
	  speed += 1;
	  if (speed > 3)
		  speed = 3;
	  break;
  case 'w':
	  translatedY += 0.1;
	  if (translatedY > 5) {
		  translatedY = 5;
	  }
	  break;
  case 's':
	  translatedY -= 0.1;
	  if (translatedY < -2) {
		  translatedY = -2;
	  }
	  break;
  case ' ':
	  move = !move;
	  break;
  }
  glutPostRedisplay();
}

void functionKeys(int key, int x, int y)
{
  VECTOR3D min, max;

  if (key == GLUT_KEY_F1)
  {
	  std::cout << "HELP FUNCTIONS" << std::endl;
	  std::cout << "Q: lowers the speed of the submarine" << std::endl;
	  std::cout << "E: raises the speed of the submarine" << std::endl;
	  std::cout << "W: raises the submarine" << std::endl;
	  std::cout << "S: lowers the submarine" << std::endl;
	  std::cout << "A: turns the submarine counter-clockwise" << std::endl;
	  std::cout << "D: turns the submarine clockwise" << std::endl;
	  std::cout << "Spacebar: starts and stops moving the submarine forward" << std::endl;
    
  }
  glutPostRedisplay();
}