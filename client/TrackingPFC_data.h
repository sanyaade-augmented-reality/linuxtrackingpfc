 #ifndef TRACKINGPFC_DATA_
#define TRACKINGPFC_DATA_

#include <pthread.h>
#include <stdlib.h>
enum TPFCdatatype { TPFCDATA2D, TPFCDATA2DSIZE, TPFCDATA3D, TPFCDATA3DORI};

class TrackingPFC_data{
  private:
    // tipo de datos almacenados y tamaño del buffer
    TPFCdatatype type;
    int size;
    int dsize;
    
    // Datos almacenados
    float *data; 
    // indices que controlan el bufer
    int ind; // indice que apunta al primer dato valido del buffer
    int count; // total de datos recibidos

    float obsx;
    float obsy;
    float obsz;

    pthread_mutex_t* lock;

  public:  
    TrackingPFC_data(TPFCdatatype type=TPFCDATA3DORI, int size=1);
    ~TrackingPFC_data();

    //devuelve el tipo de datos almacenado
    int datatype();
    // devuelve el tamaño de los datos que se guardan (de cuanto es el vector que se devuelve)
    int datasize();

    float* getlastpos();
    void setnewpos(float, float, float);
    void setnewdata(float* d);

};
#endif /*TRACKINGPFC_DATA_*/