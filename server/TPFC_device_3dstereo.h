#ifndef TPFC_DEVICE_3DSTEREO_
#define TPFC_DEVICE_3DSTEREO_

#include "TPFC_device.h"

class TPFC_device_3dstereo : public TPFC_device{
   private:
    TPFC_device** sources;

    bool calibrated; // flag de calibrado
    void calibrate(); // funcion para calibrar
    float* calib_data; // buffer que se usa durante el calibrado
    int calib_samples; // numero de muestras en cada punto para calibrar
    int calib_count; // total de puntos almacenados en calib_data (indice)
    int calib_dots; // numero de puntos simultaneos durante el calibrado
    pthread_mutex_t* calib_lock;
    void adddot(float,float,float,float,int);

    public:
    // consctructora y creadora
    TPFC_device_3dstereo(int ident, TPFC_device*,TPFC_device*);
    ~TPFC_device_3dstereo();
    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);
    void nullreport_from(TPFC_device*);
    
    // funcion que devuelve en un string la información relativa al dispositivo
    string info();
};

#endif /*TPFC_DEVICE_3DSTEREO_*/