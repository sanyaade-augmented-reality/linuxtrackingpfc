#include "TrackingPFC_client.h"
// calback que actualiza los datos y llama al callback personalizado si lo hay
void TrackingPFC_client::TrackingPFC_client_callback(void *userdata, const vrpn_TRACKERCB t){
   // t.sensor es la variable que da el numero de sensor
   // en este ejemplo no se usa xq se ha registrado el callback para ejecutarse solo con el sensor0

  // obtenemos el tracker desde los argumentos
  TrackingPFC_client * trk= (TrackingPFC_client*)(userdata);
  pthread_mutex_lock( trk->lock ); // liberamos el acceso
  // nos aseguramos de que el sensor tiene espacio en el vector
  while (trk->data.size()<=t.sensor){
    trk->data.push_back(new float[7]);
    struct timeval tv;
    gettimeofday(&tv, &(trk->tz));
    trk->time.push_back(tv);
  }
  // guardamos los datos
  trk->data[t.sensor][0]=t.pos[0];
  trk->data[t.sensor][1]=t.pos[1];
  trk->data[t.sensor][2]=t.pos[2];
  trk->data[t.sensor][3]=t.quat[0];
  trk->data[t.sensor][4]=t.quat[1];
  trk->data[t.sensor][5]=t.quat[2];
  trk->data[t.sensor][6]=t.quat[3];
  // actualizamos el tiempo
  gettimeofday(&(trk->time[t.sensor]), &(trk->tz));
  pthread_mutex_unlock( trk->lock ); // liberamos el acceso
  
  if (trk->callback_func!=NULL)
    trk->callback_func(trk);
}

