#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"

#include <vector>
using namespace std;

vrpn_Connection * connection;
bool alive;
pthread_t mainlooper;

// thread que se encarga del mainloop del vrpn
void* mainloop_thread(void* ){
  while (alive){
    if (connection!=NULL){
      connection->mainloop();
      vrpn_SleepMsecs(1); 
    }else{
      vrpn_SleepMsecs(1000); 
    }
  }
}

// funcion auxiliar para arrancar los Trackers
void settracker(TPFC_device* dev, const char* name, int nsensors = 1){
  // si es la primera vez que se llama a settracker, iniciamos la conexion
  if (connection == NULL){
    int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
    char  con_name[1024];
    sprintf(con_name, ":%d", port);
    connection = vrpn_create_server_connection(con_name, NULL, NULL);
    // lanzamos el thread del mainloop
    pthread_create( &mainlooper, NULL, mainloop_thread ,NULL);
  }
  // creamos el tracker en la conexion
  dev->settracker(connection, name, nsensors);
}

int main( int argc, char** argv ){
    // incializaciones
    connection=NULL;
    alive = true;

    // Creamos los dispositivos del servidor
    vector<TPFC_device*> dev;

    // Facedetec
    dev.push_back( new TPFC_device_opencv_face(0,0) );
    dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
    settracker(dev[1], "Tracker0");

    // wiimote
    /*dev.push_back( new TPFC_device_wiimote(0) );
    dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
    settracker(dev[1], "Tracker0",4);*/

    /*dev.push_back( new TPFC_device_wiimote(0) );
    dev.push_back( new TPFC_device_wiimote(1) );
    dev.push_back( new TPFC_device_3dfrom2d(2,dev[0]) );
    settracker(dev[2], "Tracker0");
    dev.push_back( new TPFC_device_3dfrom2d(3,dev[1]) );
    settracker(dev[3], "Tracker1");*/

    // placeholder para el bucle principal
    char x = getchar();

    // paramos los devices
    for (int i =0; i<dev.size();i++){
       dev[i]->stop();
    }
    // desactivamos el flag para que se detenga el thread del mainloop
    alive=false;
    // esperamos a que pare el mainloop (si hay servidor)
    if (connection!=NULL)
      pthread_join( mainlooper, NULL);
    // finalizamos
    return 0;
}

/*
TO DO LIST:
- Quitar la chapuza del if (d->camera()==0) cvWaitKey( 10 ); en el opencv
- Ver por que mostrar 2 ventanas del facedetect acaba provocando un segfault
*/