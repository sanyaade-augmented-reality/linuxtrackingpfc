#include "TrackingPFC_data.h"
TrackingPFC_data::TrackingPFC_data(float x, float y, float z){
  obsx=x;
  obsy=y;
  obsz=z;
  lock = new pthread_mutex_t();
}

// Destructora
TrackingPFC_data::~TrackingPFC_data(){
}

float* TrackingPFC_data::getlastpos(){
  float* res= new float[3];
  pthread_mutex_lock( lock );
  res[0]=obsx;
  res[1]=obsy;
  res[2]=obsz;
  pthread_mutex_unlock( lock );
  return res;
}

void TrackingPFC_data::setnewpos(float x, float y, float z){
  pthread_mutex_lock( lock );
  obsx=x;
  obsy=y;
  obsz=z;
  pthread_mutex_unlock( lock );
}