#ifndef TPFC_DEVICE_
#define TPFC_DEVICE_

#include <stdio.h>
#include <string>
#include "TrackingPFC_data.h"
#include <vrpn_Tracker.h>

#include <pthread.h>

#include <vector>
using namespace std;

#include <quat.h>

// para poder leer del archivo de configuracion
#include <iostream>
#include <fstream>
#include <sstream>
// para file exist
#include <sys/stat.h>

class TPFC_device{
  private:
    // id asignada a este dispositivo
    int id; 
    // Lista de los demas dispositivos a los que hay que avisar si hay un update
    vector<TPFC_device*> listeners;
    // vrpn_Tracker server
    vrpn_Tracker_Server * server;
    int sensors; // numero maximo de sensores que reportará el tracker

  protected:
    // apuntador al buffer de datos
    TrackingPFC_data * data;
    // flag de funcionamiento
    enum { STOP, RUN, PAUSE } running;

    // funcion auxiliar para leer opciones del archivo de configuracion
    // devuelve si se ha encontrado o no los datos
    // target es el nombre de la opcion a buscar
    // res el puntero al string resultado, no se modifica si se devuelve false
    static bool getconfig(string target, string* res);
    // funcion auxiliar para partir strings
    static void StringExplode(string str, string separator, vector<string>* results);
    // funcion auxiliar para comprobar si existen los archivos
    static bool FileExists(string strFilename);

  public:
    // consctructora y creadora
    TPFC_device(int ident);
    ~TPFC_device();
    // registra otro device como listener.
    void report_to(TPFC_device*);
    // funcion de aviso de nuevos reports desde los inputs
    virtual void report_from(TPFC_device*);
    virtual void nullreport_from(TPFC_device*);
    // devuelve el puntero a los datos
    TrackingPFC_data * getdata();
    // devuelve el id del dispositivo
    int idnum();
    // funciones para detener temporalmente o definitivamente los dispositivos
    void pause();
    void unpause();
    virtual void stop();
    // consultoras sobre el estado del funcionamiento
    bool alive(); // devuelve 1 si el estado no es STOP
    bool working(); // devuelve 1 si el estado es RUN
    // función para avisar a los listeners de que hay nuevos datos
    void report(bool onlytagged=false);
    // función para avisar a los listeners de que no nuevos datos en este report
    void nullreport();
    
    // función para activar el envio de datos via vrpn_tracker al hacer report()
    int settracker(vrpn_Connection *, const char*, int nsensors=1);

    // funcion que devuelve en un string la información relativa al dispositivo
    virtual string info() =0;
    
    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_*/