#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"

#include <vector>

#include <iostream>
//#include <string>

using namespace std;

vrpn_Connection * connection;
bool alive;
pthread_t mainlooper;

// thread que se encarga del mainloop del vrpn
void* mainloop_thread(void* ){
  // el bucle se ejecuta mientras el flag de alive no este a falso
  while (alive){
    if (connection!=NULL){
      connection->mainloop();
      // sleep para no consumir cpu innecesariamente
      vrpn_SleepMsecs(1); 
    }else{
      // si aun no hay conexión, hacemos un sleep mas largo
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
    vector<TPFC_device*> dev; // lista de dispositivos

    string s; // buffer de las ordenes recibidas
    bool loaded = false; // flag, a cierto si se carga 

    // bucle principal, lee una linea del input a no ser que se haya recibido la orden de parar
    while (alive && getline(cin, s) ){
      
      // Fin de programa
      if (s.compare("exit")==0 || s.compare("quit")==0 || s.compare("q")==0){
	// paramos los devices
	for (int i =0; i<dev.size();i++){
	  dev[i]->stop();
	}
	// desactivamos el flag para que se detenga el thread del mainloop
	alive=false;
      }else
      

      // Presets que cargan tipos comunes de server
      if (s.compare("face")==0){
	if (loaded){
	  printf("Solo se puede cargar un archivo o una configuración predeterminada. Orden ignorada\n");
	}else{
	  // Facedetec
	  dev.push_back( new TPFC_device_opencv_face(0,0) );
	  dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
	  settracker(dev[1], "Tracker0");
	  //((TPFC_device_3dfrom2d*)dev[1])->setdeep(TPFC_device_3dfrom2d::FIJA, 0.5);
	  loaded=true;
	}
      }else

      if (s.compare("wii")==0){
	if (loaded){
	  printf("Solo se puede cargar un archivo o una configuración predeterminada. Orden ignorada\n");
	}else{
	  // wiimote
	  dev.push_back( new TPFC_device_wiimote(0) );
	  dev.push_back( new TPFC_device_3dfrom2d(1,dev[0]) );
	  settracker(dev[1], "Tracker0",4);
	  loaded=true;
	}
      }else

      if (s.compare("wii2")==0){
	if (loaded){
	  printf("Solo se puede cargar un archivo o una configuración predeterminada. Orden ignorada\n");
	}else{
	  // wiimote x2
	  dev.push_back( new TPFC_device_wiimote(0) );
	  dev.push_back( new TPFC_device_wiimote(1) );
	  dev.push_back( new TPFC_device_3dfrom2d(2,dev[0]) );
	  settracker(dev[2], "Tracker0",4);
	  dev.push_back( new TPFC_device_3dfrom2d(3,dev[1]) );
	  settracker(dev[3], "Tracker1",4);
	  loaded= true;
	}
      }else
      

      // Si hemos llegado a este punto o es una orden vacia, o petición de ayuda o una orden incorrecta
      if (s.compare("")!=0){ // si no es una orden vacia
	if (s.compare("help")==0 || s.compare("?")==0) // si es una peticion de ayuda
	  printf("Ayuda de TPFCServer, lista de ordenes:\n");
	else // si es una orden incorrecta
	  printf("Orden no reconocida, lista de ordenes:\n");
	// sea por petición de ayuda o por orden incorrecta, mostramos la ayuda
	printf("Face, wii, wii2 -> cargar uno de los presets predeterminados\n");
	printf("Exit, quit, q -> salir\n\n");
      }

    }

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
- eliminar el mensaje de error de vrpn tracker cuando no se envian reports en >10 secs
*/