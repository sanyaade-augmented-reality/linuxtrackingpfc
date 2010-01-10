#ifndef TPFC_DEVICE_3DSTEREO_
#define TPFC_DEVICE_3DSTEREO_

#include "TPFC_device.h"

class TPFC_device_3dstereo : public TPFC_device{
   private:
    TPFC_device** sources;
    TrackingPFC_data::datachunk** lastdata;
    pthread_mutex_t* lock;

    // funcion que encuentra el indice interno de la fuente (si esta en sources[0] o sources[1]
    int getsourcepos(TPFC_device* s);
    
    bool calibrated; // flag de calibrado
    float* calib_data; // buffer que se usa durante el calibrado. los datos estan almacenados
		       // en tramos de calib_sample longitud, en este orden:
		       // x_wm1_dot1, y_wm1_dot1, x_wm2_dot1, y_wm2_dot1,
		       // x_wm1_dot2, y_wm1_dot2, x_wm2_dot2, y_wm2_dot2... 
    int calib_count; // total de puntos almacenados en calib_data (indice)
    pthread_mutex_t* calib_lock;
    void addsample(float*); // funcion para añadir los datos de una muestra al bufer
    float* getsamples(int, int, int); // funcion para recuperar todos los samples del buffer

    public:
    // consctructora y creadora
    TPFC_device_3dstereo(int ident, TPFC_device*,TPFC_device*);
    ~TPFC_device_3dstereo();

    // configuracion de la calibracion
    int calib_samples; // numero de muestras en cada punto para calibrar
    int calib_dots; // numero de puntos simultaneos durante el calibrado
    //int calib_discard; // % de las muestras a descartar al procesar los datos
    void calibrate(); // funcion para calibrar

    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);
    void nullreport_from(TPFC_device*);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();
};

#endif /*TPFC_DEVICE_3DSTEREO_*/