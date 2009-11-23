#include "TrackingPFC_client.h"
// calback que actualiza los datos y llama al callback personalizado si lo hay
void TrackingPFC_client_callback(void *userdata, const vrpn_TRACKERCB t){
   // t.sensor es la variable que da el numero de sensor
   // en este ejemplo no se usa xq se ha registrado el callback para ejecutarse solo con el sensor0
  TrackingPFC_client * trk= (TrackingPFC_client*)(userdata);
  trk->obsx = t.pos[0];
  trk->obsy = t.pos[1];
  trk->obsz = t.pos[2];
  if (trk->callback_func!=NULL)
    trk->callback_func(trk);
  //printf("%f\n",trk->obsx); // descomentar esto para ver si conecta
}

// Creadora
TrackingPFC_client::TrackingPFC_client(const char* tname, void (cbfx)(TrackingPFC_client*)){
  obsx=0;
  obsy=0;
  obsz=10.5;// para que si no hay tracker podemos ver algo, asumimos que no tenemos pegada la nariz a la pantalla
  

  alive=1;
  tracker = new vrpn_Tracker_Remote(tname);
  tracker->register_change_handler(this, TrackingPFC_client_callback,0);
  
  //cbfx(NULL);
  callback_func= cbfx;
  pthread_create( &mainloop_thread, NULL, mainloop_executer,this);

  // virtual display size a 0 (no se usa)
  virtualdisplaysize=0;
  // fov original a 0 (no se usa)
  originalfov=0;
  // distancia hasta el display para tener un fov = a originalfow
  zadjustment=0; // inicialmente a 0, para que no influya en casos que no requieren ese ajuste (en los que no hay fov original)
  // virtual display en modo default
  coordmode=TPFCCORD_DEFAULT;
}

// Destructora
TrackingPFC_client::~TrackingPFC_client(){
  alive=0;
  pthread_join( mainloop_thread, NULL); //?
}

// Forzar un mainloop para evitar problemas de latencia
// Esta funcion en realidad no se deberia usar casi nunca
void TrackingPFC_client::mainloop(){
  tracker->mainloop();
}

// Codigo que ejecuta el thread encargado del mainloop
void *mainloop_executer(void * t){
  while ( ((TrackingPFC_client *)t)->alive ==1 ){// esto deberia poder acabar!
    ((TrackingPFC_client *)t)->tracker->mainloop();
    vrpn_SleepMsecs(1);
  }
}

// consultoras
float TrackingPFC_client::getlastposx(){
  return obsx;
}
float TrackingPFC_client::getlastposy(){
  return obsy;
}
float TrackingPFC_client::getlastposz(){
  return obsz;
}
float TrackingPFC_client::getDisplaySizex(){
  return 0.52; // placeholder!!!
}


// modificadoras
void TrackingPFC_client::setlastposx(float x){
  obsx=x;
}
void TrackingPFC_client::setlastposy(float y){
  obsy=y;
}
void TrackingPFC_client::setlastposz(float z){
  obsz=z;
}

void TrackingPFC_client::setvirtualdisplaysize(float s){
  virtualdisplaysize=s;
}


void TrackingPFC_client::htgluPerspective(float m_dFov, float AspectRatio, float m_dCamDistMin, float m_dCamDistMax){
  // descomentar esto y comentar el resto para hacer que la función sea transparente
  //gluPerspective(m_dFov, AspectRatio, m_dCamDistMin, m_dCamDistMax);
  if (originalfov != m_dFov){
    originalfov = m_dFov;
    float scry= getDisplaySizex()/AspectRatio;
    float radfov=m_dFov*RADFACTOR;
    zadjustment = (scry/2.0)/tan(radfov/2.0);
    //printf("Z adjustment2 %f %f %f\n", zadjustment,scry/2.0,radfov/2.0);
  }
  htadjustPerspective(AspectRatio, m_dCamDistMin, m_dCamDistMax);
}

void TrackingPFC_client::htadjustPerspective(float AspectRatio, float m_dCamDistMin, float m_dCamDistMax){
  // en un principio asumiremos que estamos en full screen, por lo tanto el tamaño horizontal del display es el 100% del reportado
  float frleft, frright, frup,frdown, scrx, scry, fact;
  // tamaños del display
  scrx= getDisplaySizex();
  scry= scrx/AspectRatio;

  // obtenemos el factor znear_display/mundo real
  fact=m_dCamDistMin/obsz;
  frleft= fact*((-scrx/2.0)-obsx);
  frright= fact*((scrx/2.0)-obsx);
  frup=fact*((-scry/2.0)-obsy);
  frdown=fact*((scry/2.0)-obsy);
  
  // calculamos si tenemos que ampliar zfar (por estar moviendo la camara hacia atras
  float adj=0.0;
  if (obsz>zadjustment){
    float mdl2scr = virtualdisplaysize / getDisplaySizex();
    adj= (obsz -zadjustment)*mdl2scr;
    printf("zfar amplied %f units\n",adj);
  }

  glFrustum (frleft, frright, frup, frdown, m_dCamDistMin, m_dCamDistMax+adj);
}

void TrackingPFC_client::htgluLookAt(float eyex, float eyey, float eyez,
				   float tarx, float tary, float tarz,
				   float upx, float upy, float upz){
  // descomentar esto y comentar el resto para hacer que la funcion sea transparente
  //gluLookAt(eyex,eyey, eyez,  tarx,tary, tarz,   upx, upy, upz);
  float vecx,vecy,vecz,neweyex,neweyey,neweyez, mdl2scr;
  // vector hacia el que está mirando la camara
  vecx=tarx-eyex;
  vecy=tary-eyey;
  vecz=tarz-eyez;
  // calculamos el ratio modelo/realidad
  if (virtualdisplaysize==0){
    printf("Warning, TrackingPFC_client::htgluLookAt is being called without setting first the virtual display size or distance. Aborting program now.\n");
    exit(-1);
  }
  mdl2scr = virtualdisplaysize / getDisplaySizex();
  
  // corregimos el obsz (por si habia un fov original)
  float cobsz= obsz -zadjustment;

  // posicion modificada del ojo
  if (coordmode==TPFCCORD_DEFAULT){
    neweyex=eyex+(obsx*mdl2scr); // horizontal, derecha=positivo
    neweyey=eyey+(obsy*mdl2scr); // vertical , arriba = positivo
    neweyez=eyez+(cobsz*mdl2scr); // profundidad, alejarse de la pantalla = positivo
  }else if (coordmode==TPFCCORD_GLC){
    neweyex=eyex-(obsx*mdl2scr); // horizontal, izda=positivo
    neweyey=eyey+(cobsz*mdl2scr); // profundidad, alejarse de la pantalla = positivo
    neweyez=eyez+(obsy*mdl2scr); // vertical, arriba = positivo
  }else{
    printf("Warning, TrackingPFC_client::htgluLookAt is being called withan unknown coordinate system. Aborting program now.\n");
    exit(-1);
  }

  //printf("DEBUG: %f, %f, %f    %f, %f, %f\n",neweyex,neweyey,neweyez, neweyex+vecx,neweyey+vecy,neweyez+vecz);
  gluLookAt(neweyex,neweyey,neweyez, neweyex+vecx,neweyey+vecy,neweyez+vecz,  upx,upy,upz);
}