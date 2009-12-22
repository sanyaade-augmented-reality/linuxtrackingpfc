#include "TPFC_device_wiimote.h" 


TPFC_device_wiimote::TPFC_device_wiimote(int ident):TPFC_device(ident){
  data = new TrackingPFC_data(TPFCDATA2DSIZE);
  // lanzamos el thread
  //pthread_create( &wiimote_thread, NULL, tpfcdevopencvfacedetect,this);
  
}

TPFC_device_wiimote::~TPFC_device_wiimote(){
  free(data);
}

void TPFC_device_wiimote::stop(){
   running= TPFC_STOP;
   pthread_join( wiimote_thread, NULL);
}
