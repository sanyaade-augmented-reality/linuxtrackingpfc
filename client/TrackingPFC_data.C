#include "TrackingPFC_data.h"
TrackingPFC_data::TrackingPFC_data(float x, float y, float z){
  obsx=x;
  obsy=y;
  obsz=z;
}

// Destructora
TrackingPFC_data::~TrackingPFC_data(){
}

float* TrackingPFC_data::getlastpos(){
  float* res= new float[3];
  res[0]=obsx;
  res[1]=obsy;
  res[2]=obsz;
  return res;
}

void TrackingPFC_data::setnewpos(float x, float y, float z){
  obsx=x;
  obsy=y;
  obsz=z;
}