 #ifndef TRACKINGPFC_CLIENT_
#define TRACKINGPFC_CLIENT_

#include <vrpn_Tracker.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <GL/glut.h>
#define TPFCCORD_DEFAULT 0
#define TPFCCORD_GLC 1
#define RADFACTOR 3.14159265/180.0

class TrackingPFC_client{
  public:
    // Datos de posicion (PLACEHOLDERS, esto mas tarde será un tipo de datos complejo)
    float obsx;
    float obsy;
    float obsz;
  
    int alive; // indicador de si el thrad de mainloop debe seguir funcionando 1=si, 2=no

    float virtualdisplaysize; // indicador del tamaño de la pantalla virtual
    float originalfov;
    float zadjustment;
    float aspectratio;

  
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
    
    // Devuelve el tamaño de la pantalla (horizontal)
    float getDisplaySizex();
    //float getDisplaySizey();
    //float getDisplayRatio();

    // consultoras y escritoras de los datos
    float getlastposx();
    float getlastposy();
    float getlastposz();
    void setlastposx(float);
    void setlastposy(float);
    void setlastposz(float);
    
    void setvirtualdisplaysize(float);
    
    // llamadas glut y GL modificadas para usar tracking
    void htgluLookAt(float,float,float,  float,float,float,  float,float,float);
    void htgluPerspective(float, float, float, float);
    void htadjustPerspective(float, float, float); // como la anterior, pero sin fov

    // TO DO && BRAINSTORM
    // pause/unpause/togglepause (que se quede con la ultima posicion)
    // enable tracking/disable tracking (en casos donde hay un original fov, que LookAt y Perspective pasen a "modo transparente"
    

};

// Estas 2 funciones deberian estar dentro de la clase y ademas ser PRIVATE
// pero no consigo hacer que register_change_handler y pthreads las acepten si lo hago

//  Callback que actualiza los datos
void TrackingPFC_client_callback(void *, const vrpn_TRACKERCB);
// Codigo que ejecuta el thread de mainloop
void *mainloop_executer(void * );

#endif /*TRACKINGPFC_CLIENT_*/