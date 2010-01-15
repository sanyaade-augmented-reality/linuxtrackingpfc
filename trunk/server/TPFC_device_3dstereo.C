#include "TPFC_device_3dstereo.h" 


TPFC_device_3dstereo::TPFC_device_3dstereo(int ident, TPFC_device* s1, TPFC_device* s2):TPFC_device(ident){

  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D);

  lock = new pthread_mutex_t(); // inicializamos el semaforo

  // guardamos un puntero a las fuentes
  sources= new TPFC_device*[2];
  sources[0]= s1;
  sources[1]= s2;

  // y creamos un vector donde guardar los reports
  lastdata = new TrackingPFC_data::datachunk*[2];
  lastdata[0]=NULL;
  lastdata[1]=NULL;

  // marcamos el sensor izquierdo con un valor invalido, y el contador de fallos a 0
  left=-1;
  fails=0;

  // inicializamos camdist con la distancia standad entre 2 wiimotes
  camdist=0.036;

  // registramos este dispositivo en la lista de listeners que vamos a usar como input
  s1-> report_to(this);
  s2-> report_to(this);

}

TPFC_device_3dstereo::~TPFC_device_3dstereo(){
  free(data);
  if (lastdata[0]!=NULL);
    free(lastdata[0]);
  if (lastdata[1]!=NULL);
    free(lastdata[1]);
}

double TPFC_device_3dstereo::angleconversion(double original, int sensor){
  // en un principio un angulo de 0 es un angulo recto
  double converted=(TPFCPI/2.0);
  // dependiendo del sensor en el que estamos, sumamos o restamos el angulo original
  if (sensor==left){
    converted+=original;
  }else{
    converted-=original;
  }
  return converted;
}

// devuelve el indice interno de la fuente (si esta en sources[0] o sources[1])
int TPFC_device_3dstereo::getsourcepos(TPFC_device* s){
  if (s==sources[0])
    return 0;
  else
    return 1;
}

