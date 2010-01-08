#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* source):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI);
  // La opción de merge esta activada por defecto
  merge = true;
  // La opción por defecto es usar una profundidad fija
  deep=FIJA;
  dist = 1.0;
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
  }
}

// función auxiliar que añade los datos segun el tipo de deep
// si new==true se usara setdata, si ==false, se usara setmoredata (no se empezara report nuevo)
void TPFC_device_3dfrom2d::setdata(float x, float y, bool newrep){
  float* aux= new float[3];

  // Tipo de calculo de profundidad
  if (deep==FIJA){
    aux[0]=tan(x);
    aux[1]=tan(y);
    aux[2]=dist;
  }
  
  // comprobamos si hay que empezar report nuevo o añadir al ya existente
  if (newrep)
    data->setnewdata(aux);
  else
    data->setmoredata(aux);
}

// Opciones
void TPFC_device_3dfrom2d::setmerge(bool m){
  merge = m;
}
void TPFC_device_3dfrom2d::setdeep(deeptype d, float di){
  deep=d;
  dist=di;
}