#include "TPFC_device.h" 

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

void TPFC_device::report_to(TPFC_device* l){
  listeners.push_back( l );
}

void TPFC_device::report(){
  for (int i =0; i<listeners.size();i++){
    listeners[i]-> report_from(this);
  }

  if (server!=NULL){
    struct timeval current_time;
    vrpn_float64 position[3];
    vrpn_float64 quaternion[4];
    float* aux = data->getlastpos();
    position[0]=aux[0];
    position[1]=aux[1];
    position[2]=aux[2];
    quaternion[0]=aux[3];
    quaternion[1]=aux[4];
    quaternion[2]=aux[5];
    quaternion[3]=aux[6];
    vrpn_gettimeofday(&current_time, NULL);
    server->report_pose(0,current_time, position, quaternion);
    server->mainloop();
    //connection->mainloop();
  }
}

int TPFC_device::settracker(vrpn_Connection * con, const char* name){
  if (server!=NULL)
    return -1;
  if ((server = new vrpn_Tracker_Server(name, con)) == NULL){
    fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
    return -1;
  }
 
 
  //connection= con;
  return 0;
}


void TPFC_device::pause(){
  running= PAUSE;
}
void TPFC_device::unpause(){
  running= RUN;
}
void TPFC_device::stop(){
  running= STOP;
}

int TPFC_device::alive(){
  return running!=STOP;
}
int TPFC_device::working(){
  return running==RUN;
}

TrackingPFC_data * TPFC_device::getdata(){
  return data;
}
int TPFC_device::idnum(){
  return id;
}

// placeholders para funciones virtualizadas
void TPFC_device::report_from(TPFC_device*){
}

