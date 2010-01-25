#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TrackingPFC_client.h>
#include <quat.h>
#include <time.h>



#define PI 3.14159265

// Defines de movimiento
#define MOVUNIT 0.06
#define ROTUNIT 1.5

#define UPDATETIME 0.03 // 33.3 frames por segundo

#define TRESHOLD 0.4 // angulo (en radianes) a partir del que se detecta

// Cliente
TrackingPFC_client* track;

// propiedades de la ventana
int winx, winy;
double aspectratio;

// Colores
GLfloat lightskinColor[4] = {233.0/355.0, 132.0/255.0, 20.0/255.0, 1.0};
GLfloat darkskinColor[4] = {197.0/355.0, 112.0/255.0, 58.0/255.0, 1.0};
GLfloat eyesColor[4] = {0.0, 0.0, 0.1, 1.0};
GLfloat floorColor[4] = {0.5, 0.5, 0.5, 1.0};

// datos relativos al robot
double pos[3]; // pos x, pos y, rotacion
struct timeval lastupdate;
struct timezone tz;
  


// inicializaciones
void init(void){
  // color de fondo
  glClearColor (0.0, 0.0, 0.0, 0.0);
  // tipo de shader
  glShadeModel (GL_FLAT);
  // activamos los modos
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  // luces
  GLfloat lightZeroPosition[] = {5, 0, 5, 1.0};
  GLfloat lightZeroColor[] = {1.0, 1.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
  glEnable(GL_LIGHT0);
  GLfloat lightOnePosition[] = {-5, 0, 5, 1.0};
  GLfloat lightOneColor[] = {1.0, 1.0, 1.0, 1.0};
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
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

  static float fog_color[] = {0.0, 0.0, 0.0, 1.0};
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_EXP);
  glFogf(GL_FOG_DENSITY, 0.0125);
  glFogfv(GL_FOG_COLOR, fog_color);

}

// funcion auxiliar para calcular las normales
void calcNorm(float v0[3],float v1[3],float v2[3]){
  q_vec_type vec1, vec2, vn;
  q_vec_set(vec1, v0[0]-v1[0], v0[1]-v1[1], v0[2]-v1[2]);
  q_vec_set(vec2, v0[0]-v2[0], v0[1]-v2[1], v0[2]-v2[2]);
  q_vec_cross_product(vn,vec1,vec2);
  glNormal3f(vn[Q_X],vn[Q_Y],vn[Q_Z]);
}
void poly(float v0[3],float v1[3],float v2[3], float v3[3]){
  calcNorm(v0,v1,v2);
  glBegin(GL_POLYGON);
    glVertex3fv(v0);glVertex3fv(v1);glVertex3fv(v2);glVertex3fv(v3);
  glEnd();
}

void leye(){
  float v0[]={-1.5,1,-3}; float v1[]={2.5, 1,-3}; float v2[]={2.6, 0,-3};
  float v3[]={1.5, -1,-3}; float v4[]={-0.5, -1,-3};

  float v5[]={-1.5,1,3}; float v6[]={2.5, 1,3}; float v7[]={2.6, 0,3};
  float v8[]={1.5, -1,3}; float v9[]={-0.5, -1,3};

  float v10[]={-1.3,0.9,3}; float v11[]={2.3, 0.9,3}; float v12[]={2.4, 0,3};
  float v13[]={1.4, -0.9,3}; float v14[]={-0.3, -0.9,3};

  float v15[]={-1.3,0.9,2.5}; float v16[]={2.3, 0.9,2.5}; float v17[]={2.4, 0,2.5};
  float v18[]={1.4, -0.9,2.5}; float v19[]={-0.3, -0.9,2.5};

  glMaterialfv(GL_FRONT, GL_DIFFUSE, lightskinColor);

  // tapa trasera
  calcNorm(v0,v1,v2);
  glBegin(GL_POLYGON);
    glVertex3fv(v0);glVertex3fv(v1);glVertex3fv(v2);glVertex3fv(v3);glVertex3fv(v4);
  glEnd();
  // tapas laterales
  poly(v5,v6,v1,v0);
  poly(v9,v5,v0,v4);
  poly(v8,v9,v4,v3);
  poly(v7,v8,v3,v2);
  poly(v6,v7,v2,v1);
  // bordes
  poly(v6,v5,v10,v11);
  poly(v7,v6,v11,v12);
  poly(v8,v7,v12,v13);
  poly(v9,v8,v13,v14);
  poly(v5,v9,v14,v10);
  // borde interior
  poly(v11,v10,v15,v16);
  poly(v12,v11,v16,v17);
  poly(v13,v12,v17,v18);
  poly(v14,v13,v18,v19);
  poly(v10,v14,v19,v15);
  // tapa delantera
  calcNorm(v19,v18,v17);
  glBegin(GL_POLYGON);
    glVertex3fv(v19);glVertex3fv(v18);glVertex3fv(v17);glVertex3fv(v16);glVertex3fv(v15);
  glEnd();

  glMaterialfv(GL_FRONT, GL_DIFFUSE, eyesColor);
  glTranslatef(0,0,2.0);
  glutSolidSphere (0.75,32,32);
  glTranslatef(0,0,-2.0);
}

