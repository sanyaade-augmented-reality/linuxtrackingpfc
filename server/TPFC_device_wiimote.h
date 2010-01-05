#ifndef TPFC_DEVICE_WIIMOTE_
#define TPFC_DEVICE_WIIMOTE_

#include "TPFC_device.h"
#include <cwiid.h>
#include <functional>
#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))
#define TPFC_DEVICE_WII_MAXWIIMOTES 4

class TPFC_device_wiimote : public TPFC_device{
  private:
    cwiid_wiimote_t *wiimote;	/* wiimote handle */
    static int* wiimoteids;
    static TPFC_device_wiimote** wiimotedevices;
    static int totalwiimotes;

  public:
    // consctructora y creadora
    TPFC_device_wiimote(int ident);
    ~TPFC_device_wiimote();

    static void wiimote_callback(cwiid_wiimote_t *, int , union cwiid_mesg mesg[], struct timespec *);
    
    static void err(cwiid_wiimote_t *, const char *, va_list);
    static void set_rpt_mode(cwiid_wiimote_t *, unsigned char);

    static void registerwiimote(cwiid_wiimote_t *, TPFC_device_wiimote* );
    static TPFC_device_wiimote* getwiimotedev(cwiid_wiimote_t *wiimote);

};


#endif /*TPFC_DEVICE_WIIMOTE_*/