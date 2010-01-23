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
    // semaforo para exclusion mutua al escribir los datos en data
    pthread_mutex_t* lock2;

    double camdist; // distancia entre ambos sensores
    int left; // indice del sensor izquierdo
    int fails; // contador de fallos consecutivos al encontrar el lado correcto

    // funcion que encuentra el indice interno de la fuente (si esta en sources[0] o sources[1]
    int getsourcepos(TPFC_device* s);
    // funcion que convierte el angulo del device fuente apuntado por sources[2o parametro]
    // a un angulo usable para coordenar (el angulo interior del triangulo usado)
    double angleconversion(double, int);
    
    public:
    // consctructora y creadora
    TPFC_device_3dstereo(int ident, TPFC_device*,TPFC_device*, double dist = 0.036);
    ~TPFC_device_3dstereo();

    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);
    void nullreport_from(TPFC_device*);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_3DSTEREO_*/