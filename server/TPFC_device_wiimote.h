#ifndef TPFC_DEVICE_WIIMOTE_
#define TPFC_DEVICE_WIIMOTE_

#include "TPFC_device.h"
#include <cwiid.h>
#include <functional>
#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

class TPFC_device_wiimote : public TPFC_device{
  private:
    // wiimote handle 
    cwiid_wiimote_t *wiimote;
    bdaddr_t bdaddr;// direccion BT del wiimote
    
    // estructura que contiene la información de los wiimotes
    // para poder usar mas de 1 (el callback no sabe a que device esta
    // asociado, asi que ha de recuperar un puntero al device a partir
    // de la id del wiimote
    struct wiimoteinfo{
      int id;
      TPFC_device_wiimote* dev;
      // creadora
      wiimoteinfo(int i, TPFC_device_wiimote* d){id=i; dev=d;};
    };
    // vector que guarda la información de los wiimotes
    static vector<wiimoteinfo> wiimotes;

    // Función que registra los wiimotes por su id, 
    static void registerwiimote(cwiid_wiimote_t *, TPFC_device_wiimote* );
    // Funcion que recupera un puntero al dev wiimote dada su id
    static TPFC_device_wiimote* getwiimotedev(cwiid_wiimote_t *wiimote);


    // funciones auxiliares de cwiid para configurar el report y suprimir errores
    static void err(cwiid_wiimote_t *, const char *, va_list);
    static void set_rpt_mode(cwiid_wiimote_t *, unsigned char);


  public:
    // consctructora y creadora
    TPFC_device_wiimote(int ident, string bta="");
    ~TPFC_device_wiimote();

    // callback del wiimote
    static void callback(cwiid_wiimote_t *, int , union cwiid_mesg mesg[], struct timespec *);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que devuelve en un string la direccion Bluetooth del wiimote
    string btaddress();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
    
};


#endif /*TPFC_DEVICE_WIIMOTE_*/