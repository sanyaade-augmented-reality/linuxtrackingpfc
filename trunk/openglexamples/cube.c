#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

   GLfloat obsx;
   GLfloat obsy;
   GLfloat obsz;

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
   glutWireCube (4.0);

   glColor3f(1.0, 1.0, 1.0);
   char buffer[160];
   sprintf(buffer, "X %f", obsx);   
   output(-7,-3.5,buffer ); 
   sprintf(buffer, "Y %f", obsy);   
   output(-7,-4.0,buffer ); 
   sprintf(buffer, "Z %f", obsz);   
   output(-7,-4.5,buffer ); 
   //glFlush ();
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
      case 119: // w
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
	 break;
      default:
	printf("Key %i not supported\n", key);
	break;
   }
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   /*glutInitWindowSize (960, 600); 
   glutInitWindowPosition (0,0);
   glutCreateWindow (argv[0]);*/
   //glutGameModeString( "1920x1200:32@60" ); //the settings for fullscreen mode
   glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
   glutEnterGameMode(); //set glut to fullscreen using the settings in the line above
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMainLoop();
   return 0;
}
