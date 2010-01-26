#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TrackingPFC_client.h>
#include <quat.h>
#include <time.h>

// Tamaños de la sala
#define ALTO 8
#define ANCHO 50
#define FONDO 150
#define CERCA 50

// Tamaño y numero de pelotas
#define MAXBALLSIZE 3
#define MINBALLSIZE 1
#define BALLSUB 32
#define TOTALBALLS 75

// Framerate
#define UPDATETIME 0.016 // 60 frames por segundo

// Tiempo tras el cual deja de considerarse activo un sensor
#define ALIVETIME 0.2

// Variables auxiliares para los mensajes
GLint framen;
GLchar mensaje[100];

// Cliente
TrackingPFC_client* track;

// Propiedades de la ventana
int winx, winy;
float aspectratio;
// informacion del mouse
int mousepos[2];
bool buttonpressed;

// Flags de control
bool useht; // usar HT

// control de tiempo para el redraw
struct timeval lastframeupdate;
struct timezone tz;

// Informacion sobre las esferas
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

  // Material por defecto
  static float matAmb[4] = {0.2, 0.2, 0.2, 1.0};
  static float matDiff[4] = {0.8, 0.8, 0.8, 1.0};
  static float matSpec[4] = {0.4, 0.4, 0.4, 1.0};
  static float matEmission[4] = {0.0, 0.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);
  glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
  glMaterialfv(GL_FRONT, GL_EMISSION, matEmission);
  glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

  // Antialiasing para las lineas
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  // Niebla
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
  
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
  glEnable(GL_LIGHTING);
}

// Funcion auxiliar para dar un random float entre 0 y 1
inline float randomf(float rango){
  return ((float)rand()/(float)RAND_MAX)*rango;
}

// Resetea posicion, tamaño y color de las esferas
void resetballs(){
  srand ( time(NULL) );
  for (int i =0; i<TOTALBALLS;i++){
    // posiciones
    balls[i].pos[0]=randomf(ANCHO*2)-ANCHO;
    balls[i].pos[1]=randomf(ALTO*2)-ALTO;
    balls[i].pos[2]=-randomf(CERCA+FONDO)+CERCA;
    //colores
    balls[i].col[0]=randomf(1);
    balls[i].col[1]=randomf(1);
    balls[i].col[2]=randomf(1);
    // tamaño
    balls[i].size = randomf(MAXBALLSIZE-MINBALLSIZE)+MINBALLSIZE;
  }
}
// Dibuja las esferas
void drawballs(){
  for (int i =0; i<TOTALBALLS;i++){
    // color
    GLfloat ballcolor[4] = {balls[i].col[0], balls[i].col[1], balls[i].col[2], 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ballcolor);
    // posicion
    glPushMatrix();
    glTranslatef(balls[i].pos[0], balls[i].pos[1], balls[i].pos[2]);
    // la esfera
    glutSolidSphere(balls[i].size,BALLSUB,BALLSUB);
    // volvemos a la posicion original
    glPopMatrix();
  }
}

// funcion auxiliar para calcular la magnitud de un vector
float magnitud( float* p1, float* p2 ){
    
  float aux[3];
  aux[0] = p2[0] - p1[0];
  aux[1] = p2[1] - p1[1];
  aux[2] = p2[2] - p1[2];
  return (float)sqrt( aux[0] * aux[0] + aux[1] * aux[1] + aux[2] * aux[2] );
}

float distanciapuntolinea( float *p, float *l1, float *l2, float* fact){
    
    float length = magnitud( l2, l1 );
    float u = ( ( ( p[0] - l1[0] ) * ( l2[0] - l1[0] ) ) +
        ( ( p[1] - l1[1] ) * ( l2[1] - l1[1] ) ) +
        ( ( p[2] - l1[2] ) * ( l2[2] - l1[2] ) ) ) /
        ( length * length );
    // if( U < 0.0f || U > 1.0f ) -> el punto no cae en el segmento
    float intersec[3]; 
    intersec[0] = l1[0] + u * ( l2[0] - l1[0] );
    intersec[1] = l1[1] + u * ( l2[1] - l1[1] );
    intersec[2] = l1[2] + u * ( l2[2] - l1[2] );
    
    *fact=u;
    return magnitud( p, intersec );
}

