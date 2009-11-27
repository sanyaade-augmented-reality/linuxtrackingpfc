 #ifndef TRACKINGPFC_DATA_
#define TRACKINGPFC_DATA_

#include <vrpn_Tracker.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <GL/glut.h>
#define RADFACTOR 3.14159265/180.0

class TrackingPFC_data{
  private:
    // Datos de posicion (PLACEHOLDERS, esto mas tarde será un tipo de datos complejo)
    float obsx;
    float obsy;
    float obsz;
  public:  
    TrackingPFC_data();
    ~TrackingPFC_data();

};
#endif /*TRACKINGPFC_DATA_*/