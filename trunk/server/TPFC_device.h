#ifndef TPFC_DEVICE_
#define TPFC_DEVICE_

#include <stdio.h>
#include <TrackingPFC_data.h>
#include <vrpn_Tracker.h>
#include <pthread.h>
#define TPFC_DEVICE_MAX_LISTENERS 10

class TPFC_device{
  private:
    // id asignada a este dispositivo
    int id; 

    // Lista de los demas dispositivos a los que hay que avisar si hay un update
    TPFC_device* listeners[TPFC_DEVICE_MAX_LISTENERS];
    int registered_listeners;

    // vrpn_Tracker server
    vrpn_Tracker_Server * server;
    //vrpn_Connection * connection;
    

  protected:
    // apuntador al buffer de datos
    TrackingPFC_data * data;
    // flag de funcionamiento. Estados: TPFC_RUN, TPFC_STOP, TPFC_PAUSE
    enum TPFC_device_state { STOP, RUN, PAUSE };
    TPFC_device_state running; 


    


  public:
    
    // consctructora y creadora
    TPFC_device(int ident);
    ~TPFC_device();

    // registra otro device como listener. devuelve 0 si es correcto, -1 si ha habido un error
    int report_to(TPFC_device*);
    // funcion que indica que hay nuevos datos desde uno de los inputs
    virtual void report_from(TPFC_device*);
  
    // devuelve el puntero a los datos
    TrackingPFC_data * getdata();
    // devuelve el id
    int idnum();
    
    // funciones para detener temporalmente o definitivamente los dispositivos
    void pause();
    void unpause();
    virtual void stop();
    

    // consultoras sobre el estado del funcionamiento
    int alive(); // devuelve 1 si el estado no es TPFC_STOP
    int working(); // devuelve 1 si el estado es TPFC_RUN

    // función para avisar a los listeners
    void report();

    // función para activar el envio de datos via vrpn_tracker al hacer report()
    int settracker(vrpn_Connection *, const char*);

    
    
};

#endif /*TPFC_DEVICE_*/