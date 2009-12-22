#ifndef TPFC_DEVICE_WIIMOTE_
#define TPFC_DEVICE_WIIMOTE_

#include "TPFC_device.h"

class TPFC_device_wiimote : public TPFC_device{
  private:
    pthread_t wiimote_thread; // thread que se encarga de procesar los datos del wiimote


  public:
    // consctructora y creadora
    TPFC_device_wiimote(int ident);
    ~TPFC_device_wiimote(); 
    void stop();

};


#endif /*TPFC_DEVICE_WIIMOTE_*/