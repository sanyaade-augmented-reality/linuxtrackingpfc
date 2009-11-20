 #ifndef TRACKINGPFC_CLIENT_
#define TRACKINGPFC_CLIENT_

#include <vrpn_Tracker.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

class TrackingPFC_client{
  public:
    // Datos de posicion (PLACEHOLDERS)
    float obsx;
    float obsy;
    float obsz;
  
    // vrpn_Tracker
    vrpn_Tracker_Remote *tracker;
    // thread que se encarga de ejecutar el mainloop del tracker
    pthread_t mainloop_thread;
    // placeholder para el callback personalizado (si es necesario)
    void (*callback_func)(TrackingPFC_client*);
    // Forzar la ejecución de mainloop (para solucionar problemas de latencia en bucles largos)
    void mainloop();


    // Creadoras y destructora
    //TrackingPFC_client(); // No hace falta, la de abajo la suple
    TrackingPFC_client(const char* tname="Tracker0@localhost", void(TrackingPFC_client*)=NULL);
    ~TrackingPFC_client();
    
    // TO DO:
    // float getDisplaySize()
    // consultoras y escritoras de los datos
    

};

// Estas 2 funciones deberian estar dentro de la clase y ademas ser PRIVATE
// pero no consigo hacer que register_change_handler y pthreads las acepten si lo hago

//  Callback que actualiza los datos
void TrackingPFC_client_callback(void *, const vrpn_TRACKERCB);
// Codigo que ejecuta el thread de mainloop
void *mainloop_executer(void * );

#endif /*TRACKINGPFC_CLIENT_*/