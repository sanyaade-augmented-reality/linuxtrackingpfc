 #ifndef TRACKINGPFC_CLIENT_
#define TRACKINGPFC_CLIENT_

#include <vrpn_Tracker.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <GL/glut.h>

#define RADFACTOR 3.14159265/180.0

#include <iostream>
#include <fstream>
#include <sstream>

# include <X11/Xlib.h>

#include <vector>
using namespace std;

class TrackingPFC_client{
  private:
    // datos recibidos
    vector<float*> data;
    vector<timeval> time;
    struct timezone tz;
  
    int alive; // indicador de si el thrad de mainloop debe seguir funcionando 1=si, 2=no

    pthread_mutex_t* lock; // semaforo para la exclusión mutua para que 2 callbacks no escriban a la vez

    float mdl2scr; // ratio de la escala modelo / mundo real (o pantalla)
    // datos para ajustes si la camara original no estaba pensada para HT
    float originalfov; // fov original de la aplicación(si lo tenia)
    float aspectratio; // aspect ratio de la ultima llamada a gluPerspective
    float zadjustment; // ajuste en el eje z necesario si la aplicacion original tenia fov

    // vrpn_Tracker
    vrpn_Tracker_Remote *tracker;
    // thread que se encarga de ejecutar el mainloop del tracker
    pthread_t mainloop_thread;

    // Codigo que ejecuta el thread de mainloop
    static void *mainloop_executer(void * );
    //  Callback que actualiza los datos
    static void TrackingPFC_client_callback(void *, const vrpn_TRACKERCB);
    
    // funcion auxiliar para getDisplaySizex() y getDisplaySizey()
    static float getDisplaySize(int);

    // variables donde guardar el tamaño de la pantalla, para no estar leyendo continuamente del disco
    static float screensizex;
    static float screensizey;

    // funcion auxiliar para partir strings
    static void StringExplode(string str, string separator, vector<string>* results);
    // pasar de str a int
    static int str2int (const string &str);
    // calcular la diferencia entre 2 timevals (devuelve en secs, con precision de microsecs
    static double diff(struct timeval * x,struct timeval * y);

  public:
    // placeholder para el callback personalizado (si es necesario)
    void (*callback_func)(TrackingPFC_client*);
    // Forzar la ejecución de mainloop (para solucionar problemas de latencia en bucles largos)
    void mainloop();

    // Creadoras y destructora
    //TrackingPFC_client(); // No hace falta, la de abajo la suple
    TrackingPFC_client(const char* tname="Tracker0@localhost", void(TrackingPFC_client*)=NULL);
    ~TrackingPFC_client();
    
    // Devuelve el tamaño de la pantalla (en metros), leida del archivo de configuracion
    static float getDisplaySizex();
    static float getDisplaySizey();
    // Devuelve el tamaño de la pantalla en pixeles (leido del servidor X)
    static int getDisplayWidth();
    static int getDisplayHeight();
    // Devuelve el tamaño de la pantalla en metros (leido del servidor X)
    static float getDisplayWidthMM();
    static float getDisplayHeightMM();

    // consultoras y escritoras de los datos
    float* getlastpos(int sensor=0);
    float getlasttime(int sensor=0);
    void setdata(float*, int);
    int isalive();

    // numero de sensores (puede aumentar con el tiempo)
    int sensors();
    
    // configurar el tamaño o la distancia de la pantalla virtual
    void setvirtualdisplaysize(float);
    void setvirtualdisplaydistance(float);

    // devuelve la escala por la que hay que multiplicar las posiciones reales
    // para obtener la posicion en la escena
    float getscale();
    
    // llamadas glut y GL modificadas para usar tracking
    void htgluLookAt(float,float,float,  float,float,float,  float,float,float);
    void htgluPerspective(float, float, float, float, int x = getDisplayWidth(), int y = getDisplayHeight());
    void htadjustPerspective(float, float, int x = getDisplayWidth(), int y = getDisplayHeight()); // como la anterior, pero sin fov ni aspect ratio originales

    // TO DO && BRAINSTORM
    // pause/unpause/togglepause (que se quede con la ultima posicion)
    // enable tracking/disable tracking (en casos donde hay un original fov, que LookAt y Perspective pasen a "modo transparente"

};



#endif /*TRACKINGPFC_CLIENT_*/