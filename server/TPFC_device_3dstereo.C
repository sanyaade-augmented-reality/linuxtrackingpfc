#include "TPFC_device_3dstereo.h" 


TPFC_device_3dstereo::TPFC_device_3dstereo(int ident, TPFC_device* s1, TPFC_device* s2):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D);
  // guardamos un puntero a la fuente
  sources= new TPFC_device*[2];
  sources[0]= s1;
  sources[1]= s2;
  // registramos este dispositivo en la lista de listeners que vamos a usar como input
  s1-> report_to(this);
  s2-> report_to(this);
  

}

TPFC_device_3dstereo::~TPFC_device_3dstereo(){
  free(data);
}

void TPFC_device_3dstereo::report_from(TPFC_device* s){
  
  /*// obtenemos los datos
  TrackingPFC_data::datachunk* sourcedata= (s->getdata())->getlastdata();
  // comprobamos si los datos son validos
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
}

// Informacion sobre el dispositivo
string TPFC_device_3dstereo::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dstereo (usando de fuente los dispositivo %i y %i)", sources[0]->idnum(), sources[1]->idnum());
  return aux;
}