#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TrackingPFC_client.h>
#include <quat.h>
#include <time.h>

#define ALTO 8
#define ANCHO 50
#define FONDO 150
#define CERCA 50

#define MAXBALLSIZE 3
#define MINBALLSIZE 1
#define BALLSUB 32
#define TOTALBALLS 75

#define UPDATETIME 0.033 // 30 frames por segundo



// Variables auxiliares para los mensajes
GLint framen;
GLchar mensaje[100];

// Cliente
TrackingPFC_client* track;

int winx, winy;
float aspectratio;

bool useht;

// control de tiempo para el redraw
struct timeval lastframeupdate;
struct timezone tz;

// struct ball
typedef struct ball{
  float pos[3];
  float col[3];
  float size;
} ball;
ball balls[TOTALBALLS];

// inicializaciones
void init(void){

  // color de fondo
  glClearColor (0.0, 0.0, 0.0, 0.0);

  // tipo de shader
  glShadeModel (GL_SMOOTH);

  // activamos los modos
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  // luces
  GLfloat lightZeroPosition[] = {5, 0, 5, 1.0};
  GLfloat lightZeroColor[] = {0.5, 0.5, 0.5, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
  glEnable(GL_LIGHT0);
  GLfloat lightOnePosition[] = {-5, 0, 5, 1.0};
  GLfloat lightOneColor[] = {0.5, 0.5, 0.5, 1.0};
  glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
  glEnable(GL_LIGHT1);

  static float modelAmb[4] = {0.2, 0.2, 0.2, 1.0};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmb);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

  static float matAmb[4] = {0.2, 0.2, 0.2, 1.0};
  static float matDiff[4] = {0.8, 0.8, 0.8, 1.0};
  static float matSpec[4] = {0.4, 0.4, 0.4, 1.0};
  static float matEmission[4] = {0.0, 0.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
  glMaterialfv(GL_FRONT, GL_EMISSION, matEmission);
  glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  static float fog_color[] = {0.0, 0.0, 0.0, 1.0};
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogf(GL_FOG_DENSITY, 0.0125);
  glFogfv(GL_FOG_COLOR, fog_color);
}

// Funcion auxiliar para mostrar mensajes
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void output(float x, float y, char *string){
  glDisable(GL_LIGHTING);
  glColor3f(1.0, 1.0, 1.0);
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
  glEnable(GL_LIGHTING);
}

inline float randomf(float rango){
  return ((float)rand()/(float)RAND_MAX)*rango;
}

void resetballs(){
  srand ( time(NULL) );
  for (int i =0; i<TOTALBALLS;i++){
    balls[i].pos[0]=randomf(ANCHO*2)-ANCHO;
    // posicion vertical, puede ir de ballsize/2-alto a alto-ballsize/2
    balls[i].pos[1]=randomf(ALTO*2)-ALTO;
    // puede ir de ballsize/2-FONDO a CERCA-ballsize/2
    balls[i].pos[2]=-randomf(CERCA+FONDO)+CERCA;
    //colores
    balls[i].col[0]=randomf(1);
    balls[i].col[1]=randomf(1);
    balls[i].col[2]=randomf(1);
    // tamaño
    balls[i].size = randomf(MAXBALLSIZE-MINBALLSIZE)+MINBALLSIZE;
  }
}
void drawballs(){
  for (int i =0; i<TOTALBALLS;i++){
    GLfloat ballcolor[4] = {balls[i].col[0], balls[i].col[1], balls[i].col[2], 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ballcolor);
    glTranslatef(balls[i].pos[0], balls[i].pos[1], balls[i].pos[2]);
    glutSolidSphere(balls[i].size,BALLSUB,BALLSUB);
    glTranslatef(-balls[i].pos[0], -balls[i].pos[1], -balls[i].pos[2]);
  }
}
void display(void){

  GLfloat znear =0.1;
  GLfloat zfar =FONDO+100.0;

  float* pos = track->getlastpos();;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  // segun si tenemos activado o no el HT, llamamos
  // a la funcion del cliente o a la de glu
  if (useht)
    track->htadjustPerspective(znear, zfar);
  else
    gluPerspective(45.0, aspectratio, znear, zfar);

  glMatrixMode (GL_MODELVIEW);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();
  
  // segun si esta activado o no el HT, llamamos a gluLookAt o a la del cliente
  if (useht){
    track->setvirtualdisplaysize( 16.0);
    track->htgluLookAt (0, 0, 0,  0, 0, -1.0,  0.0, 1.0, 0.0);
  }else{
    gluLookAt(0, 0, 12,  0, 0, -1.0,  0.0, 1.0, 0.0);
  }

  glEnable(GL_FOG);
  glDisable(GL_LIGHTING);
    glLineWidth(1.5);
  float i;
  glColor3f(0.4, 0.7, 0.99);
  // lineas paralelas
  for (i=-FONDO;i<=0.0;i+=5.0){
   glBegin(GL_LINES);
    glVertex3f(-ANCHO, -ALTO-MAXBALLSIZE, i);
    glVertex3f( ANCHO, -ALTO-MAXBALLSIZE, i);
    glVertex3f(-ANCHO,  ALTO+MAXBALLSIZE, i);
    glVertex3f( ANCHO,  ALTO+MAXBALLSIZE, i);
   glEnd();
  }
  // perpendiculares
  for (i=-ANCHO;i<=ANCHO;i+=5.0){
   glBegin(GL_LINES);
    glVertex3f( i,  ALTO+MAXBALLSIZE,   0);
    glVertex3f( i,  ALTO+MAXBALLSIZE, -FONDO);
    glVertex3f( i, -ALTO-MAXBALLSIZE,   0);
    glVertex3f( i, -ALTO-MAXBALLSIZE, -FONDO);
   glEnd();
  }
  glEnable(GL_LIGHTING);
  glDisable(GL_FOG);

  

  
  // añadimos mensajes
  char buffer[160];
  sprintf(buffer, "HeadTrack %s", useht?"on":"off");   
  output(5.3,-4.0,buffer );
  sprintf(buffer, "Frame %i", framen);   
  output(5.3,-4.5,buffer );
  framen++;
  output(-7.0, 4.5, mensaje );
  sprintf(buffer, "X %f", pos[0]);   
  output(-7,-3.5,buffer ); 
  sprintf(buffer, "Y %f", pos[1]);   
  output(-7,-4.0,buffer ); 
  sprintf(buffer, "Z %f", pos[2]);   
  output(-7,-4.5,buffer );

  drawballs();

  glutSwapBuffers(); //swap the buffers
}

// gestion de cambio de tamaño
void reshape (int w, int h){
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  winx=w;
  winy=h;
  aspectratio=(float)w/(float)h;
}

// gestion de teclado
void keyboard(unsigned char key, int x, int y){
  switch (key) {
    case 27: // esc
	//glutLeaveGameMode(); //set the resolution how it was
	delete(track);
	exit(0);
	break;
    case 104: // h
	useht=!useht;
	break;
    case 114: // r
	resetballs();
	break;
    default:
      printf("Key %i not supported\n", key);
      break;
  }
}
// funcion auxiliar para calcular la diferencia (en segundos, con precision de microsecs)
double diff(struct timeval * x,struct timeval * y){
  double secs = x->tv_sec-y->tv_sec;
  double usecs = x->tv_usec-y->tv_usec;
  usecs=usecs/1000000.0;
  return (double)(secs+usecs);
}

// funcion que repinta la escena
void redraw(){
  // solo repintamos si ha pasado UPDATETIME
  struct timeval current;
  gettimeofday(&current, &tz);
  double sincelast=diff(&current, &lastframeupdate);
  
  if ( sincelast>=UPDATETIME){
    // updateamos
    lastframeupdate.tv_sec= current.tv_sec;
    lastframeupdate.tv_usec= current.tv_usec;
    glutPostRedisplay();
  }
}

int main(int argc, char** argv)
{
  // marcamos flag de usar ht a falso
  useht=true;
  
  // inicializamos el aspect ratio a 1,6
  winx=960;
  winy=600;
  aspectratio=(float)winx/(float)winy;

  framen=0;
  sprintf(mensaje,"Keys:  esc-> exit   h->Headtrack   r->reset\n");

  resetballs();

  // inicializamos lastframeupdate
  gettimeofday(&lastframeupdate, &tz);

  char* trkname = (char*)"Tracker0@localhost";
  // si se ha llamado con un parametro, asumimos que es un nombre de tracker alternativo
  // se espera un nombre valido y libre, si no lo es, la aplicación fallara
  if (argc>1)
    trkname=argv[1];

  track = new TrackingPFC_client(trkname);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (winx, winy); 
  glutInitWindowPosition (0,0);
  glutCreateWindow (argv[0]);
  //glutGameModeString( "1920x1200:32@60" ); //the settings for fullscreen mode
  /*glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
  glutEnterGameMode(); //set glut to fullscreen using the settings in the line above*/
  init ();
  glutDisplayFunc(display); 
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(redraw);
  glutMainLoop();
return 0;
}