// sobrecarga del handler de los reports
void TPFC_device_3dstereo::report_from(TPFC_device* s){
  // comprobamos que no estemos en pausa
  if (working()){
    // guardamos estos datos en su buffer
    int sn = getsourcepos(s);
    pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
    if (lastdata[sn]!=NULL){
      // si habia datos, los eliminamos antes
      free(lastdata[sn]);
      lastdata[sn]=NULL;
    }
    lastdata[sn]= (s->getdata())->getlastdata();
    
    // tabla auxiliar para ordenar los datos de los puntos
    // dots[sn][dn][xy]
    int totaldots = lastdata[sn]->size();
    double dots[2][totaldots][2];
    // flag de si hay que reportar o no
    bool needreport=false;
    // comprobamos si los datos son validos y el numero de puntos coincide
    if (lastdata[0]!=NULL && lastdata[0]->getvalid() && 
	  lastdata[1]!=NULL && lastdata[1]->getvalid() &&
	  lastdata[1]->size()==lastdata[0]->size()){
      // rellenamos dots
      const double* tmp;
      for (int dn=0;dn<totaldots;dn++){
	// datos del sensor 0
	tmp = lastdata[0]->getdata(dn);
	dots[0][dn][0]=tmp[0];
	dots[0][dn][1]=tmp[1];
	// datos del sensor 1
	tmp = lastdata[1]->getdata(dn);
	dots[1][dn][0]=tmp[0];
	dots[1][dn][1]=tmp[1];
      }
      // marcamos el flag de nuevo report
      needreport=true;
      // por ultimo, eliminamos los datos de la fuente mas vieja
      free(lastdata[ (sn+1)%1 ]);
      lastdata[ (sn+1)%1 ]=NULL;
    }
    pthread_mutex_unlock( lock ); // liberamos acceso exclusivo
    
    // en caso de tener que hacer un nuevo report, tenemos los datos, pero sin ordenar
    if (needreport){
      // ordenamos de mayor a menor el angulo horizontal y en caso de empate, por el vertical
      // ya que el numero de puntos no es muy elevado, un algoritmo burbuja es suficiente para ordenar
      for (int i=0; i<2; i++){
	for (int dn=0;dn<totaldots;dn++){
	  for (int dnx=dn+1;dnx<totaldots;dnx++){
	    // comprobamos si el angulo de dn es menor que el de dnx
	    // y en caso de empate, si el vertical lo es
	    if ( dots[i][dn][0]<dots[i][dnx][0] || 
	        (dots[i][dn][0]==dots[i][dnx][0] && dots[i][dn][1]<dots[i][dnx][1]) ){
	      double aux=dots[i][dn][0];
	      dots[i][dn][0]=dots[i][dnx][0];
	      dots[i][dnx][0]=aux;
	      aux=dots[i][dn][1];
	      dots[i][dn][1]=dots[i][dnx][1];
	      dots[i][dnx][1]=aux;
	    }
	  } 
	}
      }

      // Primero deducimos que sensor es el de cada lado
      // el izquierdo (desde el punto de vista de los sensores) será aquel cuyo angulo
      // horizontal sea menor

      bool sideok=true; // flag que indica que todos los puntos son congruentes
      int izq= ( dots[0][0][0]<dots[1][0][0] )?0:1; // deduccion del primer punto
      // recorremos el resto de puntos comprobando que coincida
      int izq2;
      for (int dn =1; dn<totaldots;dn++){
	izq2 = ( dots[0][dn][0]<dots[1][dn][0] )?0:1;
	// si no lo hace, marcamos sideok a falso
	if (izq!=izq2){
	  sideok=false;
	}
      }
      // Avisamos al usuario si ha fallado la deteccion de colocacion
      if (!sideok)
	printf("3dstereo: no ha sido posible determinar la colocacion de los sensores en este report\n");
      // si el report es congruente y left esta sin inicializar, lo inicializamos
      if ( sideok && left==-1)
	left=izq;
      // comprobamos que coincida el valor almacenado de left con el obtenido
      if (sideok && izq!=left){
	fails++; // incrementamos el contador de fallos
	if (fails >20){ // si llevamos 20 fallos consecutivos, entendemos que la deteccion anterior era erronea
	  fails=0; // resteamos los contadores
	  left=izq; // cambiamos la izquierda
	  printf("3dstereo: Detectados 20 errores de deteccion consecutivos. Cambiando izquierdo a dispositivo %i\n", sources[left]->idnum());
	}else{
	  sideok=false; // si no lo es, marcamos la congruencia como erronea
	  printf("3dstereo: la colocacion detectada no coincide con la de los anteriores reports (%i).\n", fails);
	}
      }
      // llegados a este punto, si la detección de posicion del sensor
      // ha fallado, no habra datos que reportar
      if (!sideok){
	needreport=false;
      }else{// el report es valido, seguimos con el proceso
	// reseteamos el contador de fallos
	fails=0; // resteamos los contadores
	// realizamos calculos
	// variables para las posiciones de los puntos
	double x,y,z;
	double ang0, ang1;
	for (int dn =0; dn<totaldots;dn++){
	  // calculo de la profundidad
	  // pasamos los angulos al sistema de referencia necesario
	  ang0 = angleconversion(dots[0][dn][0],0);
	  ang1 = angleconversion(dots[1][dn][0],1);
	  
	  // los 2 calculos son equivalentes, pero el de abajo tiene una division menos
	  //z= camdist/( (1.0/tan(ang0) ) + (1.0/tan(ang1) ));
	  z= camdist / ( ( tan(ang0)+tan(ang1) ) / ( tan(ang0)*tan(ang1) ) );

	  // calculo de la posición horizontal
	  // damos la distancia desde el sensor derecho, y restamos la mitad
	  // de la distancia entre sensores para dar la posicion desde el centro
	  x = ( z/ tan(angleconversion(dots[ (left+1)%1 ][dn][0], (left+1)%1 )) )- camdist/2.0;
	  
	  // calculo de la posicion vertical
	  y=z*tan( ( dots[0][dn][1] + dots[1][dn][1] )/2.0 );

	  // guardamos en data
	  double* aux = new double[3];
	  aux[0]=x;
	  aux[1]=y;
	  aux[2]=z;
	  if (dn==0) // estamos en un nuevo report
	    data->setnewdata(aux);
	  else
	    data->setmoredata(aux);
	  free(aux);
	}//for (int dn =0; dn<totaldots;dn++)
      }// sideok
    } // needreport
    if (needreport){
      // y reportamos
      report();
    }else{
      // en esta toma de datos no hay datos que reportar
      data->setnodata();
      nullreport();
    }
  }// if workikg
}

// sobrecarga del handler de los reports nulos
void TPFC_device_3dstereo::nullreport_from(TPFC_device* s){
  // redirigimos al report normal (ya se gestiona internamente la validez)
  report_from(s);
}

// Informacion sobre el dispositivo
string TPFC_device_3dstereo::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dstereo (usando de fuente los dispositivo %i y %i)", sources[0]->idnum(), sources[1]->idnum());
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_3dstereo::checksource(TPFC_device* s){
  string ret = "ok";
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA2D && s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA2DSIZE)
    ret ="El tipo de datos de la fuente no es el adecuado: debe ser un dispositivo 2d (tenga o no estimacion de tamaño).\n";
  return ret;
}