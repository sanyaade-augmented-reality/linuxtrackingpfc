 #ifndef TRACKINGPFC_DATA_
#define TRACKINGPFC_DATA_

#include <pthread.h>

class TrackingPFC_data{
  private:
    // Datos de posicion (PLACEHOLDERS, esto mas tarde será un tipo de datos complejo)
    float obsx;
    float obsy;
    float obsz;

    pthread_mutex_t* lock;

  public:  
    TrackingPFC_data(float x =0.0, float y = 0.0, float z = 0.5);
    ~TrackingPFC_data();

  float* getlastpos();
  void setnewpos(float, float, float);

};
#endif /*TRACKINGPFC_DATA_*/