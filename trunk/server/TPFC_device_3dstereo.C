#include "TPFC_device_3dstereo.h" 


TPFC_device_3dstereo::TPFC_device_3dstereo(int ident, TPFC_device* s1, TPFC_device* s2):TPFC_device(ident){

  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D);

  // guardamos un puntero a las fuentes
  sources= new TPFC_device*[2];
  sources[0]= s1;
  sources[1]= s2;
  // y creamos un vector donde guardar los reports
  lastdatas = new TrackingPFC_data::datachunk*[2];

  // marcamos a falso el flag de calibrado
  calibrated = false;
  // este dispositivo estara en pausa (ignorando los reports) hasta que no empiece la calibración
  running=PAUSE;

  // registramos este dispositivo en la lista de listeners que vamos a usar como input
  s1-> report_to(this);
  s2-> report_to(this);

  // calibramos
  calibrate();
}

TPFC_device_3dstereo::~TPFC_device_3dstereo(){
  free(data);
  if (lastdatas[0]!=NULL);
    free(lastdatas[0]);
  if (lastdatas[1]!=NULL);
    free(lastdatas[1]);
}

void TPFC_device_3dstereo::calibrate(){
  calib_samples=500;
  calib_dots=1;
  int progressinc=calib_samples/50;// cada cuantos samples actualizar estado

  calib_lock = new pthread_mutex_t(); // inicializamos el semaforo
  // creamos el buffer de datos
  calib_data = (float*)malloc(calib_samples*calib_dots*4*sizeof(float));
  // inicializamos el indice
  calib_count=0;
  printf("Pulsa enter para calibrar\n");
  // activamos el flag de running para que report_from empiece a capturar datos
  char trash= getchar();
  running=RUN;
  int progress=progressinc;
  while (calib_count < calib_samples){
    vrpn_SleepMsecs(1);
    if (calib_count>=progress){
      progress+=progressinc;
      printf(".");
      fflush(stdout);
    }
  }
  printf("\nyay!\n");

  // calibrar

  calibrated = true;
  free(calib_data);
}

void TPFC_device_3dstereo::adddot(float x1, float y1, float x2, float y2, int dot){
  pthread_mutex_lock( calib_lock ); // obtenemos acceso exclusivo
  // comprobamos que no este lleno el buffer
  if (calib_count<calib_samples){
    int desp=calib_count*calib_dots*4 + dot*4;
    calib_data[desp]=x1;
    calib_data[desp+1]=y1;
    calib_data[desp+2]=x2;
    calib_data[desp+3]=y2;
    // si es el ultimo punto del report, aumentamos el indice
    if ((dot+1)==calib_dots)
      calib_count++;
  }
  pthread_mutex_unlock( calib_lock ); // liberamos el acceso exclusivo
}

void TPFC_device_3dstereo::report_from(TPFC_device* s){
  // comprobamos que no estemos en pausa
  if (working()){
    //free(sourcedata);
    TrackingPFC_data::datachunk* sourcedata= (s->getdata())->getlastdata();
    // comprobamos si los datos son validos

    if (calibrated){ // si ya esta el dispositivo calibrado


/*// obtenemos los datos
if (sourcedata->getvalid() == false){
  // si no son validos guardamos un chunk no valido en nuestros datos y damos un nullreport
  data->setnodata();
  nullreport();
}else{// si son validos...
  // obtenemos el numero de puntos del report
  int n = sourcedata->size();
  const float* aux;

  if (merge){ // Hay que unificar todos los datos
    // acumuladores
    float acumx=0;
    float acumy=0;
    // recorremos los puntos sumando los datos
    for (int i =0; i<n; i++){
      aux = sourcedata->getdata(i);
      acumx+=aux[0];
      acumy+=aux[1];
    }
    // obtenemos la media
    acumx=acumx/n;
    acumy=acumy/n;
    // guardamos los datos
    setdata(acumx, acumy);
  }else{ // Los datos se envian como sensores separados
    // obtenemos el primer punto
    aux = sourcedata->getdata();
    // y lo guardamos en un nuevo report
    setdata(aux[0], aux[1]);
    // recorremos los siguientes puntos (si los hay)
    for (int i =1; i<n; i++){
      // obtenemos y guardamos los datos de los puntos, pero añadiendolos al report existente
      aux = sourcedata->getdata(i);
      setdata(aux[0], aux[1], false);
    }
  }
  // sea cual sea la opción, una vez escritos los datos, reportamos
  report();
}*/




    }else{ // si no esta calibrado (lo estamos haciendo)
      adddot(0.0,0.0,0.0,0.0,1);
    }// if calibrated
  }// if workikg
}

void TPFC_device_3dstereo::nullreport_from(TPFC_device* s){
  report_from(s);
}

// Informacion sobre el dispositivo
string TPFC_device_3dstereo::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dstereo (usando de fuente los dispositivo %i y %i)", sources[0]->idnum(), sources[1]->idnum());
  return aux;
}