#include "TPFC_device_wiimote.h" 


// Inicializamos el vector global donde se guarda la información de los wiimotes
vector<TPFC_device_wiimote::wiimoteinfo> TPFC_device_wiimote::wiimotes;

// Función que registra los wiimotes por su id, 
void TPFC_device_wiimote::registerwiimote(cwiid_wiimote_t *wiimote, TPFC_device_wiimote* dev){
  wiimotes.push_back(wiimoteinfo(cwiid_get_id(wiimote),dev));
}
// Funcion que recupera un puntero al dev wiimote dada su id
TPFC_device_wiimote* TPFC_device_wiimote::getwiimotedev(cwiid_wiimote_t *wiimote){
  TPFC_device_wiimote* dev=NULL;
  // recorremos la lista por orden (esta operación es poco eficiente, pero
  // teniendo en cuenta que el numero de wiimotes será normalmente bajo (<4)
  // la eficiencia en la busqueda no es un problema
  for (int i = 0; i<wiimotes.size() && dev==NULL;i++){
    // comparamos los ids registrados con el actual
    if (wiimotes[i].id==cwiid_get_id(wiimote))
      // cuando encontramos la coincidencia, guardamos el puntero
      dev= wiimotes[i].dev;
  }
  // y lo devolvemos
  return dev;
}


// Callback del wiimote
void TPFC_device_wiimote::callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp){
  int i, j;
  int valid_source;
  // bucle principal del callback 
  for (i=0; i < mesg_count; i++){
    switch (mesg[i].type) {
      case CWIID_MESG_IR:
	valid_source = 0;
	for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
	  if (mesg[i].ir_mesg.src[j].valid) {
	    // aumentamos el contador de fuentes validas
	    valid_source++;
	    float* aux= new float[2];
	    // el factor de 1280 se ha inferido a partir de las observaciones en
	    // las mediciones de calibración.
	    // el wiimote tiene una camara de 1024x768
	    // con el punto 0,0 en la esquina inferior izquierda
	    aux[0]=atan( (512.0-mesg[i].ir_mesg.src[j].pos[CWIID_X]) /1280.0 );
	    aux[1]=atan( (mesg[i].ir_mesg.src[j].pos[CWIID_Y]-384.0) /1280.0 );
	    if (valid_source==1) // es el primer punto
	      getwiimotedev(wiimote)->getdata()->setnewdata(aux);
	    else // no es el primer punto, debemos usar el report existente
	      getwiimotedev(wiimote)->getdata()->setmoredata(aux);
	  }
	}
	
	if (valid_source>0) {
	  // Habia fuentes validas, al llegar a este punto los datos ya los tenemos guardados
	  // solo falta avisar a los listeners
	  getwiimotedev(wiimote)->report();
	}else{
	  // en esta toma de datos no hay datos que reportar
	  getwiimotedev(wiimote)->getdata()->setnodata();
	  getwiimotedev(wiimote)->nullreport();
	}
	break;
      default:
	printf("Unknown Report");
	break;
    }
  }
}


// Creadora del device
TPFC_device_wiimote::TPFC_device_wiimote(int ident):TPFC_device(ident){
  // creamos el bufer de datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA2D);
  // sobreescribimos el mensaje de error por defecto con el nuestro, para evitar spam
  cwiid_set_err(err);

  // conectarse al primer wiimote que se encuentre
  bdaddr_t bdaddr = *BDADDR_ANY;// bluetooth device address

  // Conectar los wiimotes
  printf("Pon el wiimote en modo discoverable (pulsa 1+2)...\n");
  while (!wiimote){
    if ( !(wiimote = cwiid_open(&bdaddr, 0)) && !(wiimote = cwiid_open(&bdaddr, 0)) ){
	    printf("Esperando al wiimote (pulsa 1+2)...\n");
    }
  }

  // registramos el wiimote para poder identificar despues los callbacks 
  registerwiimote(wiimote, this);
  
  // registramos nuestro callback con el wiimote
  if (cwiid_set_mesg_callback(wiimote, callback)) {
	  fprintf(stderr, "No se ha podido registrar el callback\n");
  }
  // activamos el paso de mensajes desde el wiimote
  if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
	  fprintf(stderr, "Error activando los mensajes\n");
  }
  // configuramos el report mode
  unsigned char rpt_mode = 0;
  toggle_bit(rpt_mode, CWIID_RPT_IR);
  set_rpt_mode(wiimote, rpt_mode);

  
  printf("Wiimote conectado!\n");
  
}

// Destructora
TPFC_device_wiimote::~TPFC_device_wiimote(){
  free(data);
}

// Funciones necesarias para el control del wiimote via cwiid
void TPFC_device_wiimote::err(cwiid_wiimote_t *wiimote, const char *s, va_list ap){
	// comento para que no salga nada de errores
}
// report mode
void TPFC_device_wiimote::set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode){
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}

// Informacion sobre el dispositivo
string TPFC_device_wiimote::info(){
  return "Wiimote";
}