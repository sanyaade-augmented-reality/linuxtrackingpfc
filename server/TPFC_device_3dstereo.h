#ifndef TPFC_DEVICE_3DSTEREO_
#define TPFC_DEVICE_3DSTEREO_

#include "TPFC_device.h"

class TPFC_device_3dstereo : public TPFC_device{
   private:
    TPFC_device** sources;
    
    public:
    // consctructora y creadora
    TPFC_device_3dstereo(int ident, TPFC_device*,TPFC_device*);
    ~TPFC_device_3dstereo();
    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);
    
    // funcion que devuelve en un string la información relativa al dispositivo
    string info();
};

#endif /*TPFC_DEVICE_3DSTEREO_*/