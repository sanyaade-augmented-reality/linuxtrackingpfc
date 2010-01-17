#include "TPFC_device_3dmod.h" 


TPFC_device_3dmod::TPFC_device_3dmod(int ident, TPFC_device* s):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);

  // kalman
  kalman = NULL;
  activatekalman();

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
      // predict point position
      const CvMat* y_k = cvKalmanPredict( kalman, 0 );
      double x =CV_MAT_ELEM(*y_k,float,0,0);
      double y =CV_MAT_ELEM(*y_k,float,1,0);
      double z =CV_MAT_ELEM(*y_k,float,2,0);
      data->setnewpos(x,y,z);

      const double* dotdata = sourcedata->getdata();
      CvMat* z_k = cvCreateMat( 3, 1, CV_32FC1 );
      cvZero( z_k );
      *( (float*)CV_MAT_ELEM_PTR(*z_k,0,0 ) ) = dotdata[0];
      *( (float*)CV_MAT_ELEM_PTR(*z_k,1,0 ) ) = dotdata[1];
      *( (float*)CV_MAT_ELEM_PTR(*z_k,2,0 ) ) = dotdata[2];
      cvKalmanCorrect( kalman, z_k );
      
      report();

    } // validos
  }//working


}

// activa el filtro de kalman
void TPFC_device_3dmod::activatekalman(){
    CvRandState rng;
    cvRandInit( &rng, 0, 1, -1, CV_RAND_UNI );

    kalman = cvCreateKalman( 3, 3, 0 );


    cvSetIdentity( kalman->measurement_matrix,    cvRealScalar(1) );
    cvSetIdentity( kalman->process_noise_cov,     cvRealScalar(1e-5) );
    cvSetIdentity( kalman->measurement_noise_cov, cvRealScalar(1e-1) );
    cvSetIdentity( kalman->error_cov_post,        cvRealScalar(1));


    cvRand( &rng, kalman->state_post );
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