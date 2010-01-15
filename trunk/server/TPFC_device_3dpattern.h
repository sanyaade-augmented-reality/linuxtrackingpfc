#ifndef TPFC_DEVICE_3DPATTERN_
#define TPFC_DEVICE_3DPATTERN_

#include "TPFC_device.h"


class TPFC_device_3dpattern : public TPFC_device{
  public:
    enum keepothersoptions {NEVER, WITHPATTERN, ALWAYS};
  private:
    // Puntero a los 2 device fuente
    TPFC_device* source;

    int dots; // puntos a buscar
    float dist; // distancia entre puntos
    int tag; // tag a usar
    bool all; // se requieren todos los puntos para dar el report?
    keepothersoptions keepothers; // incluir los puntos que no pertenecen al patron
    float tolerance; // % de error aceptado
    double* lastpattern; // numero de dato que contiene el ultimo pattern registrado

    // funcion auxiliar que calcula la distancia entre 2 puntos
    double dotdist(const double*,const double*);

  public:
    // consctructora y creadora
    TPFC_device_3dpattern(int ident, TPFC_device* s, int dot, float dis, bool al = false, int t = 1,keepothersoptions others = WITHPATTERN);
    ~TPFC_device_3dpattern();

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

#endif /*TPFC_DEVICE_3DPATTERN_*/