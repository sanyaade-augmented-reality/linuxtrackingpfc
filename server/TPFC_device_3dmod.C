#include "TPFC_device_3dmod.h" 


TPFC_device_3dmod::TPFC_device_3dmod(int ident, TPFC_device* s):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);

  // escala
  scale = 1; //100%, no se modifica

  // reorientacion
  orientopt=NONE; // no se aplican cambios de orientacion
  orientdir=FORWARD;

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
  
      for (int dn=0; dn<n;dn++){
	// obtenemos los datos reales
	const double* dotdata = sourcedata->getdata(dn);

	double* newdot = new double[7];

	if (kalman.size()!=0){ // hay filtros de kalman
	  // FALTA comprobar que no estemos en un punto sin filtro
	  // si no tenemos suficientes filtros, creamos uno nuevo
	  if (kalman.size()<(dn+1))
	    addkalman();
	  // obtenemos la predicción del filtro de kalman
	  const CvMat* y_k = cvKalmanPredict( kalman[dn], 0 );
	  newdot[0] =CV_MAT_ELEM(*y_k,float,0,0);
	  newdot[1]=CV_MAT_ELEM(*y_k,float,1,0);
	  newdot[2] =CV_MAT_ELEM(*y_k,float,2,0);

	  // preparamos el vector opencv
	  CvMat* z_k = cvCreateMat( 3, 1, CV_32FC1 );
	  cvZero( z_k );
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,0,0 ) ) = dotdata[0];
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,1,0 ) ) = dotdata[1];
	  *( (float*)CV_MAT_ELEM_PTR(*z_k,2,0 ) ) = dotdata[2];
	  // introducimos las lecturas en el filtro
	  cvKalmanCorrect( kalman[dn], z_k );
	}else{// no hay filtro de kalman
	  newdot[0]=dotdata[0];
	  newdot[1]=dotdata[1];
	  newdot[2]=dotdata[2];
	}//kalman
	
	// Aplicamos la escala
	newdot[0]=newdot[0]*scale;
	newdot[1]=newdot[1]*scale;

	// Reajuste de orientacion
	if ( orientopt==ALL || // si hay que reorientar todos
	     (orientopt==UNTAGGED && sourcedata->gettag(dn)!=0) ){ // o solo los que estan sin marcar
	  if (orientdir==FORWARD){
	    // la orientacion debe ser el quaternion nulo (perpendicular al plano normal del sensor)
	    newdot[3]=0;
	    newdot[4]=1;
	    newdot[5]=0;
	    newdot[6]=0;
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


      report();

    } // validos
  }//working
}

// activa el filtro de kalman
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

void TPFC_device_3dmod::setrotation(reorientopt o, reorientdir d){
  orientopt=o;
  orientdir=d;
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