#ifndef TPFC_DEVICE_3DFROM2D_
#define TPFC_DEVICE_3DFROM2D_

#include "TPFC_device.h"

class TPFC_device_3dfrom2d : public TPFC_device{
   
   public:
    // consctructora y creadora
    TPFC_device_3dfrom2d(int ident, TPFC_device* source);
    ~TPFC_device_3dfrom2d();
    void report_from(TPFC_device*);
};

#endif /*TPFC_DEVICE_3DFROM2D_*/