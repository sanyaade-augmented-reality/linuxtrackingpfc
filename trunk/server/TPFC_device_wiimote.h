#ifndef TPFC_DEVICE_WIIMOTE_
#define TPFC_DEVICE_WIIMOTE_

#include "TPFC_device.h"
#include <cwiid.h>
#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))
#define TPFC_DEVICE_WII_MAXWIIMOTES 4

class TPFC_device_wiimote : public TPFC_device{
  private:
    cwiid_wiimote_t *wiimote;	/* wiimote handle */
    /*static int* wiimoteids;
    static TPFC_device_wiimote** wiimotedevices;
    static int totalwiimotes;*/

  public:
    // consctructora y creadora
    TPFC_device_wiimote(int ident);
    ~TPFC_device_wiimote();
    
    static void err(cwiid_wiimote_t *, const char *, va_list);
    static void set_rpt_mode(cwiid_wiimote_t *, unsigned char);

};


#endif /*TPFC_DEVICE_WIIMOTE_*/