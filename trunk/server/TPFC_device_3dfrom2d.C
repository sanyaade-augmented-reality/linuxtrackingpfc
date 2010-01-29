#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* s):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D);
  // La opción de merge esta activada por defecto solo si la fuente es 2d sin tamaño
  merge = s->getdata()->datatype()==TrackingPFC_data::TPFCDATA2D;
  // La opción por defecto es usar una profundidad fija
  deep=FIJA;
  dist = 1.0;
  lastvaliddist=1.0;
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  s-> report_to(this);
  // guardamos un puntero a la fuente
  source=s;

}

TPFC_device_3dfrom2d::~TPFC_device_3dfrom2d(){
  free(data);
}

void TPFC_device_3dfrom2d::report_from(TPFC_device* s){
  // comprobamos que no estemos en pausa
  if (working()){
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
      const double* aux;
      bool validreport = true;

      if (merge){ // Hay que unificar todos los datos
	// acumuladores
	double acumx=0;
	double acumy=0;
	// recorremos los puntos sumando los datos
	for (int i =0; i<n; i++){
	  aux = sourcedata->getdata(i);
	  acumx+=aux[0];
	  acumy+=aux[1];
	}
	// obtenemos la media
	acumx=acumx/n;
	acumy=acumy/n;
	if (deep==FIJA || deep==ROTACION){
	  // si la profundidad no se ha de calcular por tamaño, simplemente guardamos los datos
	  setdata(acumx, acumy);
	}else if (deep==APROXSIZE || (deep==ONLYSIZE && n>1)) {
	  // si se ha de calcular por tamaño, debemos recorrer de nuevo los datos obteniendo la distancia
	  // media del punto central, que es lo que devolveremos como tamaño
	  double acumangle=0;
	  double diff;
	  for (int i =0; i<n; i++){
	    aux = sourcedata->getdata(i);
	    diff= (acumx-aux[0])*(acumx-aux[0]) + (acumy-aux[1])*(acumy-aux[1]);
	    acumangle+=sqrt(diff);
	  }
	  // obtenemos la media
	  acumangle=acumangle/n;
	  // y guardamos
	  setdata(acumx, acumy, true, acumangle);
	}else{// onlysize y solo habia un punto, por lo que no hay size
	  validreport=false; // marcamos el flag a falso
	  data->setnodata(); // guardamos datos vacios
	}
      }else{ // Los datos se envian como sensores separados
	// obtenemos el primer punto
	aux = sourcedata->getdata();
	// y lo guardamos en un nuevo report
	if (source->getdata()->datatype()==TrackingPFC_data::TPFCDATA2D)
	  setdata(aux[0], aux[1]);
	else
	  setdata(aux[0], aux[1], true, aux[2]);
	// recorremos los siguientes puntos (si los hay)
	for (int i =1; i<n; i++){
	  // obtenemos y guardamos los datos de los puntos, pero añadiendolos al report existente
	  aux = sourcedata->getdata(i);
	  if (source->getdata()->datatype()==TrackingPFC_data::TPFCDATA2D)
	    setdata(aux[0], aux[1], false);
	  else
	    setdata(aux[0], aux[1], false, aux[2]);
	}
      }
      // sea cual sea la opción, una vez escritos los datos, reportamos si el flag es valido
      if (validreport)
	report();
      else
	nullreport();
    }//valid
  // liberamos la memoria de sourcedata
  free(sourcedata);
  }// working
}

// función auxiliar que añade los datos segun el tipo de deep
// si new==true se usara setdata, si ==false, se usara setmoredata (no se empezara report nuevo)
void TPFC_device_3dfrom2d::setdata(double x, double y, bool newrep, double obsangle){
  double* aux= new double[3];

  // Tipo de calculo de profundidad
  if (deep==FIJA){ // la distancia al plano del sensor es =dist
    aux[0]=tan(x)*dist;
    aux[1]=tan(y)*dist;
    aux[2]=dist;
  } else if (deep==ROTACION){ // la distancia al sensor es =dist
    aux[0]=dist*sin(x);
    aux[1]=dist*sin(y);
    aux[2]=sqrt(dist*dist-aux[0]*aux[0]-aux[1]*aux[1]);
  } else{ //deep==APROXSIZE || ONLYSIZE// la distancia se infiere segun el tamaño observado
    double d;
    // si no tenemos tamaño (por estar usando solo 1 punto), usamos la ultima distancia valida
    if (obsangle==0){
      d=lastvaliddist;
    }else{
      d= (dist/2.0)/tan(obsangle);
      // actualizamos last valid
      lastvaliddist=d;
    }
    // y aplicamos un calculo similar al de rotacion
    aux[0]=d*sin(x);
    aux[1]=d*sin(y);
    aux[2]=sqrt(d*d-aux[0]*aux[0]-aux[1]*aux[1]); 
  }
  // comprobamos si hay que empezar report nuevo o añadir al ya existente
  if (newrep)
    data->setnewdata(aux);
  else
    data->setmoredata(aux);
  // liberamos la memoria del vector auxiliar
  free(aux);
}

// Opciones
// si la fuente no tiene es 2dsize (es solo 2d), no se puede tener merge=false, deeptype=SIZE
// ya que no hay forma de calcular un size
void TPFC_device_3dfrom2d::setmerge(bool m){
  if (source->getdata()->datatype()==TrackingPFC_data::TPFCDATA2D && m==false && deep==APROXSIZE){
    printf("Error: no se puede desactivar el merge si el calculo de profundidad se hace usando aproximacion");
    printf("por tamaño y la fuente no proporciona informacion de tamaño.\n");
    printf("Para desactivar merge, primero se debe cambiar el tipo de calculo de profundidad.\n");
  }else
    merge = m;
}
void TPFC_device_3dfrom2d::setdeep(deeptype d, double di){
  if (source->getdata()->datatype()==TrackingPFC_data::TPFCDATA2D && merge==false && d==APROXSIZE){
    printf("Error: no se puede calcular la profundidad por aproximacion de tamaño");
    printf("si no se esta usando la opcion merge o la fuente proporciona informacion sobre el tamaño.\n");
    printf("Para usar esta opcion, primero se debe activar merge.\n");
  }else{
    deep=d;
    dist=di;
  }
}

// Informacion sobre el dispositivo
string TPFC_device_3dfrom2d::info(){
  char aux[50]; // si se crean mas de 1 millon de dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dfrom2d (usando de fuente el dispositivo %i)", source->idnum());
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_3dfrom2d::checksource(TPFC_device* s){
  string ret = "ok";
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA2D && s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA2DSIZE)
    ret ="El tipo de datos de la fuente no es el adecuado: debe ser un dispositivo 2d (tenga o no estimacion de tamaño).\n";
  return ret;
}