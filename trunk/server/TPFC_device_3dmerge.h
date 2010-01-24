#ifndef TPFC_DEVICE_3DMERGE_
#define TPFC_DEVICE_3DMERGE_

#include "TPFC_device.h"

class TPFC_device_3dmerge : public TPFC_device{

  private:
    TPFC_device* sources[2];

    TrackingPFC_data * datad[2]; // bufer de datas para modo different

    pthread_mutex_t* lock; // semaforo para la exclusión mutua

    // flag para saber si hay que diferenciar las fuentes por sensores
    // si esta a cierto solo se aceptará un punto por sensor
    // y se guardara un punto para cada sensor
    bool different; 

  public:
    // consctructora y creadora
    TPFC_device_3dmerge(int ident, TPFC_device*, TPFC_device*);
    ~TPFC_device_3dmerge();

    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);

    // cambiar el modo 
    void setdifferent (bool);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_3DMERGE_*/