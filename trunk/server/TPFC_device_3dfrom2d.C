#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* source):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TPFCDATA3DORI);
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  source-> report_to(this);

}

TPFC_device_3dfrom2d::~TPFC_device_3dfrom2d(){
  free(data);
}

void TPFC_device_3dfrom2d::report_from(TPFC_device* s){
  //updateamos nuestros datos con los de la fuente
  float* aux = (s->getdata())->getlastpos();
  data->setnewpos(aux[0], aux[1], 0.5);
  report();
}