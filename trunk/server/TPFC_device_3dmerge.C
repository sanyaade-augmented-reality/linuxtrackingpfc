#include "TPFC_device_3dmerge.h" 


TPFC_device_3dmerge::TPFC_device_3dmerge(int ident, TPFC_device* s1, TPFC_device* s2):TPFC_device(ident){
  // creamos los datos
  // tendremos orientacion solo si las 2 fuentes la tienen
  if (s1->getdata()->datatype()==TrackingPFC_data::TPFCDATA3DORI && s2->getdata()->datatype()==TrackingPFC_data::TPFCDATA3DORI){
    data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);
  }else{
    data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D,1);
  }
  datad[0]=NULL;
  datad[1]=NULL;

  // inicializamos el semaforo
  lock = new pthread_mutex_t(); 

  // inicializamos la diferenciacion a falso
  different=false;

  // guardamos los punteros a las fuentes
  sources[0]=s1;
  sources[1]=s2;
  
  // registramos como listener
  s1-> report_to(this);
  s2-> report_to(this);
}

TPFC_device_3dmerge::~TPFC_device_3dmerge(){
  free(data);
}


// cambiar el modo
void TPFC_device_3dmerge::setdifferent(bool m){
  if (m && datad[0]==NULL){
    datad[0] = new TrackingPFC_data(data->datatype(),1);
    datad[1] = new TrackingPFC_data(data->datatype(),1);
  }
  different = m;
}

// sobrecarga del handler de los reports
void TPFC_device_3dmerge::report_from(TPFC_device* s){
  // comprobamos que no estemos en pausa
  if (working()){
    // obtenemos los datos
    TrackingPFC_data::datachunk* sourcedata= (s->getdata())->getlastdata();
    // comprobamos si los datos son validos
    if (sourcedata->getvalid() == false){
      if (different){
	if (s->idnum()==sources[0]->idnum()){
	  datad[0]->setnodata();
	}else{
	  datad[1]->setnodata();
	}
      }else{
	// si no son validos guardamos un chunk no valido en nuestros datos y damos un nullreport
	data->setnodata();
      }
      nullreport();
    }else{// si son validos...
      // obtenemos el numero de puntos del report
      int n = sourcedata->size();
      
      pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
      if (different){
	const double* dotdata = sourcedata->getdata();
	// Comprobamos de que fuente viene el report
	int t0=0;
	int t1=0;
	if (s->idnum()==sources[0]->idnum()){// fuente 1
	  datad[0]->setnewdata(dotdata);
	  datad[0]->settag(1);
	  t0=1;
	}else{
	  datad[1]->setnewdata(dotdata);
	  datad[1]->settag(1);
	  t1=1;
	}
	data-> setnewdata(datad[0]->getlastdata()->getdata());
	data-> setmoredata(datad[1]->getlastdata()->getdata());
	data->settag(t0,0);
	data->settag(t1,1);
      }else{
	// recorremos los puntos del report
	for (int dn=0; dn<n; dn++){
	  // obtenemos los datos del punto
	  const double* dotdata = sourcedata->getdata(dn);
	  // los guardamos
	  if (dn==0) data->setnewdata(dotdata);
	  else data->setmoredata(dotdata);
	}
      }
      pthread_mutex_unlock( lock ); // obtenemos acceso exclusivo
      // reportamos
      if (different)
	report(true);
      else
	report();
      

    }// validos
  }//working
}


// Informacion sobre el dispositivo
string TPFC_device_3dmerge::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dmerge con fuentes %i %i", sources[0]->idnum(), sources[1]->idnum());
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_3dmerge::checksource(TPFC_device* s){
  string ret = "ok";
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3D && s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3DORI)
    ret ="El tipo de datos de la fuente no es el adecuado: debe ser un dispositivo 3d (tenga o no orientacion).\n";
  return ret;
}