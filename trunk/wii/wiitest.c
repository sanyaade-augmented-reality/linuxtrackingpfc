#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwiid.h>
#include <GL/glut.h>
#include <string.h>

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);
cwiid_mesg_callback_t cwiid_callback;
cwiid_err_t err;
cwiid_wiimote_t *wiimote;	/* wiimote handle */
int led_pos;				


// *******************************************************************
   // posiciones del observador
   GLfloat obsx;
   GLfloat obsy;
   GLfloat obsz;
   GLint framen;
   GLchar mensaje[100];

void init(void){
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);
   //glViewport (0, 0, (GLsizei) w, (GLsizei) h); // al estar usando fullscreen esto ya esta definido?
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
  
void display(void){

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
void detectwiimote();
void keyboard(unsigned char key, int x, int y){
   switch (key) {
      case 27:
	 glutLeaveGameMode(); //set the resolution how it was
	 if (wiimote && cwiid_close(wiimote)) {
		fprintf(stderr, "Error on wiimote disconnect\n");
		exit(-1);
	 }
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
      case 99: // c
	detectwiimote();
	break;
      default:
	printf("Key %i not supported\n", key);
	break;
   }
}

// ************************************************

void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap){
	// comento para que no salga nada de errores
}

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state){
	if (cwiid_set_led(wiimote, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
	}
}
	
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode){
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}

void updateleds(int x){
  int pos;
  if (x==-1){
    pos=0;
  }else if (x<256){
    pos=1;
  }else if (x<512){
    pos=2;
  }else if (x<768){
    pos=3;
  }else {
    pos=4;
  }
  
  if (pos!=led_pos){
    int led_state=0;
    led_pos=pos;
    switch (pos){
      case 1:
	toggle_bit(led_state, CWIID_LED1_ON);
        set_led_state(wiimote, led_state);
	break;
      case 2:
	toggle_bit(led_state, CWIID_LED2_ON);
        set_led_state(wiimote, led_state);
	break;
      case 3:
	toggle_bit(led_state, CWIID_LED3_ON);
        set_led_state(wiimote, led_state);
	break;
      case 4:
	toggle_bit(led_state, CWIID_LED4_ON);
        set_led_state(wiimote, led_state);
	break;
      default: //case 0
	set_led_state(wiimote, led_state);
	break;
    }
  }
}

void updateCube(int x, int y){
  obsx=8.0 - (x/64.0);// 64=1024/16
  obsy=-5.0 + (y/48.0);// 48=768/16;

}

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp){
	int i, j;
	int valid_source;

	for (i=0; i < mesg_count; i++){
		switch (mesg[i].type) {
		case CWIID_MESG_IR:
			valid_source = 0;
			int xacum=0;
			int yacum=0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source++;
					xacum+=mesg[i].ir_mesg.src[j].pos[CWIID_X];
					yacum+=mesg[i].ir_mesg.src[j].pos[CWIID_Y];
				}
			}
			if (!valid_source) {
				updateleds(-1);
			}else{
			  updateleds(xacum/valid_source);
			  updateCube(xacum/valid_source,yacum/valid_source);
			}
			break;
		default:
			printf("Unknown Report");
			break;
		}
	}
}
/* // versión que espera a que se conecte si o si
void detectwiimote(){ // toda esta funcion deberia ser no bloqueante 
  if (wiimote) return;
  cwiid_set_err(err);
  // conectarse al primer wiimote que se encuentre
  bdaddr_t bdaddr = *BDADDR_ANY;// bluetooth device address

  // Connect to the wiimote s
  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
  while (!(wiimote = cwiid_open(&bdaddr, 0))){
	  fprintf(stderr, "Waiting for a wiimote to connect to. (press 1+2)\n");
  }
  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
	  fprintf(stderr, "Unable to set message callback\n");
  }

  if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
	  fprintf(stderr, "Error enabling messages\n");
  }
  unsigned char rpt_mode = 0;
  toggle_bit(rpt_mode, CWIID_RPT_IR);
  set_rpt_mode(wiimote, rpt_mode);

  sprintf(mensaje,"Wiimote connected!\n");
}*/

void detectwiimote(){ // toda esta funcion deberia ser no bloqueante
  if (wiimote) return;
  cwiid_set_err(err);

  // conectarse al primer wiimote que se encuentre
  bdaddr_t bdaddr = *BDADDR_ANY;// bluetooth device address

  // Connect to the wiimote s
  sprintf(mensaje,"Put Wiimote in discoverable mode now (press 1+2)...\n");
  display();
  if ( !(wiimote = cwiid_open(&bdaddr, 0)) && !(wiimote = cwiid_open(&bdaddr, 0)) ){
	  sprintf(mensaje,"Wiimote not found\n");
	  display();
  }else{
    if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
	    fprintf(stderr, "Unable to set message callback\n");
    }
    if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
	    fprintf(stderr, "Error enabling messages\n");
    }
    unsigned char rpt_mode = 0;
    toggle_bit(rpt_mode, CWIID_RPT_IR);
    set_rpt_mode(wiimote, rpt_mode);
    sprintf(mensaje,"Wiimote connected!\n");
  }
}



int main(int argc, char *argv[]){
	
	led_pos=0;
        framen=0;
        sprintf(mensaje,"Keys: w a s d x space c esc\n");
	//detectwiimote();

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (960, 600); 
	glutInitWindowPosition (0,0);
	glutCreateWindow (argv[0]);
	//glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
	//glutEnterGameMode(); //set glut to fullscreen using the settings in the line above
	init ();
	glutDisplayFunc(display); 
	glutKeyboardFunc(keyboard);
	glutIdleFunc(glutPostRedisplay);
	glutMainLoop();

	return 0;
}