// Dibuja el rayo y calcula las intersecciones
void ray(){
  float* pos = track->getlastpos(1);
  if (pos==NULL || pos[7]>ALIVETIME){
    glColor3f(1.0, 0.0, 0.0);
    char buffer[160];
    sprintf(buffer, "No hay sensor!");   
    output(5.3,-3.5,buffer );
  }else{	
    // calculamos la posicion del sensor
    float scale = track->getscale();
    pos[0]*=scale;
    pos[1]*=scale;
    pos[2]*=scale;
    // la del cursor
    float curspos[3];
    curspos[0]=(((float)mousepos[0]/(float)winx)-0.5)*16.0;
    curspos[1]=-(((float)mousepos[1]/(float)winy)-0.5)*10.0;
    curspos[2]=0.0; // el cursor esta en el plano de la pantalla
    
    // obtenemos el vector distancia
    float dist[3];
    dist[0]=curspos[0]-pos[0];
    dist[1]=curspos[1]-pos[1];
    dist[2]=curspos[2]-pos[2];

    float aux;
    int ind=-1;
    float fact = 999;
    // recorremos las esferas
    for (int i =0; i<TOTALBALLS;i++){
      // comprobamos si el rayo intersecta
      if (distanciapuntolinea(balls[i].pos,pos, curspos, &aux) < balls[i].size){
	//(en aux esta el factor por el que habira que multiplicar el rayo para llegar a esa esfera
	if (aux<fact){
	  // si es la mas cercana , guardamos sus datos
	  ind = i;
	  fact=aux;
	}
      }
    }
    // si al llegar aqui ind==-1 es que no intersectamos con ninguna bola
    // si no, tenemos el indice de la 1a esfera que intersecta
    if (ind!=-1){
      
      // reducimos la esfera
      balls[ind].size=balls[ind].size/1.1;
      
      float fondopos[3];
      // sumamos curpos+fact*dist para obtener el rayo completo
      fondopos[0]=pos[0]+dist[0]*fact;
      fondopos[1]=pos[1]+dist[1]*fact;
      fondopos[2]=pos[2]+dist[2]*fact;

      // dibujamos el rayo
      glEnable(GL_FOG);
      glDisable(GL_LIGHTING);
      glLineWidth(8.0);
      glColor4f(0.9, 0.1, 0.1, 0.7);
      glBegin(GL_LINES);
	glVertex3f(fondopos[0],fondopos[1],fondopos[2]);
	glVertex3f(pos[0],pos[1],pos[2]);
      glEnd();
      glEnable(GL_LIGHTING);
      glDisable(GL_FOG);

    }else{
    
      // obtenemos el factor por el que hay que multiplicar distancia para llegar a zfar
      fact = -(FONDO+100.0)/dist[2];

      // y obtenemos en que punto del fondo va a parar el rayo
      float fondopos[3];
      // sumamos curpos+fact*dist para obtener el rayo completo
      fondopos[0]=curspos[0]+dist[0]*fact;
      fondopos[1]=curspos[1]+dist[1]*fact;
      fondopos[2]=curspos[2]+dist[2]*fact;

      // dibujamos el rayo

      glEnable(GL_FOG);
      glDisable(GL_LIGHTING);
      glLineWidth(8.0);
      glColor4f(0.3, 0.8, 0.99, 0.7);
      glBegin(GL_LINES);
	glVertex3f(fondopos[0],fondopos[1],fondopos[2]);
	glVertex3f(pos[0],pos[1],pos[2]);
      glEnd();
      glEnable(GL_LIGHTING);
      glDisable(GL_FOG);
    }
  }
}
// Redibuja la escena
void display(void){

  GLfloat znear =0.1;
  GLfloat zfar =FONDO+100.0;

  // Obtenemos la posicion del sensor 0 (el usuario)
  float* pos = track->getlastpos();

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

  // Lineas del techo y el suelo
  // activamos niebla y desactivamos la iluminacion
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
  // devolvemos la niebla y la iluminacion a su estado normal
  glEnable(GL_LIGHTING);
  glDisable(GL_FOG);
  
  // añadimos mensajes
  glColor3f(1.0, 1.0, 1.0);
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

  // Dibujamos la escena
  drawballs();
  if (buttonpressed)
    ray();

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
    case 104: // h -> activar/desactivar HT
	useht=!useht;
	break;
    case 114: // r -> resetear esferas
	resetballs();
	break;
    default:
      printf("Key %i not supported\n", key);
      break;
  }
}

// funcion auxiliar para calcular tiempos (en segundos, con precision de microsecs)
double diff(struct timeval * x,struct timeval * y){
  double secs = x->tv_sec-y->tv_sec;
  double usecs = x->tv_usec-y->tv_usec;
  usecs=usecs/1000000.0;
  return (double)(secs+usecs);
}

// funcion que controla los fps
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


void motion(int x, int y){
  float selx, sely;
  mousepos[0] = x;
  mousepos[1] = y;
  lastframeupdate.tv_sec--;
  redraw();
}

void mouse(int b, int s, int x, int y){
  if (s == GLUT_DOWN) {
    buttonpressed=true;
  } else {
    buttonpressed=false;
  }
  motion(x, y);
}


int main(int argc, char** argv)
{
  // marcamos flag de usar ht a cierto
  useht=true;
  
  // inicializamos las propiedades de la ventana y el raton
  winx=960;
  winy=600;
  aspectratio=(float)winx/(float)winy;
  mousepos[0]=0;
  mousepos[1]=0;
  buttonpressed=false;

  // frames y mensaje
  framen=0;
  sprintf(mensaje,"Keys:  esc-> exit   h->Headtrack   r->reset\n");

  // inicializamos las esferas
  resetballs();

  // inicializamos lastframeupdate
  gettimeofday(&lastframeupdate, &tz);

  // Inicializamos el cliente
  char* trkname = (char*)"Tracker0@localhost";
  // si se ha llamado con un parametro, asumimos que es un nombre de tracker alternativo
  // se espera un nombre valido y libre, si no lo es, la aplicación fallara
  if (argc>1)
    trkname=argv[1];
  track = new TrackingPFC_client(trkname);

  // preparamos opengl
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
  glutMotionFunc(motion);
  glutMouseFunc(mouse);
  glutIdleFunc(redraw);
  glutMainLoop();
  return 0;
}
