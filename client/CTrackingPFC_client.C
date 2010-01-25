#include "CTrackingPFC_client.h"
#include "TrackingPFC_client.h"

extern "C" {


/*class TrackingPFC_client{
  public:
    // Creadoras y destructora
    //TrackingPFC_client(); // No hace falta, la de abajo la suple
    TrackingPFC_client(const char* tname="Tracker0@localhost", void(TrackingPFC_client*)=NULL);
    ~TrackingPFC_client();
    
    // Devuelve el tamaño de la pantalla (horizontal)
    float getDisplaySizex();
    //float getDisplaySizey();
    //float getDisplayRatio();

    // consultoras y escritoras de los datos
    float* getlastpos();
    
};*/

// Implementacion de las funciones basicas en C

CTrackingPFC_client* tpfcclient(const char* tname){
  return (CTrackingPFC_client*) (new TrackingPFC_client(tname));
}


void tpfccdelete(CTrackingPFC_client* c){
  TrackingPFC_client* client = (TrackingPFC_client*)c;
  delete(client);
}

float tpfccgetDisplaySizex(CTrackingPFC_client* c){
  return ((TrackingPFC_client*)c)->getDisplaySizex();
}
float tpfccgetDisplaySizey(CTrackingPFC_client* c){
  return ((TrackingPFC_client*)c)->getDisplaySizey();
}
int tpfccgetDisplayWidth(CTrackingPFC_client* c){
  return ((TrackingPFC_client*)c)->getDisplayWidth();
}
int tpfccgetDisplayHeight(CTrackingPFC_client* c){
  return ((TrackingPFC_client*)c)->getDisplayHeight();
}

const float* tpfccgetlastpos(CTrackingPFC_client* c){
  return ((TrackingPFC_client*)c)->getlastpos();
}


}