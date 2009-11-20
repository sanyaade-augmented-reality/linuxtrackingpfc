#include "TrackingPFC_client.h"

void hi(TrackingPFC_client* data){
  printf("hello! %f\n",data->getlastposx());
}

int main(){
  TrackingPFC_client* test;
  test = new TrackingPFC_client("Tracker0@localhost",hi);
  //test = new TrackingPFC_client("Tracker0@localhost");
  //test = new TrackingPFC_client();

  if (test!=NULL){
    printf("Existe test!\n");
  }

  while (1){
    vrpn_SleepMsecs(1);
  }
  return 0;
}