#include "TPFC_device_wiimote.h" 


// Inicializamos el vector global donde se guarda la información de los wiimotes
vector<TPFC_device_wiimote::wiimoteinfo> TPFC_device_wiimote::wiimotes;

void TPFC_device_wiimote::registerwiimote(cwiid_wiimote_t *wiimote, TPFC_device_wiimote* dev){
  wiimotes.push_back(wiimoteinfo(cwiid_get_id(wiimote),dev));
}
TPFC_device_wiimote* TPFC_device_wiimote::getwiimotedev(cwiid_wiimote_t *wiimote){
  TPFC_device_wiimote* dev=NULL;
  for (int i = 0; i<wiimotes.size() && dev==NULL;i++){
    if (wiimotes[i].id==cwiid_get_id(wiimote))
      dev= wiimotes[i].dev;
  }
  return dev;
}


// Callback del wiimote
void TPFC_device_wiimote::callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp){
	int i, j;
	int valid_source;
	int xacum, yacum;
	float x,y;
	for (i=0; i < mesg_count; i++){
		switch (mesg[i].type) {
		  case CWIID_MESG_IR:
			  valid_source = 0;
			  xacum=0;
			  yacum=0;
			  for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				  if (mesg[i].ir_mesg.src[j].valid) {
					  valid_source++;
					  xacum+=mesg[i].ir_mesg.src[j].pos[CWIID_X];
					  yacum+=mesg[i].ir_mesg.src[j].pos[CWIID_Y];
				  }
			  }
			  //printf("Callback de %i\n", cwiid_get_id(wiimote));
			  
			  if (valid_source>0) {
			    // el wiimote es de ~ 1024x768
			    x=512.0-(float(xacum)/float(valid_source));
			    y=(float(yacum)/float(valid_source))-384.0;
			    x=x/512.0;
			    y=y/384.0;
			    x=x/2.0;
			    y=y/2.0;
			    float* aux= new float[2];
			    aux[0]=x;
			    aux[1]=y;
			    getwiimotedev(wiimote)->getdata()->setnewdata(aux);
			    getwiimotedev(wiimote)->report();
			    //printf("%f, %f\n",x,y);
			    /*position[0]=x;
			    position[1]=y;
			    position[2]=0.5;
			    vrpn_gettimeofday(&current_time, NULL);
			    nt->report_pose(0,current_time, position, quaternion);
			    nt->mainloop();
			    connection->mainloop();*/
			  }
			  break;
		  default:
			  printf("Unknown Report");
			  break;
		}
	}
}




TPFC_device_wiimote::TPFC_device_wiimote(int ident):TPFC_device(ident){
  
  data = new TrackingPFC_data(TPFCDATA2D);
  
  cwiid_set_err(err);

  // conectarse al primer wiimote que se encuentre
  bdaddr_t bdaddr = *BDADDR_ANY;// bluetooth device address

  // Connect to the wiimote s
  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
  while (!wiimote){
    if ( !(wiimote = cwiid_open(&bdaddr, 0)) && !(wiimote = cwiid_open(&bdaddr, 0)) ){
	    printf("waiting for a wiimote (press 1+2)...\n");
    }
  }
  // registramos el wiimote para poder identificar despues los callbacks 
  registerwiimote(wiimote, this);
  
  if (cwiid_set_mesg_callback(wiimote, callback)) {
	  fprintf(stderr, "Unable to set message callback\n");
  }
  if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
	  fprintf(stderr, "Error enabling messages\n");
  }
  unsigned char rpt_mode = 0;
  toggle_bit(rpt_mode, CWIID_RPT_IR);
  set_rpt_mode(wiimote, rpt_mode);
  printf("Wiimote connected!\n");
  
  /*for (int i = 0; i<totalwiimotes;i++){
    printf("Wiimote %i: id (%i), id del device (%i)\n", i, wiimoteids[i], wiimotedevices[i]->idnum());
  }*/
  
}

TPFC_device_wiimote::~TPFC_device_wiimote(){
  free(data);
}


// Funciones necesarias para el control del wiimote via cwiid
void TPFC_device_wiimote::err(cwiid_wiimote_t *wiimote, const char *s, va_list ap){
	// comento para que no salga nada de errores
}
void TPFC_device_wiimote::set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode){
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}