#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <TrackingPFC_client.h>
#include <quat.h>
#include <time.h>

// Variables auxiliares para los mensajes
GLint framen;
GLchar mensaje[100];

// Cliente
TrackingPFC_client* track;

// flags de funcionamiento
bool useht; // flag de HT on/off
int mode; // modo (follow, imitate, stop
bool old; // flag de usar el modelo viejo

#define FOLLOW 0
#define IMITATE 1
#define STOP 2

int winx, winy;
double aspectratio;

// variables auxiliares para el calculo de usuarios
int lifeforms;
int diffcount;
int mainuser;
int newuser;
time_t newusertime;
bool followingnewuser;
bool thereismainuser;

// datos sobre los reports
int lastreport;
int lastreportsize;
vector<time_t> lasttime;
vector<double*> lastpos;
GLfloat lightskinColor[4] = {233.0/355.0, 132.0/255.0, 20.0/255.0, 1.0};
GLfloat darkskinColor[4] = {197.0/355.0, 112.0/255.0, 58.0/255.0, 1.0};
GLfloat eyesColor[4] = {0.0, 0.0, 0.1, 1.0};

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
void eyes(float angle){
  glRotatef(-angle, 0,0,1);
  glTranslatef(1.6,0.3,0);
  leye();
  glTranslatef(-1.6,-0.3,0);
  glRotatef(angle, 0,0,1);

  glRotatef(angle, 0,0,1);
  glTranslatef(-1.6,0.3,0);
  reye();
  glTranslatef(1.6,-0.3,0);
  glRotatef(-angle, 0,0,1);

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

// Funcion auxiliar que procesa el datachunk actual y updatea los datos
// devuelve el numero de usuarios detectados (en positivo si hay nuevos datos
// en negativo si no hay nuevos datos
int updateusers(TrackingPFC_data::datachunk* data ){
  bool newdata = false;
  // si es un report nuevo, o tiene mas datos que antes, updateamos
  if (data->getcount()>lastreport || lastreportsize<data->size()){
    // guardamos el nuevo report count
    lastreport=data->getcount();
    lastreportsize= data->size();
    // recorremos los puntos del report updateando
    int n = lastreportsize;
    for (int dn =0; dn<n;dn++){
      int sensor = data->gettag(dn);
      // comprobamos que no sea un sensor nuevo
      // si lo es, creamos sus datos
      while (lasttime.size()<=sensor){
	time_t t0 =0;
	lasttime.push_back(time(&t0));
	lastpos.push_back(new double[3]);
      }
      lasttime[sensor]=time(NULL);
      const double* aux = data->getdata(dn);
      lastpos[sensor][0]=aux[0];
      lastpos[sensor][1]=aux[1];
      lastpos[sensor][2]=aux[2];
    }// recorrido por los puntos
    newdata=true;
  } // updates por nuevo report
  
  // llegados aqui, tenemos en lasttime y lastpos los datos necesarios
  int valid=0;
  for (int s =0; s<lasttime.size();s++){
    if ( difftime(time(NULL),lasttime[s])<2)
      valid++;
  }
  lifeforms=valid;
  return newdata?valid:-valid;
}
   
void display(void){

  GLfloat znear =1.0;
  GLfloat zfar =100.0;

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
  glColor3f(0.2, 0.2, 0.2);
  for (i=-100.0;i<=0.0;i+=5.0){
   glBegin(GL_LINES);
    glVertex3f(-100.0, -5.0, i);
    glVertex3f( 100.0, -5.0, i);
    glVertex3f(-100.0,  5.0, i);
    glVertex3f( 100.0,  5.0, i);
   glEnd();
  }
  for (i=-100.0;i<=100.0;i+=5.0){
   glBegin(GL_LINES);
    glVertex3f( i,  5.0,   0);
    glVertex3f( i,  5.0, -100);
    glVertex3f( i, -5.0,   0);
    glVertex3f( i, -5.0, -100);
   glEnd();
  }
  glEnable(GL_LIGHTING);
  glDisable(GL_FOG);

  // añadimos mensajes
  char buffer[160];
  sprintf(buffer, "HeadTrack %s", useht?"on":"off");   
  output(5.3,-3.5,buffer );
  sprintf(buffer, "Mode: %s", (mode==FOLLOW)?"follow":((mode==IMITATE)?"imitate":"stop"));   
  output(5.3,-4.0,buffer );
  sprintf(buffer, "Frame %i", framen);   
  output(5.3,-4.5,buffer );
  framen++;
  output(-7.0, 4.5, mensaje );
  
  if (!old)
    body();

  if (mode==FOLLOW){
    TrackingPFC_data::datachunk* data = track->getdata()->getlastdata();
    // actualizamos las estructuras de datos
    int detectedusers= updateusers(data);

    // actualizamos diffcount y lifeform (solo si hay nuevos datos)
    if (detectedusers>0){
      if(detectedusers!=lifeforms)
	diffcount++;
      else
	diffcount-=2;
      if (diffcount<0) diffcount =0;
    }
    // si despues de 10 updates sigue habiendo discrepancias, se cambia el valor
    if (diffcount>10){
      lifeforms=detectedusers;
      diffcount=0;
    }

    // añadimos un mensaje adicional
    sprintf(buffer, "%i lifeforms", lifeforms);   
    output(5.3,-3.0,buffer ); 
    
    if (lifeforms>0){
      thereismainuser=true;
      // determinacion de usuario al que seguimos
      if (mainuser==-1 || lifeforms==1){
	for (int s =0; s<lasttime.size();s++){
	  if ( difftime(time(NULL),lasttime[s])<2){
	    if (mainuser!=s)
	      printf("Adquirido sensor %i como usario principal\n",s);
	    mainuser=s;
	  }
	}
      }

      // si hay nuevos usuarios y no estamos siguiendo ya a uno nuevo, 
      // lo deeterminamos
      if (lifeforms>1 && newuser==-1){
	for (int s =0; s<lasttime.size();s++){
	  if ( difftime(time(NULL),lasttime[s])<2 && s!=mainuser){
	    printf("Detectado nuevo usuario en el sensor %i\n",s);
	    newuser=s;
	    newusertime = time(NULL);
	  }
	}
      }else if (lifeforms==1 && newuser!=-1){
	newuser=-1;
	printf("El nuevo usuario ha desparecido\n");
      }

      int activeuser;
      if (newuser!=-1 && difftime(time(NULL),newusertime)<5){
	activeuser = newuser;
	followingnewuser=true;
      }else{
	activeuser = mainuser;
	if (followingnewuser){
	  followingnewuser=false;
	  printf("Se ha perdido el interes en el nuevo usuario\n");
	}
      }

      
      double* pos = lastpos[activeuser];
      // añadimos mensajes de posicion
      sprintf(buffer, "X %f", pos[0]);   
      output(-7,-3.5,buffer ); 
      sprintf(buffer, "Y %f", pos[1]);   
      output(-7,-4.0,buffer ); 
      sprintf(buffer, "Z %f", pos[2]);   
      output(-7,-4.5,buffer );

      q_vec_type vn, dir;
      // inversa al vector posicion
      dir[Q_X]=pos[0];
      dir[Q_Y]=pos[1];
      dir[Q_Z]=pos[2];
      // vector normal al display
      vn[Q_X]=0;
      vn[Q_Y]=0;
      vn[Q_Z]=1;
      // quaternion que pasa de vn a dir
      q_type diff;
      q_from_two_vecs(diff, vn, dir);
      if (!useht){
	// dividimos la rotacion a la 1/4 (por alguna razon el efecto queda mejor)
	// pero solo si no estamos usando HT
	q_type zero;
	zero[Q_X]=0;zero[Q_Y]=0;zero[Q_Z]=0;zero[Q_W]=1;
	q_slerp (diff, diff, zero, 0.5);
      }

      // obtenemos la matriz
      qogl_matrix_type mat;
      q_to_ogl_matrix(mat, diff);
      glMultMatrixd(mat);
    }else{
      // no hay formas de vida, no estamos siguiendo a ningun usuario
      mainuser=-1;
      if (thereismainuser){
	printf("No hay usuarios a la vista\n");
	thereismainuser=false;
	// bajamos otros flags para evitar mensajes extraños despues
	followingnewuser=false;
	newuser=-1;
      }
    }

  }else if (mode==IMITATE){ // modo imitacion
    // añadimos mensajes de posicion
    float* pos = track->getlastpos();
    sprintf(buffer, "X %f", pos[0]);   
    output(-7,-3.5,buffer ); 
    sprintf(buffer, "Y %f", pos[1]);   
    output(-7,-4.0,buffer ); 
    sprintf(buffer, "Z %f", pos[2]);   
    output(-7,-4.5,buffer );
    
    q_type diff;
    diff[Q_X]=pos[3];
    diff[Q_Y]=pos[4];
    diff[Q_Z]=pos[5];
    diff[Q_W]=pos[6];
    // obtenemos la matriz
    qogl_matrix_type mat;
    q_to_ogl_matrix(mat, diff);
    glMultMatrixd(mat);
  }else{
    // modo stop, simplemente añadimos los mensajes de posicion
    float* pos = track->getlastpos();
    sprintf(buffer, "X %f", pos[0]);   
    output(-7,-3.5,buffer ); 
    sprintf(buffer, "Y %f", pos[1]);   
    output(-7,-4.0,buffer ); 
    sprintf(buffer, "Z %f", pos[2]);   
    output(-7,-4.5,buffer );
  }
  
  if (old){//,pdeñp viejo
    // Un cubo
    GLfloat skinColor[4] = {0.2, 0.2, 0.2, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, skinColor);
    glutSolidCube (4.0);
    GLfloat skinColor2[4] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, skinColor2);
    glTranslatef(1,1,2);
    glutSolidCube (1.0);
    glTranslatef(-2,0,0);
    glutSolidCube (1.0);
    glTranslatef(1,0,-4);
    glutSolidCube (1.0);
  }else{ // modelo nuevo
    if ((mode==FOLLOW && lifeforms==0) || mode==STOP){
      glRotatef(20,1,0,0);
      eyes(30);
    }else
      eyes(10);
  }
  


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
    case 109: // m
	mode = (mode+1)%3;
	break;
    case 111: // o
	old=!old;
	break;
    default:
      printf("Key %i not supported\n", key);
      break;
  }
}

int main(int argc, char** argv)
{
  // marcamos flag de usar ht a falso
  useht=false;
  // y el modo a follow
  mode = FOLLOW;
  // usamos el modelo nuevo
  old=false;

  // inicializamos el aspect ratio a 1,6
  winx=960;
  winy=600;
  aspectratio=(float)winx/(float)winy;

  lifeforms=0;
  diffcount=0;
  mainuser=-1;
  newuser=-1;

  followingnewuser=false;
  thereismainuser=false;

  lastreport=-1;

  framen=0;
  sprintf(mensaje,"Keys:  esc-> exit   h->Headtrack   m->mode   o->old_model\n");


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
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();
return 0;
}
