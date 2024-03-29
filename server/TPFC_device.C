#include "TPFC_device.h" 

// Creadora y destructora
TPFC_device::TPFC_device(int ident){
  // guardamos la ID del dispositivo
  id = ident;
  // iniciamos el servidor a NULL (si es necesario, se ajusta con una llamada a settracker posteriormente
  server=NULL;
  // ajustamos el flag de funcionamiento
  running=RUN;
}
TPFC_device::~TPFC_device(){
}

// registrar un nuevo listener
void TPFC_device::report_to(TPFC_device* l){
  // simplemente añadimos el listener a la lista.
  // no hay control de repetidos, como son los propios dispositivos los que se registran como listeners
  // en el momento de su creación, asumimos que ninguno intentará registrarse 2 veces
  listeners.push_back( l );
}

// Reportar a los listeners que hay nuevos datos (enviando solo el aviso, no los datos en si)
// y enviar los datos via vrpn_tracker si es que existe uno (en esta caso si que se envian los datos)
void TPFC_device::report(bool onlytagged){
  // recorremos la lista de listeners enviando el aviso
  for (int i =0; i<listeners.size();i++){
    listeners[i]-> report_from(this);
  }
  // si este dispositivo tiene un tracker asociado, enviamos un nuevo paquete
  if (server!=NULL){
    struct timeval current_time;
    vrpn_float64 position[3];
    vrpn_float64 quaternion[4];

    TrackingPFC_data::datachunk* d = data->getlastdata();
    vrpn_gettimeofday(&current_time, NULL);
    const double* aux;
    for (int i =0; i<d->size() && i<sensors;i++){
      // si estamos en modo solo taggeds y no tiene tag, no reportamos
      if (!onlytagged || (onlytagged && d->gettag(i)!=0) ){
	aux= d->getdata(i);
	position[0]=aux[0];
	position[1]=aux[1];
	// si el tipo de datos no tiene los 7 campos requeridos, rellenamos con un quaternion nulo
	if (data->datasize()>=3)
	  position[2]=aux[2];
	else
	  position[2]=0.0;
	if (data->datasize()>=4){
	  quaternion[0]=aux[3];
	  quaternion[1]=aux[4];
	  quaternion[2]=aux[5];
	  quaternion[3]=aux[6];
	}else{
	  quaternion[0]=0.0;
	  quaternion[1]=0.0;
	  quaternion[2]=0.0;
	  quaternion[3]=1.0;
	}
	server->report_pose(i,current_time, position, quaternion);
      }
    }
    server->mainloop();
    // si habia mas puntos de los que podemos reportar por el numero fijado en sensors, avisamos
    if (d->size()>sensors)
      fprintf(stderr,"El dispositivo %i tiene un numero maximo de sensores de %i en el tracker, pero habia %i puntos que reportar\n", id, sensors, d->size());
  }
}

// Reportar a los listeners que no hay nuevos datos
void TPFC_device::nullreport(){
  // recorremos la lista de listeners enviando el aviso
  for (int i =0; i<listeners.size();i++){
    listeners[i]-> nullreport_from(this);
  }
}

// registrar un tracker asociado a este dispositivo
int TPFC_device::settracker(vrpn_Connection * con, const char* name, int nsensors){
  // si ya habia un tracker asociado, devolvemos -1, aunque esto no deberia pasar
  if (server!=NULL)
    return -1;
  // si hay algun problema al crear el tracker, enviamos un -2
  if ((server = new vrpn_Tracker_Server(name, con, nsensors)) == NULL){
    fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
    return -2;
  }
  // guardamos el numero de sensores
  sensors=nsensors;
  //si todo es correcto devolvemos un 0;
  return 0;
}

// Funciones para cambiar el estado del dispositivo
void TPFC_device::pause(){
  running= PAUSE;
}
void TPFC_device::unpause(){
  running= RUN;
}
void TPFC_device::stop(){
  running= STOP;
}

// Consultoras sobre el estado
bool TPFC_device::alive(){
  return running!=STOP;
}
bool TPFC_device::working(){
  return running==RUN;
}

// Consultora: obtiene un puntero a los datos del device
TrackingPFC_data * TPFC_device::getdata(){
  return data;
}
// consultora de la id del dispositivo
int TPFC_device::idnum(){
  return id;
}

// placeholders para el handler de los reports. Esta función debe ser sobrecargada si el dispositivo va a usarla.
void TPFC_device::report_from(TPFC_device*){
}
void TPFC_device::nullreport_from(TPFC_device*){
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device::checksource(TPFC_device*){
  string ret = "Este dispositivo no acepta fuentes.";
  return ret;
}




// funcion auxiliar para leer el archivo de configuracion
bool TPFC_device::getconfig(string target, string* res){

  // accedemos al archivo de configuracion
  fstream indata;
  char filename[200];

  // formateamos el nombre del archivo
  sprintf(filename, "%s/.trackingpfc/tpfc.cfg",getenv ("HOME"));

  indata.open(filename); // abrimos
  
  bool found = false;// flag de datos encontrados
  if(indata){ // si se ha podido acceder...

    string l; // string auxiliar
    vector<string> t;
    
    while ( !indata.eof() && !found ) { //sigue leyendo hasta el EOF
      getline(indata,l); // obtenemos una linea
      t.clear();
      StringExplode(l, " ", &t);
      if (t[0].compare(target)==0){ // hemos encontrado la opcion que buscabamos
	*res=t[1];
	found=true;
      }
    }
    indata.close();
  }
  return found;
}

// funcion auxiliar para parsear la entrada
// (encontrada en http://www.infernodevelopment.com/perfect-c-string-explode-split)
void TPFC_device::StringExplode(string str, string separator, vector<string>* results){
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


// funcion auxiliar para comprobar si un archivo existe
// (encontrada en http://www.techbytes.ca/techbyte103.html)
bool TPFC_device::FileExists(string strFilename) {
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }
  
  return(blnReturn);
}