void reye(){
  float v0[]={1.5,1,-3}; float v1[]={-2.5, 1,-3}; float v2[]={-2.6, 0,-3};
  float v3[]={-1.5, -1,-3}; float v4[]={0.5, -1,-3};

  float v5[]={1.5,1,3}; float v6[]={-2.5, 1,3}; float v7[]={-2.6, 0,3};
  float v8[]={-1.5, -1,3}; float v9[]={0.5, -1,3};

  float v10[]={1.3,0.9,3}; float v11[]={-2.3, 0.9,3}; float v12[]={-2.4, 0,3};
  float v13[]={-1.4, -0.9,3}; float v14[]={0.3, -0.9,3};

  float v15[]={1.3,0.9,2.5}; float v16[]={-2.3, 0.9,2.5}; float v17[]={-2.4, 0,2.5};
  float v18[]={-1.4, -0.9,2.5}; float v19[]={0.3, -0.9,2.5};

  glMaterialfv(GL_FRONT, GL_DIFFUSE, lightskinColor);
  // tapa trasera
  calcNorm(v4,v3,v2);
  glBegin(GL_POLYGON);
    glVertex3fv(v4);glVertex3fv(v3);glVertex3fv(v2);glVertex3fv(v1);glVertex3fv(v0);
  glEnd();
  // tapas laterales
  poly(v0,v1,v6,v5);
  poly(v4,v0,v5,v9);
  poly(v3,v4,v9,v8);
  poly(v2,v3,v8,v7);
  poly(v1,v2,v7,v6);
  // bordes
  poly(v11,v10,v5,v6);
  poly(v12,v11,v6,v7);
  poly(v13,v12,v7,v8);
  poly(v14,v13,v8,v9);
  poly(v10,v14,v9,v5);
  // borde interior
  poly(v16,v15,v10,v11);
  poly(v17,v16,v11,v12);
  poly(v18,v17,v12,v13);
  poly(v19,v18,v13,v14);
  poly(v15,v19,v14,v10);
  // tapa delantera
  calcNorm(v15,v16,v17);
  glBegin(GL_POLYGON);
    glVertex3fv(v15);glVertex3fv(v16);glVertex3fv(v17);glVertex3fv(v18);glVertex3fv(v19);
  glEnd();

  glMaterialfv(GL_FRONT, GL_DIFFUSE, eyesColor);
  glTranslatef(0,0,2.0);
  glutSolidSphere (0.75,32,32);
  glTranslatef(0,0,-2.0);
}
void eyes(float eyes, float head){
  glRotatef(head,1,0,0);
  glRotatef(-eyes, 0,0,1);
  glTranslatef(1.6,0.3,0);
  leye();
  glTranslatef(-1.6,-0.3,0);
  glRotatef(eyes, 0,0,1);

  glRotatef(eyes, 0,0,1);
  glTranslatef(-1.6,0.3,0);
  reye();
  glTranslatef(1.6,-0.3,0);
  glRotatef(-eyes, 0,0,1);

  float v0[]={ 1.6,  0.3,  1};
  float v1[]={ 1.6,  0.3, -1};
  float v2[]={ 1.6, -0.3, -1};
  float v3[]={ 1.6, -0.3,  1};
  float v4[]={-1.6,  0.3,  1};
  float v5[]={-1.6,  0.3, -1};
  float v6[]={-1.6, -0.3, -1};
  float v7[]={-1.6, -0.3,  1};

  glMaterialfv(GL_FRONT, GL_DIFFUSE, darkskinColor);
  // laterales
  poly(v0,v3,v2,v1);
  poly(v4,v5,v6,v7);
  //frente
  poly(v0,v4,v7,v3);
  // arriba
  poly(v1,v5,v4,v0);
  // atras
  poly(v2,v6,v5,v1);
  // abajo
  poly(v3,v7,v6,v2);
  glRotatef(-head,1,0,0);
}
void body(){
  float v0[]={ 0.3,  0.0,  0.3};
  float v1[]={ 0.3,  0.0, -0.3};
  float v2[]={ 0.3, -5.0, -0.3};
  float v3[]={ 0.3, -5.0,  0.3};
  float v4[]={-0.3 , 0.0,  0.3};
  float v5[]={-0.3 , 0.0, -0.3};
  float v6[]={-0.3 ,-5.0, -0.3};
  float v7[]={-0.3 ,-5.0,  0.3};
  
  glMaterialfv(GL_FRONT, GL_DIFFUSE, darkskinColor);
  // laterales
  poly(v0,v3,v2,v1);
  poly(v4,v5,v6,v7);
  //frente
  poly(v0,v4,v7,v3);
  // arriba
  poly(v1,v5,v4,v0);
  // atras
  poly(v2,v6,v5,v1);
  // abajo
  poly(v3,v7,v6,v2);
  glTranslatef(0,-9,0);
  glutSolidCube(10);
  glTranslatef(0,9,0);
}
// funcion que recomputa el movimiento
// solo uno de los valores puede ser distinto de 0
void move(float forward, float left, float ccw){
  // pos[0] -> posicion x, pos[1]->posicion y, pos[2] -> rotacion
  if (ccw!=0){
    pos[2]+=ccw;
  }
  if (forward!=0){
    pos[0]=pos[0]+sin(2*PI*pos[2]/360.0)*forward;
    pos[1]=pos[1]+cos(2*PI*pos[2]/360.0)*forward;
  }
  if (left!=0){
    pos[0]=pos[0]+sin(2*PI*(90-pos[2])/360.0)*left;
    pos[1]=pos[1]-cos(2*PI*(90-pos[2])/360.0)*left;
  }
  // correcciones
  if(pos[0]<-5)pos[0]=-5;
  if(pos[0]>5)pos[0]=5;
  if(pos[1]<-5)pos[1]=-5;
  if(pos[1]>5)pos[1]=5;
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
  double sincelast=diff(&current, &lastupdate);
  
  if ( sincelast>=UPDATETIME){
    // updateamos
    lastupdate.tv_sec= current.tv_sec;
    lastupdate.tv_usec= current.tv_usec;
    glutPostRedisplay();
  }
}
// funcion que updatea la posicion si es necesario
void updatepos(){
  // obtenemos los datos del tracker
  float* pos = track->getlastpos();
  // creamos el QUAT
  q_type rot;
  rot[Q_X]=pos[3];
  rot[Q_Y]=pos[4];
  rot[Q_Z]=pos[5];
  rot[Q_W]=pos[6];
  // obtenemos los angulos de euler
  q_vec_type euler;
  q_to_euler(euler, rot);
  // Assumes roll is rotation about X, pitch is rotation about Y, yaw is about Z.
  // void q_to_euler(q_vec_type yawPitchRoll, const q_type q);

  // avance
  double fwd = 0.0;
  if (euler[Q_Z]<-TRESHOLD)
    fwd=-MOVUNIT;
  if (euler[Q_Z]>TRESHOLD)
    fwd=MOVUNIT;
  
  // strafe
  double left = 0.0;
  if (euler[Q_X]<-TRESHOLD)
    left=-MOVUNIT;
  if (euler[Q_X]>TRESHOLD)
    left=MOVUNIT;
  // giro
  double ccw = 0.0;
  if (euler[Q_Y]<-TRESHOLD)
    ccw=ROTUNIT;
  if (euler[Q_Y]>TRESHOLD)
    ccw=-ROTUNIT;


  move(fwd, left, ccw);
}
void display(void){

  GLfloat znear =1.0;
  GLfloat zfar =100.0;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gluPerspective(45.0, aspectratio, znear, zfar);

  glMatrixMode (GL_MODELVIEW);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();
  
  gluLookAt(0, 7, 12,  0, 0, 0,  0.0, 1.0, 0.0);

  // recalculamos las posiciones
  updatepos();

  glPushMatrix();
  glTranslatef(pos[0],0,pos[1]);
  glRotatef(pos[2],0,1,0);
  glTranslatef(0,1.4,0);
  glScalef(0.1,0.1,0.1);
  body();
  eyes(15,0);
  glPopMatrix();

  // suelo
  glPushMatrix();
  glMaterialfv(GL_FRONT, GL_DIFFUSE, floorColor);
  glTranslatef(0,-0.1,0);
  glScalef(11,0.2,11);
  glutSolidCube(1);
  glPopMatrix();
  

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
    case 119: // w (avanzar)
	move(MOVUNIT,0,0);
	break;
    case 115: // s (retroceder)
	move(-MOVUNIT,0,0);
	break;
    case 97: // a (girar izq)
	move(0,0,ROTUNIT);
	break;
    case 100: // d (girar der)
	move(0,0,-ROTUNIT);
	break;
    case 113: // q (izquierda)
	move(0,MOVUNIT,0);
	break;
    case 101: // e (derecha)
	move(0,-MOVUNIT,0);
	break;
    case 114: // r (reset)
	// reinicializamos las posiciones
	pos[0]=0.0;
	pos[1]=0.0;
	pos[2]=0.0;
  
	break;
    
    default:
      printf("Key %i not supported\n", key);
      break;
  }
}

int main(int argc, char** argv)
{
  
  
  // inicializamos el aspect ratio a 1,6
  winx=960;
  winy=600;
  aspectratio=(float)winx/(float)winy;

  // inicializamos las posiciones
  pos[0]=0.0;
  pos[1]=0.0;
  pos[2]=0.0;
  // y el lastupdate
  gettimeofday(&lastupdate, &tz);

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
