#include "TrackingPFC_data.h"
TrackingPFC_data::TrackingPFC_data(){
  obsx=0;
  obsy=0;
  obsz=0.5;// para que si no hay tracker podemos ver algo, asumimos que no tenemos pegada la nariz a la pantalla
}

// Destructora
TrackingPFC_data::~TrackingPFC_data(){
}