#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* source):TPFC_device(ident){
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  source-> report_to(this);
}

TPFC_device_3dfrom2d::~TPFC_device_3dfrom2d(){
}

void TPFC_device_3dfrom2d::report_from(int id){
  //printf("Report received\n");
  //printf(".");
  report();
}