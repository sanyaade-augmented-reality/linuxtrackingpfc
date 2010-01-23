#include "TPFC_device_3dmod.h" 


TPFC_device_3dmod::TPFC_device_3dmod(int ident, TPFC_device* s):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);

  // escala
  scale = 1.00; //100%, no se modifica

  // reorientacion
  if (s->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3DORI)
    // si la fuente no tiene orientacion, por defecto se le da 1
    orientopt=ALL; // no se aplican cambios de orientacion
  else
    orientopt=NONE; // no se aplican cambios de orientacion
  orientdir=FORWARD;

  // localizacion
  location = NULL;
  calibrando = false;

  // guardamos un puntero a la fuente
  source=s;

  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  s-> report_to(this);
  
}

TPFC_device_3dmod::~TPFC_device_3dmod(){
  free(data);
}

// sobrecarga del handler de los reports
void TPFC_device_3dmod::report_from(TPFC_device* s){
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

      // Obtencion de datos para calibrado
      // solo si estamos calibrando, el report tiene exactamente los puntos requeridos
      // y el report no es artificial
      if (calibrando && n==dots && sourcedata->getreal()){
	bool needdata=false; // flag para saber si debemos guardar los datos
	pthread_mutex_lock( caliblock ); // obtenemos acceso exclusivo
	if (processedsamples<TPFC_CALIBSAMPLES){ // si aun no tenemos suficientes
	  processedsamples++;
	  needdata=true;
	}
	pthread_mutex_unlock( caliblock ); // liberamos acceso exclusivo

	if (needdata){
	  // primero ordenamos los puntos
	  int order[dots]; // orden
	  if (dots==1){
	    order[0]=0;
	  }else{ // 2 o 3 dots
	    double xs[dots];// bufer donde guardaremos la posicion x de los puntos para ordenarlos
	      for (int dn=0; dn<n;dn++){
		const double* dotdata = sourcedata->getdata(dn);
		xs[dn]=dotdata[0];
	      }
	    // obtenemos el orden a partir de los datos de xs
	    if (dots==2){
	      if (xs[0]>xs[1]){
		order[0]=0;order[1]=1;
	      }else{
		order[0]=1;order[1]=0;
	      }
	    }else{ // dots = 3
	      order[0]=0;order[1]=1;order[2]=2;
	      if (xs[order[1]]>xs[order[0]]){
		int aux = order[0];
		order[0]=order[1];
		order[1]=aux;
	      }
	      if (xs[order[2]]>xs[order[0]]){
		int aux = order[0];
		order[0]=order[2];
		order[2]=aux;
	      }
	      if (xs[order[2]]>xs[order[1]]){
		int aux = order[1];
		order[1]=order[2];
		order[2]=aux;
	      }
	    }// 3 dots
	  } // 2 o 3 dots

	  // los guardamos por ese orden
	  for (int dn=0; dn<n;dn++){
	    const double* dotdata = sourcedata->getdata(order[dn]);
	    if (dn==0)
	      calibdata->setnewdata(dotdata);
	    else
	      calibdata->setmoredata(dotdata);
	  }
	}
	
      }
  
      for (int dn=0; dn<n;dn++){
	// obtenemos los datos reales
	const double* dotdata = sourcedata->getdata(dn);

	double* newdot = new double[7];

	// copiamos los datos
	newdot[0]=dotdata[0];
	newdot[1]=dotdata[1];
	newdot[2]=dotdata[2];

	// Aplicamos el cambio de ubicacion
	if (location!=NULL){
	  //printf("olddot: %f %f %f\n", newdot[0], newdot[1], newdot[2]);
	  // primero aplicamos la translacion
	  newdot[0]-=location[0];
	  newdot[1]-=location[1];
	  newdot[2]-=location[2];
	  
	  double aux[3];
	  aux[0]=newdot[0]*rotation[0][0]+newdot[1]*rotation[0][1]+newdot[2]*rotation[0][2];
	  aux[1]=newdot[0]*rotation[1][0]+newdot[1]*rotation[1][1]+newdot[2]*rotation[1][2];
	  aux[2]=newdot[0]*rotation[2][0]+newdot[1]*rotation[2][1]+newdot[2]*rotation[2][2];

	  newdot[0]=aux[0];
	  newdot[1]=aux[1];
	  newdot[2]=aux[2];
	  //printf("newdot: %f %f %f\n", newdot[0], newdot[1], newdot[2]);
	}


	if (kalman.size()!=0){ // hay filtros de kalman
	  // si no tenemos suficientes filtros, creamos uno nuevo
	  if (kalman.size()<(dn+1))
	    addkalman();

	  // Guardamos los datos para corregir el filtro en un vector opencv
	  CvMat* z_k = cvCreateMat( 3, 1, CV_32FC1 );
	  cvZero( z_k );
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,0,0 ) ) = newdot[0];
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,1,0 ) ) = newdot[1];
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,2,0 ) ) = newdot[2];

	  // obtenemos la predicción del filtro de kalman
	  double* aux = new double[3];
	  const CvMat* y_k = cvKalmanPredict( kalman[dn], 0 );
	  newdot[0] =CV_MAT_ELEM(*y_k,float,0,0);
	  newdot[1]=CV_MAT_ELEM(*y_k,float,1,0);
	  newdot[2] =CV_MAT_ELEM(*y_k,float,2,0);

	  // aplicamos la correccion al filtro
	  cvKalmanCorrect( kalman[dn], z_k );
	}
	
	// Aplicamos la escala
	newdot[0]=newdot[0]*scale;
	newdot[1]=newdot[1]*scale;

	// Reajuste de orientacion
	if ( orientopt==ALL || // si hay que reorientar todos
	     (orientopt==UNTAGGED && sourcedata->gettag(dn)!=0) ){ // o solo los que estan sin marcar
	  if (orientdir==FORWARD){
	    // la orientacion debe ser el quaternion nulo (perpendicular al plano normal del sensor)
	    newdot[3]=0;
	    newdot[4]=0;
	    newdot[5]=0;
	    newdot[6]=1;
	  }else{ //CENTER
	    q_vec_type cent, norm;
	    q_type rot;
	    q_vec_set(norm,0,0,-1);
	    q_vec_set(cent,-newdot[0],-newdot[1],-newdot[2]);
	    q_from_two_vecs(rot,norm,cent);
	    newdot[3]=rot[Q_X];
	    newdot[4]=rot[Q_Y];
	    newdot[5]=rot[Q_Z];
	    newdot[6]=rot[Q_W];
	  }
	}//reorientacion


	//guardamos los datos
	if (dn==0)
	  data->setnewdata(newdot);
	else
	  data->setmoredata(newdot);

	// liberamos el vector
	free(newdot);
	  
      }// recorrido por los puntos 

      // reportamos
	report();

    } // validos
    delete(sourcedata);
  }//working
}

