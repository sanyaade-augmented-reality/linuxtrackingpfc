 #ifndef TRACKINGPFC_CLIENT_
#define TRACKINGPFC_CLIENT_

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <GL/glut.h>
#include "TrackingPFC_data.h"
#define RADFACTOR 3.14159265/180.0

#include <vector>
using namespace std;

class TrackingPFC_client{
  public:
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
    float* getlastpos();
    void setdata(float*, int);
    int isalive();
    
    void setvirtualdisplaysize(float);
    void setvirtualdisplaydistance(float);
    
    // llamadas glut y GL modificadas para usar tracking
    void htgluLookAt(float,float,float,  float,float,float,  float,float,float);
    void htgluPerspective(float, float, float, float);
    void htadjustPerspective(float, float, float); // como la anterior, pero sin fov

    // TO DO && BRAINSTORM
    // pause/unpause/togglepause (que se quede con la ultima posicion)
    // enable tracking/disable tracking (en casos donde hay un original fov, que LookAt y Perspective pasen a "modo transparente"

};



#endif /*TRACKINGPFC_CLIENT_*/