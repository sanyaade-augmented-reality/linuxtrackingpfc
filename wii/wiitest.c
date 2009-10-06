#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwiid.h>
#include <GL/glut.h>

cwiid_mesg_callback_t cwiid_callback;

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	// comento para que no salga nada de errores
	/*if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");*/
}

int led_pos;				
cwiid_wiimote_t *wiimote;	/* wiimote handle */
void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
	if (cwiid_set_led(wiimote, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
	}
}
	
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
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
void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp)
{
	int i, j;
	int valid_source;

	for (i=0; i < mesg_count; i++)
	{
		switch (mesg[i].type) {
		case CWIID_MESG_IR:
			valid_source = 0;
			int xacum=0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source++;
					xacum+=mesg[i].ir_mesg.src[j].pos[CWIID_X];
				}
			}
			if (!valid_source) {
				updateleds(-1);
			}else{
			  updateleds(xacum/valid_source);
			}
			//printf("\n");
			break;
		
		default:
			printf("Unknown Report");
			break;
		}
	}
}



GLfloat light_diffuse[] = {1.0, 0.5, 0.0, 1.0};  /* Orange diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
  {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3];  /* Will be filled in with X,Y,Z vertexes. */

int frame=0;

void
drawBox(void)
{
  int i;
  printf("Drawing frame: %i\n", frame);
  frame++;
  for (i = 0; i < 6; i++) {
    glBegin(GL_QUADS);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawBox();
  glutSwapBuffers();
}

void
animate(void)
{
  //glutPostRedisplay();
}

void
visibility(int state)
{
  if (state == GLUT_VISIBLE) {
    glutIdleFunc(animate);
  } else {
    glutIdleFunc(NULL);
  }
}

void
init(void)
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

  /* Enable a single OpenGL light. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  //glFrustum(-1, 1, -1, 1, 2, 6);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */


  /* Adjust cube position to be asthetic angle. */
  glTranslatef(0.0, 0.0, -1.0);
  glRotatef(60, 1.0, 0.0, 0.0);
  glRotatef(-20, 0.0, 0.0, 1.0);
}


int main(int argc, char *argv[])
{
	bdaddr_t bdaddr;	/* bluetooth device address */
	unsigned char mesg = 0;
	unsigned char rpt_mode = 0;
	led_pos=0;

	cwiid_set_err(err);

	  glutInit(&argc, argv);
	  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	  glutCreateWindow("Wii Test");
	  glutDisplayFunc(display);
	  glutVisibilityFunc(visibility);
	  init();


	/* Connect to address given on command-line, if present */
	if (argc > 1) {
		str2ba(argv[1], &bdaddr);
	}
	else {
		bdaddr = *BDADDR_ANY;
	}

	/* Connect to the wiimote */
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
	else {
		mesg = 1;
	}
	toggle_bit(rpt_mode, CWIID_RPT_IR);
	set_rpt_mode(wiimote, rpt_mode);

	//	printf("Wiimote succesfully connected. Press Enter button to exit\n");
	//	getchar();
	glutMainLoop();
	  


	if (cwiid_close(wiimote)) {
		fprintf(stderr, "Error on wiimote disconnect\n");
		return -1;
	}

	return 0;
}