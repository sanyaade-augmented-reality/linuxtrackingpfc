#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"

#include <vector>
using namespace std;

vrpn_Connection * connection;

// funcion auxiliar para arrancar los Trackers
void settracker(TPFC_device* dev, const char* name, int nsensors = 1){
  // si es la primera vez que se llama a settracker, iniciamos la conexion
  if (connection == NULL){
    int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
    char  con_name[1024];
    sprintf(con_name, ":%d", port);
    connection = vrpn_create_server_connection(con_name, NULL, NULL);
  }
  // creamos el tracker en la conexion
  dev->settracker(connection, name, nsensors);
}

int main( int argc, char** argv ){
    
   connection=NULL;
    
    // Creamos los dispositivos del servidor
    vector<TPFC_device*> dev;

    // Facedetec
    /*dev.push_back( new TPFC_device_opencv_face(0,0) );
    dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
    settracker(dev[1], "Tracker0");*/

    // wiimote
    dev.push_back( new TPFC_device_wiimote(0) );
    dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
    settracker(dev[1], "Tracker0");

    /*dev.push_back( new TPFC_device_wiimote(0) );
    dev.push_back( new TPFC_device_wiimote(1) );
    dev.push_back( new TPFC_device_3dfrom2d(2,dev[0]) );
    settracker(dev[2], "Tracker0");
    dev.push_back( new TPFC_device_3dfrom2d(3,dev[1]) );
    settracker(dev[3], "Tracker1");*/

    
    
    

    while (1){
      if (connection!=NULL)
	connection->mainloop();
      vrpn_SleepMsecs(1);
      
    }
    //char x = getchar();

    // paramos los devices
    for (int i =0; i<dev.size();i++){
       dev[i]->stop();
    }


    
    
    return 0;
}

/*
TO DO LIST:
- Quitar la chapuza del if (d->camera()==0) cvWaitKey( 10 ); en el opencv
- Ver por que mostrar 2 ventanas del facedetect acaba provocando un segfault
*/