// Creadora
TrackingPFC_client::TrackingPFC_client(const char* tname, void (cbfx)(TrackingPFC_client*)){
  //creacion de los datos
  data.push_back(new float[7]);
  // rellenamos con unos valores estandar
  data[0][0]=0;
  data[0][1]=0;
  data[0][2]=1;
  data[0][3]=0;
  data[0][4]=0;
  data[0][5]=0;
  data[0][6]=1;
  struct timeval tv;
  gettimeofday(&tv, &tz);
  time.push_back(tv);
  
  alive=1;
  tracker = new vrpn_Tracker_Remote(tname);
  //tracker->register_change_handler(this, TrackingPFC_client_callback,0);
  tracker->register_change_handler(this, TrackingPFC_client_callback);
  
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

  lock = new pthread_mutex_t(); // inicializamos el semaforo
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
float* TrackingPFC_client::getlastpos(int sensor){
  if (data.size()<=sensor){
    return NULL;
  }else{
    float * res = new float[8];
    pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
    // copiamos los datos
    for (int i =0; i<7;i++)
      res[i]=data[sensor][i];
    // en la 8a posicion está el tiempo que hace desde que se recibieron los datos
    struct timeval current;
    gettimeofday(&current, &tz);
    res[7]=diff(&current, &(time[sensor]));
    pthread_mutex_unlock( lock ); // liberamos
    return res;
  }
}


// variables estaticas para el tamaño de la pantalla:
// (se updatearan con la primera llamada a getDisplaySize
float TrackingPFC_client::screensizex=0;
float TrackingPFC_client::screensizey=0;

// Devuelven el tamaño guardado en el archivo de configuracion
float TrackingPFC_client::getDisplaySizex(){
  // Comprobamos si tenemos el valor guardado
  if (screensizex==0){
    // si no lo tenemos (es la primera vez que se llama a la funcion) lo guardamos
    screensizex=getDisplaySize(0); // lo obtenemos con la funcion auxiliar
  }
  // y lo devolvemos
  return screensizex;
}
float TrackingPFC_client::getDisplaySizey(){
  // Comprobamos si tenemos el valor guardado
  if (screensizey==0){
    // si no lo tenemos (es la primera vez que se llama a la funcion) lo guardamos
    screensizey=getDisplaySize(1); // lo obtenemos con la funcion auxiliar
  }
  // y lo devolvemos
  return screensizey;
}
// Devuelven la altura y el alto de la resolucion de la pantalla por defecto del display por defecto
// segun la informacion del servidor de las X
int TrackingPFC_client::getDisplayWidth(){
    Display * disp = XOpenDisplay(NULL);
    int res = DisplayWidth(disp,XDefaultScreen(disp));  
    XCloseDisplay(disp);
    return res;
}
int TrackingPFC_client::getDisplayHeight(){
    Display * disp = XOpenDisplay(NULL);
    int res = DisplayHeight(disp,XDefaultScreen(disp));  
    XCloseDisplay(disp);
    return res;
}
// Devuelven la altura y el alto (en metros) de la pantalla por defecto del display por defecto
// segun la informacion del servidor de las X
float TrackingPFC_client::getDisplayWidthMM(){
    Display * disp = XOpenDisplay(NULL);
    float res = (float)DisplayWidthMM(disp,XDefaultScreen(disp))/1000.0;
    XCloseDisplay(disp);
    return res;
}
float TrackingPFC_client::getDisplayHeightMM(){
    Display * disp = XOpenDisplay(NULL);
    float res = (float)DisplayHeightMM(disp,XDefaultScreen(disp))/1000.0;
    XCloseDisplay(disp);
    return res;
}


// funcion auxiliar para parsear la entrada
// (encontrada en http://www.infernodevelopment.com/perfect-c-string-explode-split)
void TrackingPFC_client::StringExplode(string str, string separator, vector<string>* results){
    int found;
    found = str.find_first_of(separator);
    while(found != string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
}
// funcion auxiliar para pasar de str a int
// (por Martin Gieseking, encontrada en http://bytes.com/topic/c/answers/132109-string-integer)
int TrackingPFC_client::str2int (const string &str) {
  stringstream ss(str);
  int n;
  ss >> n;
  return n;
}


// funcion auxiliar para calcular la diferencia (en segundos, con precision de microsecs)
double TrackingPFC_client::diff(struct timeval * x,struct timeval * y){
  double secs = x->tv_sec-y->tv_sec;
  double usecs = x->tv_usec-y->tv_usec;
  usecs=usecs/1000000.0;
  return (double)(secs+usecs);
}

// funcion auxiliar para los otros get display sizes
// xy = 0 -> queremos la horizontal (x), =1 -> la vertical (y)
// si no hay archivo de configuracion o falla la lectura, se devuelven los valores de las X
float TrackingPFC_client::getDisplaySize(int xy){
  
  string target = (xy==0)?"screensizex":"screensizey";
  float res=0;

  fstream indata;
  char filename[200];
  // formateamos el nombre del archivo
  sprintf(filename, "%s/.trackingpfc/tpfc.cfg",getenv ("HOME"));
  indata.open(filename); // abrimos
  if(!indata) { // Si no se puede abrir avisamos
      fprintf(stderr, "No se ha podido leer la configuracion en '%s', Se devuelven valores del servidor X (pueden no ser correctos).\n", filename);
  }else{ // si se puede...
    string l; // string auxiliar
    vector<string> t;
    bool notfound = true;
    while ( !indata.eof() && notfound ) { //sigue leyendo hasta el EOF
      getline(indata,l); // obtenemos una linea
      t.clear();
      StringExplode(l, " ", &t);
      if (t[0].compare(target)==0){ // hemos encontrado la opcion que buscabamos
	res = (float)str2int(t[1])/1000.0;
	notfound=false;
      }
    }
    indata.close();
    if (notfound){
      fprintf(stderr,"El archivo de configuracion %s no contiene datos sobre el tamaño de la pantalla. Se devuelven valores del servidor X (pueden no ser correctos).\n",filename);
    }
  }
  if (res>0){
    return res;
  }else if (xy==0){
     return getDisplayWidthMM();
  }else{
     return getDisplayHeightMM();
  }
    
}

// consultoras
int TrackingPFC_client::isalive(){
  return alive;
}
int TrackingPFC_client::sensors(){
  return data.size();
}


// modificadoras
void TrackingPFC_client::setdata(float * f, int sensor){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  
  // nos aseguramos de que el sensor tiene espacio en el vector
  while (data.size()<=sensor){
    data.push_back(new float[7]);
    struct timeval tv;
    gettimeofday(&tv, &tz);
    time.push_back(tv);
  }

  data[sensor][0]=f[0];
  data[sensor][1]=f[1];
  data[sensor][2]=f[2];
  data[sensor][3]=f[0];
  data[sensor][4]=f[1];
  data[sensor][5]=f[2];
  data[sensor][6]=f[3];
  gettimeofday(&(time[sensor]), &tz);
  pthread_mutex_unlock( lock ); // liberamos el acceso
}


void TrackingPFC_client::setvirtualdisplaysize(float s){
  mdl2scr = s / getDisplaySizex();
}
void TrackingPFC_client::setvirtualdisplaydistance(float d){
  mdl2scr = d/zadjustment;
}


void TrackingPFC_client::htgluPerspective(float m_dFov, float AspectRatio, float m_dCamDistMin, float m_dCamDistMax, int winx, int winy){
  // descomentar esto y comentar el resto para hacer que la función sea transparente
  //gluPerspective(m_dFov, AspectRatio, m_dCamDistMin, m_dCamDistMax);
  if (originalfov != m_dFov || aspectratio !=AspectRatio){
    aspectratio= AspectRatio;
    originalfov = m_dFov;
    float scry= getDisplaySizey();
    float radfov=m_dFov*RADFACTOR;
    zadjustment = (scry/2.0)/tan(radfov/2.0);
    //printf("Z adjustment2 %f %f %f\n", zadjustment,scry/2.0,radfov/2.0);
  }
  htadjustPerspective(m_dCamDistMin, m_dCamDistMax, winx, winy);
}

void TrackingPFC_client::htadjustPerspective(float m_dCamDistMin, float m_dCamDistMax, int winx, int winy){
  // en un principio asumiremos que estamos en full screen, por lo tanto el tamaño horizontal del display es el 100% del reportado
  float frleft, frright, frup,frdown, scrx, scry, fact;
  // tamaños del display
  // obtenemos del cliente el tamaño y la resolucion
  // con eso y el tamaño del area, obtenemos el tamaño de nuestra area
  scrx= winx*getDisplaySizex()/getDisplayWidth();
  scry= winy*getDisplaySizey()/getDisplayHeight();
  
  const float* lastpos= getlastpos();
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
    
   

  const float* lastpos= getlastpos();
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