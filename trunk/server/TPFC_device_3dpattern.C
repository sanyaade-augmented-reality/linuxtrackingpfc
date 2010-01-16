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
  // { pos{xyz} ori{xyzw} dot0{xyz} dot1{xyz} [dot2{xyz}]}
  lastpattern= new double[7+3*dots];
  anypattern=false; // aun no hemos encontrado patrones
  
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
	included[dn]=false;
      
      bool foundpattern=false; // flag que dice si el report tiene datos validos
      if (n>=dots || all==false){// si requerimos todos los puntos y no hay suficientes, no entramos
	//contador de pares de puntos que estan a la distancia correcta
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
	      // lo marcamos en la lista de incluidos
	      included[d1]=true;
	      included[d2]=true;
	      distok++; // incrementamos el contador
	    }
	  }
	}
	// si dots=2 deberia haber (distok=1)
	// si dots=3 deberia haber (distok=3)
	// si el numero es el esperado, hemos encontrado el patron
	if ( (dots==2 && distok==1) || (dots==3 && distok==3) ){
	  double* newdot = new double[7];
	  newdot[0]=0;
	  newdot[1]=0;
	  newdot[2]=0;
	  // para hacerlo, recorremos el vector de incluidos
	  int pdn=0; // variable auxiliar para saber en que punto del pattern nos encontramos
	  for (int dn =0; dn<n; dn++){
	    // buscamos los que estan a cierto
	    if (included[dn]){
	      const double* aux = sourcedata->getdata(dn);
	      newdot[0]+=aux[0];
	      newdot[1]+=aux[1];
	      newdot[2]+=aux[2];
	      // actualizamos lastpattern
	      lastpattern[7+pdn*3]=aux[0];
	      lastpattern[7+pdn*3+1]=aux[1];
	      lastpattern[7+pdn*3+2]=aux[2];
	      pdn++;
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
	  // guardamos la informacion en lastpattern
	  for (int i =0; i<7;i++)
	    lastpattern[i]=newdot[i];
	  // guardamos el punto
	  data->setnewdata(newdot);
	  // y lo marcamos
	  data->settag(tag);
	  //liberamos el vector auxiliar
	  //free (newdot);
	  // marcamos el flag a cierto ya que hemos incluido datos
	  foundpattern = true;
	} // hay suficientes numeros para el patron
	// si no hemos encontrado el patron completo, pero tenemos la opcion all=false
	// buscamos coincidencias parciales
	// solo entramos aqui si el problema no es que hemos encontrado puntos DE MAS
	// no entramos si aun no hemos encontrado un pattern en ningun report anterior
	if ( !foundpattern && !all && anypattern && ( (dots==2 && distok==0) || (dots==3 && distok<3))){
	  // recorremos los puntos actuales buscando cuales estan mas cerca de los del ultimo pattern
	  double distances[dots];
	  int nearest[dots];
	  float dotdis;// auxiliar para la distancia entre puntos
	  int p1=-1;
	  int p2=-1;
	  for (int dn=0; dn<n;dn++){ // dot number
	    const double* aux = sourcedata->getdata(dn);
	    for (int li=0; li<dots;li++){ // last index
	      dotdis=dotdist(aux,&lastpattern[7+3*li]);
	      if (dn==0 || dotdis<distances[li]){
		distances[li]=dotdis;
		nearest[li]=dn;
	      }
	    }
	    if (included[dn]){
	      // si este punto antes estaba marcado como parte del patron, lo guardamos
	      if (p1!=-1) p2=dn;
	      else p1=dn;
	    }
	  }

	  // si no habia ningun par usable, nos quedamos el que tenga menos distancia,
	  // siempre que esté dentro del umbral
	  int last =0; //indice en lastpattern donde está el punto relacionado
	  if (p1==-1){
	    // encontramos que distancia es la menor
	    if (distances[1]<distances[last])
	      last=1;
	    if (dots==3 && distances[2]<distances[last])
	      last=2;
	    // distances[last] es la menor de las 3
	    // como umbral definimos el tamaño del pattern
	    if (distances[last]<dist)
	      p1=nearest[last]; // guardamos el numero del punto mas cercano al anterior
	  }else{ // p1 y p2 son pares usables, comprobamos si tienen correspondencia con el ultimo pattern
	    if ( !(p1==nearest[0] || p1==nearest[1] || p1==nearest[2]) )
	      p1=-1;
	    if ( !(p2==nearest[0] || p2==nearest[1] || p2==nearest[2]) )
	      p2=-1;
	    // si p2 es valido y p1 no, guardamos p2
	    if (p1==-1 && p2!=-1)
	      p1=p2;
	    // actualizamos en last cual es el punto relacionado
	    if (p1==nearest[1])
	      last=1;
	    else if (p1==nearest[2])
	      last=2;
	  }

	  // llegados aqui, si p1=-1 es que no habia puntos utiles
	  if (p1!=-1){
	    // en last tenemos el indice del punto relacionado de lastpattern
	    // calculamos el vector desde ese punto al centro de su patron
	    double* newdot= new double[7];
	    const double* aux = sourcedata->getdata(nearest[last]);
	    newdot[0]=aux[0]-lastpattern[7+last*3];
	    newdot[1]=aux[1]-lastpattern[8+last*3];
	    newdot[2]=aux[2]-lastpattern[9+last*3];
	    // actualizamos la informacion de los puntos de lastdot, que ya no se usaran mas
	    for (int li=0;li<dots;li++){
	      lastpattern[7+li*3]+=newdot[0];
	      lastpattern[8+li*3]+=newdot[1];
	      lastpattern[9+li*3]+=newdot[2];
	    }
	    // sumamos el vector actual al centro del ultimo pattern
	    // ahora en newpos[0..2] tenemos ya la informacion del nuevo centro del pattern
	    newdot[0]+=lastpattern[0];
	    newdot[1]+=lastpattern[1];
	    newdot[2]+=lastpattern[2];
	    for (int i =0; i<3;i++) // la orientacion no hace falta copiarla, ya que es la misma
	      lastpattern[i]=newdot[i];
	    // copiamos la orientacion del vector pasado
	    newdot[3]=lastpattern[3];
	    newdot[4]=lastpattern[4];
	    newdot[5]=lastpattern[5];
	    newdot[6]=lastpattern[6];
	    
	    // guardamos el punto, pero lo marcamos como artificial
	    data->setnewdata(newdot, false);
	    // y lo marcamos
	    data->settag(tag);
	    //liberamos el vector auxiliar
	    //free (newdot);
	    // marcamos el flag a cierto ya que hemos incluido datos
	    foundpattern = true;
	    
	  }
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
      
      // actualizamos el flag de patrones encontrados
      anypattern = foundpattern || anypattern;
    
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