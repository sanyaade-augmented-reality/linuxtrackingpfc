#include "TrackingPFC_data.h"
TrackingPFC_data::TrackingPFC_data(TPFCdatatype t, int s){
  lock = new pthread_mutex_t();
  type = t;
  size = s;

  if (type==TPFCDATA2D) dsize=2;
  else if (type==TPFCDATA2DSIZE) dsize=3;
  else if (type==TPFCDATA3D) dsize= 3;
  else if (type==TPFCDATA3DORI) dsize= 7;

  ind =-1;
  count =0;
  data = (float*)malloc(size*dsize*sizeof(float));

  // ponemos los datos por defecto
  // aunque para el tipo de datos se guarden menos de 7 floats, setnewdata se encarga de copiar
  // solo los necesarios
  float* defaultdata = new float[7];
  defaultdata[0]=0.0;
  defaultdata[1]=0.0;
  defaultdata[2]=0.5;
  defaultdata[3]=0.0;
  defaultdata[4]=0.0;
  defaultdata[5]=0.0;
  defaultdata[6]=0.0;
  setnewdata(defaultdata);
  
}

// Destructora
TrackingPFC_data::~TrackingPFC_data(){
  free(data);
}

float* TrackingPFC_data::getlastpos(){
  float* res= new float[dsize];
  pthread_mutex_lock( lock );
  int desp = ind*dsize;
  for (int i =0; i<dsize; i++){
    res[i]=data[desp];
    desp++;
  }
  pthread_mutex_unlock( lock );
  return res;
}

// copia la orientacion de los ultimos datos, modificando la posición.
// aunque está pensado para funcionar con datos de tamaño 3 o mas, funcionara con los de 2
// ignorando el 3r argumento
void TrackingPFC_data::setnewpos(float x, float y, float z){
  float* aux = getlastpos();
  pthread_mutex_lock( lock );
  aux[0]=x;
  aux[1]=y;
  if (dsize >2)
    aux[2]=z;
  pthread_mutex_unlock( lock );
  setnewdata(aux);
}

void TrackingPFC_data::setnewdata(float* d){
  pthread_mutex_lock( lock );
  ind=(ind+1)%size;
  int desp = ind*dsize;
  for (int i =0; i<dsize; i++){
    data[desp]= d[i];
    desp++;
  }
  count++;
  pthread_mutex_unlock( lock );
}

int TrackingPFC_data::datatype(){
  return type;
}

inline int TrackingPFC_data::datasize(){
  return dsize;
}