#ifndef TPFC_DEVICE_3DMOD_
#define TPFC_DEVICE_3DMOD_

#include "TPFC_device.h"


class TPFC_device_3dmod : public TPFC_device{
  private:
    // Puntero a los 2 device fuente
    TPFC_device* source;

  public:
    // consctructora y creadora
    TPFC_device_3dmod(int ident, TPFC_device*);
    ~TPFC_device_3dmod();

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