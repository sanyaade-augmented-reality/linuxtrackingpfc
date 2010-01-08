#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* source):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TPFCDATA3DORI);
  // La opción de merge esta activada por defecto
  merge = false;
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  source-> report_to(this);

}

TPFC_device_3dfrom2d::~TPFC_device_3dfrom2d(){
  free(data);
}

void TPFC_device_3dfrom2d::report_from(TPFC_device* s){
  
  // obtenemos los datos
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
      data->setnewpos(acumx, acumy, 0.5);
    }else{ // Los datos se envian como sensores separados
      // obtenemos el primer punto
      aux = sourcedata->getdata();
      // y lo guardamos en un nuevo report
      data->setnewpos(aux[0], aux[1], 0.5);
      float* aux2 = new float[3];
      aux2[2]=0.5;
      // recorremos los siguientes puntos (si los hay)
      for (int i =1; i<n; i++){
	// obtenemos y guardamos los datos de los puntos, pero añadiendolos al report existente
	aux = sourcedata->getdata(i);
	aux2[0]=aux[0];
	aux2[1]=aux[1];
	data->setmoredata(aux2);
      }
    }
    // sea cual sea la opción, una vez escritos los datos, reportamos
    report();
  }
}