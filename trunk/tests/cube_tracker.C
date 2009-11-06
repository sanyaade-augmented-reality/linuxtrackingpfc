#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vrpn_Tracker.h>

// Variables globales
  // posiciones del observador
   GLfloat obsx;
   GLfloat obsy;
   GLfloat obsz;

  // Frame number y mensaje a mostrar
   GLint framen;
   GLchar mensaje[100];


// Parte de OpenGL

void init(void) 
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);
   obsx = 0.0;
   obsy = 0.0;
   obsz = 16.0;
}

void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void output(float x, float y, char *string){
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}
   
void display(void)
{

   GLfloat znear =1.0;
   GLfloat zfar =100.0;
   /*obsx = -8.0;
   obsy = 5.0;
   obsz = 16.0;*/
   GLfloat frleft=znear*(-8.0-obsx)/obsz;
   GLfloat frright=znear*(8.0-obsx)/obsz;
   GLfloat frup=znear*(-5.0-obsy)/obsz;
   GLfloat frdown=znear*(5.0-obsy)/obsz;

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glFrustum (frleft, frright, frup, frdown, znear, zfar);
   glMatrixMode (GL_MODELVIEW);

   glClear (GL_COLOR_BUFFER_BIT);
   glLoadIdentity ();             /* clear the matrix */

           /* viewing transformation  */
   gluLookAt (obsx, obsy, obsz, obsx, obsy, obsz-1.0, 0.0, 1.0, 0.0);

   //glScalef (1.0, 2.0, 1.0);      /* modeling transformation */ 

  glLineWidth(1.0);
  float i;
  for (i=-50.0;i<=0.0;i+=2.0){
   glBegin(GL_LINE_LOOP);
    if (i==0.0) glColor3f(0.0, 1.0, 0.0);
    else if (i==-2.0) glColor3f(1.0, 0.7, 0.7); 
    else glColor3f(0.7, 0.7, 1.0);
    glVertex3f(-8.0, -5.0, i);
    glVertex3f(8.0, -5.0, i);
    glVertex3f(8.0, 5.0, i);
    glVertex3f(-8.0, 5.0, i);
   glEnd();
  }
   glBegin(GL_LINES);
    glColor3f(0.7, 0.7, 1.0);
    glVertex3f(-8.0, -5.0, -30.0);
    glVertex3f(-8.0, -5.0, 30.0);
    glVertex3f(8.0, -5.0, -30.0);
    glVertex3f(8.0, -5.0, 30.0);
    glVertex3f(-8.0, 5.0, -30.0);
    glVertex3f(-8.0, 5.0, 30.0);
    glVertex3f(8.0, 5.0, -30.0);
    glVertex3f(8.0, 5.0, 30.0);
   glEnd();

   glColor3f(1.0, 0.0, 0.0);
   glLineWidth(3.0);
   glutWireCube (4.0);

   glColor3f(1.0, 1.0, 1.0);
   char buffer[160];
   sprintf(buffer, "X %f", obsx);   
   output(-7,-3.5,buffer ); 
   sprintf(buffer, "Y %f", obsy);   
   output(-7,-4.0,buffer ); 
   sprintf(buffer, "Z %f", obsz);   
   output(-7,-4.5,buffer ); 


   sprintf(buffer, "Frame %i", framen);   
   output(5.5,-4.5,buffer );
   framen++;
    
   output(-7.0, 4.5, mensaje );

   glutSwapBuffers(); //swap the buffers
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   /*glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glFrustum (-8.0, 8.0, -4.5, 4.5, 1.5, 20.0);
   glMatrixMode (GL_MODELVIEW);*/
}
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
	 glutLeaveGameMode(); //set the resolution how it was
         exit(0);
         break;
      /*case 119: // w
	 obsz-=0.1;
	 glutPostRedisplay();
	 break;
      case 115: // s
	 obsz+=0.1;
	 glutPostRedisplay();
	 break;
      case 97: // a
	 obsx-=0.1;
	 glutPostRedisplay();
	 break;
      case 100: // d
	 obsx+=0.1;
	 glutPostRedisplay();
	 break;
      case 120: // x
	 obsy-=0.1;
	 glutPostRedisplay();
	 break;
      case 32: // space
	 obsy+=0.1;
	 glutPostRedisplay();
	 break;*/
      default:
	printf("Key %i not supported\n", key);
	break;
   }
}

// Parte de vrpn
vrpn_Tracker_Remote *tkr;

void checktracker(){
  tkr->mainloop();
}

void    VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
  /*printf("handle_tracker\tSensor %d is now at (%g,%g,%g)\n", 
	 t.sensor,
	 t.pos[0], t.pos[1], t.pos[2]);*/
   obsx = t.pos[0];
   obsy = t.pos[1];
   obsz = t.pos[2]+16.0;
  glutPostRedisplay();
}
// Main y funciones auxiliares
int main(int argc, char** argv)
{
   
   tkr = new vrpn_Tracker_Remote("Tracker0@localhost");
   tkr->register_change_handler(NULL, handle_tracker,0);

   framen=0;
   //sprintf(mensaje,"Keys: w a s d x space esc\n");
   sprintf(mensaje,"Perspective adjustement via Tracker0@localhost (esc to exit)");
   
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (960, 600); 
   glutInitWindowPosition (0,0);
   glutCreateWindow (argv[0]);
   //glutGameModeString( "1920x1200:32@60" ); //the settings for fullscreen mode
   //glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
   //glutEnterGameMode(); //set glut to fullscreen using the settings in the line above
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   //glutIdleFunc(glutPostRedisplay);
   glutIdleFunc(checktracker);
   glutMainLoop();
   return 0;
}
