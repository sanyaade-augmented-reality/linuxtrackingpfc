#include "TrackingPFC_client.h"
// calback que actualiza los datos y llama al callback personalizado si lo hay
void TrackingPFC_client::TrackingPFC_client_callback(void *userdata, const vrpn_TRACKERCB t){
   // t.sensor es la variable que da el numero de sensor
   // en este ejemplo no se usa xq se ha registrado el callback para ejecutarse solo con el sensor0
  TrackingPFC_client * trk= (TrackingPFC_client*)(userdata);
  trk->setnewpos(t.pos[0],t.pos[1],t.pos[2]);
  if (trk->callback_func!=NULL)
    trk->callback_func(trk);
}

// Creadora
TrackingPFC_client::TrackingPFC_client(const char* tname, void (cbfx)(TrackingPFC_client*)){
  data = new TrackingPFC_data();
  
  alive=1;
  tracker = new vrpn_Tracker_Remote(tname);
  tracker->register_change_handler(this, TrackingPFC_client_callback,0);
  
  //cbfx(NULL);
  callback_func= cbfx;
  pthread_create( &mainloop_thread, NULL, mainloop_executer,this);

  // virtual display size a 0 (no se usa)
  mdl2scr=0;
  // fov original a 0 (no se usa)
  originalfov=0;
  // distancia hasta el display para tener un fov = a originalfow
  zadjustment=0; // inicialmente a 0, para que no influya en casos que no requieren ese ajuste (en los que no hay fov original)
  aspectratio=0; // inicialmente a 0
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
void* TrackingPFC_client::mainloop_executer(void * t){
  while ( ((TrackingPFC_client *)t)->isalive() ==1 ){// esto deberia poder acabar!
    ((TrackingPFC_client *)t)->mainloop();
    vrpn_SleepMsecs(1);
  }
}

// consultoras
float* TrackingPFC_client::getlastpos(){
  return data->getlastpos();
}
float TrackingPFC_client::getDisplaySizex(){
  return 0.52; // placeholder!!!
}
// consultoras
int TrackingPFC_client::isalive(){
  return alive;
}


// modificadoras
void TrackingPFC_client::setnewpos(float x, float y, float z){
  data->setnewpos(x,y,z);
}

void TrackingPFC_client::setvirtualdisplaysize(float s){
  mdl2scr = s / getDisplaySizex();
}
void TrackingPFC_client::setvirtualdisplaydistance(float d){
  mdl2scr = d/zadjustment;
}


void TrackingPFC_client::htgluPerspective(float m_dFov, float AspectRatio, float m_dCamDistMin, float m_dCamDistMax){
  // descomentar esto y comentar el resto para hacer que la función sea transparente
  //gluPerspective(m_dFov, AspectRatio, m_dCamDistMin, m_dCamDistMax);
  if (originalfov != m_dFov || aspectratio !=AspectRatio){
    aspectratio= AspectRatio;
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

  float* lastpos= data->getlastpos();
  float obsx, obsy, obsz;
  obsx=lastpos[0];
  obsy=lastpos[1];
  obsz=lastpos[2];
  // obtenemos el factor znear_display/mundo real
  fact=m_dCamDistMin/obsz;
  frleft= fact*((-scrx/2.0)-obsx);
  frright= fact*((scrx/2.0)-obsx);
  frup=fact*((-scry/2.0)-obsy);
  frdown=fact*((scry/2.0)-obsy);
  
  // calculamos si tenemos que ampliar zfar (por estar moviendo la camara hacia atras
  float adj=0.0;
  if (zadjustment > 0.0 && obsz>zadjustment){
    // comprobamos que se haya ajustado la escala
    if (mdl2scr==0) printf("Warning, TrackingPFC_client::htadjustPerspective is being called without setting first the virtual display size or distance.\n");
    adj= (obsz -zadjustment)*mdl2scr;
  }

  glFrustum (frleft, frright, frup, frdown, m_dCamDistMin, m_dCamDistMax+adj);
}

void TrackingPFC_client::htgluLookAt(float eyex, float eyey, float eyez,
				   float tarx, float tary, float tarz,
				   float upx, float upy, float upz){
  // descomentar esto y comentar el resto para hacer que la funcion sea transparente
  //gluLookAt(eyex,eyey, eyez,  tarx,tary, tarz,   upx, upy, upz);
    
   

  float* lastpos= data->getlastpos();
  float obsx, obsy, obsz;
  obsx=lastpos[0];
  obsy=lastpos[1];
  obsz=lastpos[2];

  // corregimos el obsz (por si habia un fov original)
  float cobsz= obsz -zadjustment;

  // comprobamos que se haya ajustado la escala
  if (mdl2scr==0) printf("Warning, TrackingPFC_client::htadjustPerspective is being called without setting first the virtual display size or distance.\n");

  // En vez de reajustar la camara, movemos todo el modelo en direccion contraria
  glTranslatef(-obsx*mdl2scr,-obsy*mdl2scr,-cobsz*mdl2scr);
  
  gluLookAt(eyex,eyey, eyez,  tarx,tary, tarz,   upx, upy, upz);
}