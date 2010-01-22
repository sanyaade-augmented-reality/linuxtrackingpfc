#include "TPFC_device_artoolkit.h" 


TPFC_device_artoolkit::TPFC_device_artoolkit(int ident):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);
}

TPFC_device_artoolkit::~TPFC_device_artoolkit(){
  free(data);
}

// Informacion sobre el dispositivo
string TPFC_device_artoolkit::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "Artoolkit");
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_artoolkit::checksource(TPFC_device* s){
  string ret = "Este dispositivo no acepta fuentes.";
  return ret;
}