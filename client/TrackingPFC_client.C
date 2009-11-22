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
  obsz=1;// para que si no hay tracker podemos ver algo, asumimos que no tenemos pegada la nariz a la pantalla

  alive=1;
  tracker = new vrpn_Tracker_Remote(tname);
  tracker->register_change_handler(this, TrackingPFC_client_callback,0);
  
  //cbfx(NULL);
  callback_func= cbfx;
  pthread_create( &mainloop_thread, NULL, mainloop_executer,this);
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

void TrackingPFC_client::htgluLookAt(float eyex, float eyey, float eyez,
				   float tarx, float tary, float tarz,
				   float upx, float upy, float upz){
  // vamos a asumir que el centro del bounding box es tar
  float vecx,vecy,vecz,neweyex,neweyey,neweyez;
  // vector hacia el que está mirando la camara
  vecx=tarx-eyex;
  vecy=tary-eyey;
  vecz=tarz-eyez;

  // posicion modificada del ojo
  neweyex=eyex+(obsx*250);
  neweyey=eyey+(obsy*250);
  neweyez=eyez+(obsz*250);
  //printf("DEBUG: %f, %f, %f    %f, %f, %f\n",neweyex,neweyey,neweyez, neweyex+vecx,neweyey+vecy,neweyez+vecz);
  gluLookAt(neweyex,neweyey,neweyez, neweyex+vecx,neweyey+vecy,neweyez+vecz,  upx,upy,upz);
}

void TrackingPFC_client::htgluPerspective(float m_dFov, float AspectRatio, float m_dCamDistMin, float m_dCamDistMax){
  gluPerspective(m_dFov, AspectRatio, m_dCamDistMin, m_dCamDistMax);
}