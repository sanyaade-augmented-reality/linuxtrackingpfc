#ifndef TPFC_DEVICE_3DMOD_
#define TPFC_DEVICE_3DMOD_

#include "TPFC_device.h"
#include <cv.h>

#define TPFC_CALIBSAMPLES 50
#define TPFC_CALIBINC TPFC_CALIBSAMPLES/50

class TPFC_device_3dmod : public TPFC_device{
  public:
    enum reorientopt {NONE, UNTAGGED, ALL};
    enum reorientdir {CENTER, FORWARD};

  private:
    // Puntero a los 2 device fuente
    TPFC_device* source;

    // Filtro Kalman
    vector<CvKalman*> kalman;

    // escala
    double scale;

    // reorientacion
    reorientopt orientopt;
    reorientdir orientdir;

    // reubicacion
    double* location;

    // variables auxiliares para el calibrado
    bool calibrando;
    int dots;
    int processedsamples;
    pthread_mutex_t* caliblock;
    TrackingPFC_data* calibdata;

  public:
    // consctructora y creadora
    TPFC_device_3dmod(int ident, TPFC_device*);
    ~TPFC_device_3dmod();

    // activa el filtrado de kalman
    void addkalman();

    // cambiar la escala
    void setscale(double);

    // cambiar la rotacion
    void setorientation(reorientopt o, reorientdir d=FORWARD);

    // calibrado
    double* calibrate(int d, double* c = NULL);

    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_3DMOD_*/