// añade un filtro de kalman
void TPFC_device_3dmod::addkalman(){
    CvRandState rng;
    cvRandInit( &rng, 0, 1, -1, CV_RAND_UNI );
    CvKalman* newkalman = cvCreateKalman( 6, 3, 0 );
    cvSetIdentity( newkalman->measurement_matrix,    cvRealScalar(1) );
    cvSetIdentity( newkalman->process_noise_cov,     cvRealScalar(1e-5) );
    cvSetIdentity( newkalman->measurement_noise_cov, cvRealScalar(1e-1) );
    cvSetIdentity( newkalman->error_cov_post,        cvRealScalar(1));
    const float F[] = { 1,0,0,1,0,0,
			0,1,0,0,1,0,
			0,0,1,0,0,1,
			0,0,0,1,0,0,
			0,0,0,0,1,0,
			0,0,0,0,0,1,
			};
    memcpy( newkalman->transition_matrix->data.fl, F, sizeof(F));
    cvRand( &rng, newkalman->state_post );

    // lo añadimos al vector
    kalman.push_back(newkalman);
}

void TPFC_device_3dmod::setscale(double s){
  scale= s;
}

void TPFC_device_3dmod::setorientation(reorientopt o, reorientdir d){
  if (!(source->getdata()->datatype()!=TrackingPFC_data::TPFCDATA3DORI && o==NONE))
    orientopt=o;
  else
    printf("La fuente no tiene orientacion, no se puede desactivar la reorientacion, ignorando orden.\n");
  orientdir=d;
}

