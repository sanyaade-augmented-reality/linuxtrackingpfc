#ifndef TPFC_DEVICE_ARTOOLKIT_
#define TPFC_DEVICE_ARTOOLKIT_

#include "TPFC_device.h"

class TPFC_device_artoolkit : public TPFC_device{

  private:

  public:
    // consctructora y creadora
    TPFC_device_artoolkit(int ident);
    ~TPFC_device_artoolkit();

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_AARTOOLKIT_*/