#ifndef TPFC_DEVICE_
#define TPFC_DEVICE_

#include <stdio.h>
#include <TrackingPFC_data.h>
#include <vrpn_Tracker.h>
#include <pthread.h>


#include <vector>
using namespace std;


class TPFC_device{
  private:
    // id asignada a este dispositivo
    int id; 

    // Lista de los demas dispositivos a los que hay que avisar si hay un update
    vector<TPFC_device*> listeners;

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

    // registra otro device como listener.
    void report_to(TPFC_device*);
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
    int alive(); // devuelve 1 si el estado no es STOP
    int working(); // devuelve 1 si el estado es RUN

    // función para avisar a los listeners
    void report();

    // función para activar el envio de datos via vrpn_tracker al hacer report()
    int settracker(vrpn_Connection *, const char*);

    
    
};

#endif /*TPFC_DEVICE_*/