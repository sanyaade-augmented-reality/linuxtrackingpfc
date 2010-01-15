#include "TPFC_device_3dmod.h" 


TPFC_device_3dmod::TPFC_device_3dmod(int ident, TPFC_device* s):TPFC_device(ident){

}

TPFC_device_3dmod::~TPFC_device_3dmod(){
  free(data);
}

// sobrecarga del handler de los reports
void TPFC_device_3dmod::report_from(TPFC_device* s){
}


// Informacion sobre el dispositivo
string TPFC_device_3dmod::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dmod (usando de fuente el dispositiv %i)", source->idnum());
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_3dmod::checksource(TPFC_device* s){
  string ret = "ok";
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3D && s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3DORI)
    ret ="El tipo de datos de la fuente no es el adecuado: debe ser un dispositivo 3d (tenga o no orientacion).\n";
  return ret;
}