// Calibrado
double* TPFC_device_3dmod::calibrate(int d, double dis, double* c){
  double* loc = new double[3];
  if (c==NULL){
    // Tenemos que obtener los datos

    // guardamos la cantidad de puntos usados
    // esta es la cantidad de puntos que se esperan en cada toma
    // en total siempre se necesitan 3 puntos
    dots = d;

    // inicializaciones
    calibdata = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D,TPFC_CALIBSAMPLES);
    caliblock = new pthread_mutex_t(); // inicializamos el semaforo
    int processeddots=0;
    int warn;
    double dotdata[4][3];
    for (int i =0; i<3;i++)
      for (int j =0; j<3;j++)
	dotdata[i][j]=0.0;

    // Explicacion al usuario
    if (dots==1){
      printf("Calibración usando 1 solo marcador:\n");
      printf("Se realizaran tres tomas de datos.");
      printf("Para todas las tomas, el marcador debe estar a una distancia de %f metros.",dis);
      printf("del plano del display.");
      printf("Las 3 posiciones para las tomas deben formar un triangulo bien diferenciado,");
      printf("cuyo centro esté alineado con el centro del display.");
    }else if (dots==2){
      printf("Calibración usando 2 marcadores simultaneos:\n");
      printf("Se realizaran dos tomas de datos, manten el marcador en posicion horizontal, ");
      printf("paralelo al display a una distancia de %f metros.",dis);
      printf("La posicion en ambas tomas debe estar centrada respecto al eje horizontal del display");
      printf("para las 2 tomas, varia la posicion vertical unos centimetros, primero hacia arriba,");
      printf("despues hacia abajo, procurando que el centro del display quede entre ambas posiciones.");
    }else{
      printf("Calibracion usando 3 marcadores simultaneos:");
      printf("Se realizará una sola toma de datos, manten el marcador frente al centro del display");
      printf("a una distancia de %f metros.",dis);
    }
    while(processeddots<3){
    
      warn= TPFC_CALIBINC;
      processedsamples =0;
      
      // Mensajes de aviso con cuenta atrás para el usuario
      printf("Preparando para obtener datos en...\n");
      for (int i =5; i>0;i--){
	printf("%i ",i);
	fflush(stdout);
	vrpn_SleepMsecs(1000);
      }
      printf("Adquiriendo datos\n");

      // activamos el flag de calibrando
      calibrando = true;

      // esperamos a que el handler de reports llene el buffer
      while (processedsamples<TPFC_CALIBSAMPLES){
	vrpn_SleepMsecs(10); // sleep para no consumir cpu
	if (processedsamples>=warn){ // barra de progreso
	  printf(".");
	  fflush(stdout);
	  warn+=TPFC_CALIBINC;
	}
      }

      // desactivamos el flag de calibrando, para dejar de capturar datos
      calibrando = false;

      // procesamos los datos
      printf("\nDatos adquiridos, procesando...\n");
      int c= calibdata->getcount();
      for (int i=0; i<TPFC_CALIBSAMPLES; i++){
	TrackingPFC_data::datachunk* sampledata=calibdata->getdata(c-i);
	for (int dn=0; dn<dots;dn++){
	  const double* aux= sampledata->getdata(dn);
	  dotdata[dn+processeddots][0]+=aux[0];
	  dotdata[dn+processeddots][1]+=aux[1];
	  dotdata[dn+processeddots][2]+=aux[2];
	}
	delete(sampledata);
      }
      
      // incrementamos el contador de datos procesados
      processeddots+=dots;

    }//while(processeddots<3){

    // CALCULAMOS LO NECESARIO
    // hacemos las medias
    int totaldots = (dots==2)?4:3;
    for (int i =0; i<totaldots;i++)
      for (int j =0; j<3;j++){
	dotdata[i][j]=dotdata[i][j]/(double)TPFC_CALIBSAMPLES;
      }
    
    // calculamos en loc el punto medio de los 3 puntos
    loc[0]=(dotdata[0][0]+dotdata[1][0]+dotdata[2][0])/3.0;
    loc[1]=(dotdata[0][1]+dotdata[1][1]+dotdata[2][1])/3.0;
    loc[2]=(dotdata[0][2]+dotdata[1][2]+dotdata[2][2])/3.0;

    // calculamos 2 vectores con los 3 puntos de los samples
    q_vec_type v1, v2,vn, vns;
    q_vec_set(v1, dotdata[0][0]-dotdata[1][0], dotdata[0][1]-dotdata[1][1], dotdata[0][2]-dotdata[1][2]);
    q_vec_set(v2, dotdata[0][0]-dotdata[2][0], dotdata[0][1]-dotdata[2][1], dotdata[0][2]-dotdata[2][2]);
    //calculamos la normal al plano que forman los vectores
    // realizamos el producto vectorial
    q_vec_cross_product(vn,v1,v2);
    // y normalizamos
    q_vec_normalize(vn,vn);
    // si el vector se aleja del sensor, lo invertimos
    if (vn[Q_Z]>0)
      q_vec_invert(vn,vn);

    // sumamos este vector a la posicion media de los puntos, con eso obtenemos la posicion del display
    // multiplicamos por dis ya que vn es normalizado, por lo que la distancia seria 1
    loc[0]+=vn[Q_X]*dis;
    loc[1]+=vn[Q_Y]*dis;
    loc[2]+=vn[Q_Z]*dis;

    // obtenemos el quaternion que rota de la normal al plano del sensor a la normal al plano del display
    q_type rot;
    q_vec_set(vns, 0,0,-1);
    q_from_two_vecs(rot, vns, vn);

    q_to_row_matrix(rotation, rot);
    location=loc;
    
    // eliminamos el buffer
    delete(calibdata);
    // creamos el vector resultado
    double* res = new double[7];
    res[0]=location[0];
    res[1]=location[1];
    res[2]=location[2];
    res[3]=rot[Q_X];
    res[4]=rot[Q_Y];
    res[5]=rot[Q_Z];
    res[6]=rot[Q_W];
    return res;
  }else{ //if c==NULL
    // simplemente cargamos los datos de c
    double* aux = new double[3];
    aux[0]=c[0];
    aux[1]=c[1];
    aux[2]=c[2];
    q_type rot;
    rot[Q_X]=c[3];
    rot[Q_Y]=c[4];
    rot[Q_Z]=c[5];
    rot[Q_W]=c[6];
    q_to_row_matrix(rotation, rot);
    location=aux;
    return NULL;
  }

}

// Informacion sobre el dispositivo
string TPFC_device_3dmod::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dmod (usando de fuente el dispositivo %i)", source->idnum());
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