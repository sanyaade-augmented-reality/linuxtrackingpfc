#include "TPFC_device_3dpattern.h" 


TPFC_device_3dpattern::TPFC_device_3dpattern(int ident, TPFC_device* s, int dot, float dis, 
					     bool al, int t, keepothersoptions others)
					     :TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,100);

  // guardamos los parametros
  dots=dot;
  dist=dis;
  all=al;
  tag=t;
  keepothers=others;

  tolerance=0.2;// 20%
  // posiciones del ultimo patron encontrado
  // { 0{xyz} 1{xyz} [2{xyz}] }
  lastpattern= new double[3*dots];
  
  // guardamos un puntero a la fuente
  source=s;
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  s-> report_to(this);
  
}

TPFC_device_3dpattern::~TPFC_device_3dpattern(){
  free(data);
}

// sobrecarga del handler de los reports
void TPFC_device_3dpattern::report_from(TPFC_device* s){
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
      // creamos un vector de bools[n] para saber si los dots forman parte o no del patron
      bool included[n];
      for (int dn =0; dn<n; dn++)
	included[n]=false;
      
      bool foundpattern=false; // flag que dice si el report tiene datos validos
      if (n>=dots || all==false){// si requerimos todos los puntos y no hay suficientes, no entramos
	// creamos matriz n*n
	bool distances[n][n];
	// y un contador de pares de puntos que estan a la distancia correcta
	int distok=0;
	// vamos punto por punto comprobando si entran dentro de la distancia y la tolerancia
	const double* dot1;
	const double* dot2;
	double dotdis;
	// calculamos los limites de tolerancia
	double tol1 = dist-dist*tolerance;
	double tol2 = dist+dist*tolerance;
	for (int d1=0;d1<n;d1++){
	  dot1 = sourcedata->getdata(d1);
	  for (int d2=d1+1;d2<n;d2++){
	    dot2 = sourcedata->getdata(d2);
	    // calculamos la distancia entre los puntos
	    dotdis=dotdist(dot1,dot2);
	    // si esa distancia esta dentro del limite de tolerancia, marcamos la casilla a cierto
	    if (dotdis>tol1 && dotdis<tol2){
	      distances[d1][d2]=true;
	      distances[d2][d1]=true;
	      // lo marcamos en la lista de incluidos
	      included[d1]=true;
	      included[d2]=true;
	      distok++; // incrementamos el contador
	    }else{// las marcamos a falso
	      distances[d1][d2]=false;
	      distances[d2][d1]=false;
	    }
	  }
	}
	// si dots=2 deberia haber 2(distok=1) ciertos en la matriz
	// si dots=3 deberia haber 6 (distok=3) ciertos en la matriz
	// si el numero es el esperado, hemos encontrado el patron
	if ( (dots==2 && distok==1) || (dots==3 && distok==3) ){
	  double* newdot = new double[7];
	  newdot[0]=0;
	  newdot[1]=0;
	  newdot[2]=0;
	  // para hacerlo, recorremos el vector de incluidos
	  for (int dn =0; dn<n; dn++){
	    // buscamos los que estan a cierto
	    if (included[dn]){
	      const double* aux = sourcedata->getdata(dn);
	      newdot[0]+=aux[0];
	      newdot[1]+=aux[1];
	      newdot[2]+=aux[2];
	    }
	  }
	  // y obtenemos la media, ese es el nuevo punto
	  newdot[0]=newdot[0]/dots;
	  newdot[1]=newdot[1]/dots;
	  newdot[2]=newdot[2]/dots;
	  // calculamos la orientacion
	  newdot[3]=0;
	  newdot[4]=0;
	  newdot[5]=0;
	  newdot[6]=0;
	  // guardamos el punto
	  data->setnewdata(newdot);
	  // y lo marcamos
	  data->settag(tag);
	  //liberamos el vector auxiliar
	  free (newdot);
	  // marcamos el flag a cierto ya que hemos incluido datos
	  foundpattern = true;
	} // hay suficientes numeros para el patron
	// si no hemos encontrado el patron completo, pero tenemos la opcion all=false
	// buscamos coincidencias parciales
	// solo entramos aqui si el problema no es que hemos encontrado puntos DE MAS
	// no entramos si aun no hemos encontrado un pattern en ningun report anterior
	if ( !foundpattern && !all &&  ( (dots==2 && distok==0) || (dots==3 && distok<3) )){
	  //TrackingPFC_data::datachunk* aux = (s->getdata())->getdatabycount(lastpattern);
	}
      } //if (n>=dots || all=false
      // si hay que incluir los puntos que no pertenezcan al patron, los guardamos en data
      if ( (foundpattern && keepothers==WITHPATTERN) ||  keepothers==ALWAYS){
	const double* dotdata;
	// recorremos el vector de incluidos
	for (int dn =0; dn<n; dn++){
	  // si no esta, lo añadimos al report
	  if (!included[dn] || !foundpattern){
	    dotdata = sourcedata->getdata(dn);
	    // si estamos en el primer punto y no habia un patron, hay que crear un nuevo report
	    // si no, simplemente añadir
	    if (dn==0 && !foundpattern)
	      data->setnewdata(dotdata);
	    else
	      data->setmoredata(dotdata);
	  }
	}
      }//if (keepothers){

      // por ultimo, reportamos
      if (foundpattern ||  keepothers==ALWAYS){
	report();
      }else{
	data->setnodata();
	nullreport();
      }
    }// validos
  }//working
}

// funcion auxiliar que calcula la distancia entre 2 puntos
double TPFC_device_3dpattern::dotdist(const double* d1,const double* d2){
  return sqrt( (d1[0]-d2[0])*(d1[0]-d2[0]) + (d1[1]-d2[1])*(d1[1]-d2[1]) + (d1[2]-d2[2])*(d1[2]-d2[2]) );
}

// Informacion sobre el dispositivo
string TPFC_device_3dpattern::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dpattern (usando de fuente el dispositivo %i)", source->idnum());
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_3dpattern::checksource(TPFC_device* s){
  string ret = "ok";
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3D && s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3DORI)
    ret ="El tipo de datos de la fuente no es el adecuado: debe ser un dispositivo 3d (tenga o no orientacion).\n";
  return ret;
}