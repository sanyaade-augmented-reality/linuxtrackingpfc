#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TrackingPFC_client.h>

   GLfloat obsx;
   GLfloat obsy;
   GLfloat obsz;

   GLint framen;
   GLchar mensaje[100];

   TrackingPFC_client* track;

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
   GLfloat mld2scr= 16/(track->getDisplaySizex());
   obsx = track->getlastposx()*mld2scr;
   obsy = track->getlastposy()*mld2scr;
   obsz = track->getlastposz()*mld2scr;

   GLfloat znear =1.0;
   GLfloat zfar =100.0;
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
   sprintf(buffer, "X %f", track->getlastposx());   
   output(-7,-3.5,buffer ); 
   sprintf(buffer, "Y %f", track->getlastposy());   
   output(-7,-4.0,buffer ); 
   sprintf(buffer, "Z %f", track->getlastposz());   
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
}
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
	 glutLeaveGameMode(); //set the resolution how it was
	 delete(track);
         exit(0);
         break;
      default:
	//printf("Key %i not supported\n", key);
	break;
   }
}

int main(int argc, char** argv)
{

   framen=0;
   sprintf(mensaje,"Press esc to exit\n");

   track = new TrackingPFC_client();

   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (960, 600); 
   glutInitWindowPosition (0,0);
   glutCreateWindow (argv[0]);
   //glutGameModeString( "1920x1200:32@60" ); //the settings for fullscreen mode
   /*glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
   glutEnterGameMode(); //set glut to fullscreen using the settings in the line above*/
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(glutPostRedisplay);
   glutMainLoop();
   return 0;
}
