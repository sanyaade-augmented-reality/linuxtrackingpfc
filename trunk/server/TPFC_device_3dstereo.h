#ifndef TPFC_DEVICE_3DSTEREO_
#define TPFC_DEVICE_3DSTEREO_

#include "TPFC_device.h"
#define TPFCPI 3.14159265

class TPFC_device_3dstereo : public TPFC_device{
   private:
    // Puntero a los 2 device fuente
    TPFC_device** sources;
    // los datachunks del ultimo report de cada fuente
    TrackingPFC_data::datachunk** lastdata;
    // semaforo para exclusion mutua en el acceso a lastdata
    pthread_mutex_t* lock;

    int left; // indice del sensor izquierdo

    // funcion que encuentra el indice interno de la fuente (si esta en sources[0] o sources[1]
    int getsourcepos(TPFC_device* s);
    // funcion que convierte el angulo del device fuente apuntado por sources[2o parametro]
    // a un angulo usable para coordenar (el angulo interior del triangulo usado)
    double angleconversion(double, int);
    

    // variables usadas en la calibracion
    bool calibrated; // flag de calibrado
    double* calib_data; // buffer que se usa durante el calibrado. los datos estan almacenados
		       // en tramos de calib_sample longitud, en este orden:
		       // x_wm1_dot1, y_wm1_dot1, x_wm2_dot1, y_wm2_dot1,
		       // x_wm1_dot2, y_wm1_dot2, x_wm2_dot2, y_wm2_dot2... 
    int calib_count; // total de puntos almacenados en calib_data (indice)
    pthread_mutex_t* calib_lock;
    void addsample(double*); // funcion para añadir los datos de una muestra al bufer
    double* getsamples(int, int, int); // funcion para recuperar todos los samples del buffer

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