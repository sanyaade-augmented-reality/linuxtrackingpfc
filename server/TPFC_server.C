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

// funcion auxiliar para pasar de str a int
int str2int (const string &str) {
  stringstream ss(str);
  int n;
  ss >> n;
  return n;
}

int main( int argc, char** argv ){
    // incializaciones
    connection=NULL;
    alive = true;
    vector<TPFC_device*> dev; // lista de dispositivos

    string s; // buffer para la entrada
    
    // bucle principal, lee una linea del input a no ser que se haya recibido la orden de parar
    while (alive && getline(cin, s) ){
      
      // ignorar la linea si es un comentario (empieza por #) o esta vacia
      if ( (s.substr(0,1)).compare("#")==0 || s.compare("")==0){
	// No hacemos nada
      } else
      

      // Nuevos devices
      if ( (s.substr(0,7)).compare("device ")==0 ){
	if ( (s.substr(7,16)).compare("opencvfacedetect")==0 ){
	  dev.push_back( new TPFC_device_opencv_face(dev.size(),0) );
	  printf("Añadido dispositivo %i: OpenCV Facedetect\n",dev.size()-1);

	}else if ( (s.substr(7,8)).compare("3dfrom2d")==0 ){
	  // comprobamos que tengamos el parametro adicional necesario
	  // comprobamos que exista
	  if (s.size()<17 || (s.substr(15,1)).compare(" ")!=0){
	    printf("Los dispositivos 3dfrom2d requieren el numero de id del dispositivo fuente\n");
	  }else{
	    int source = str2int( s.substr(16,s.size()-16) );
	    // comprobamos que sea valido
	    if (source<0 || source >=dev.size()){
	      printf("No se puede establecer la fuente: no existe un dispositivo con esa ID\n");
	    }else{ //si lo es, creamos el nuevo dispositivo
	      dev.push_back( new TPFC_device_3dfrom2d(dev.size(),dev[0]) );
	      //((TPFC_device_3dfrom2d*)dev[1])->setdeep(TPFC_device_3dfrom2d::FIJA, 0.5);
	      printf("Añadido dispositivo %i: 3dfrom2d con fuente %i\n",dev.size()-1, source);
	    }
	  }

	}else{
	  printf("'%s' no es un tipo de dispositivo valido.\n", (s.substr(7,s.size()-7)).c_str() );
	} 
      }else
      
      // añadir tracker
      if ( (s.substr(0,11)).compare("addtracker ")==0 ){
	settracker(dev[dev.size()-1], (s.substr(11,s.size()-11)).c_str() );
	printf("Añadido tracker, con nombre '%s'\n", (s.substr(11,s.size()-11)).c_str() );
      }else
      

      /* 
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
      }else*/


      // Fin de programa
      if (s.compare("exit")==0 || s.compare("quit")==0 || s.compare("q")==0){
	// paramos los devices
	for (int i =0; i<dev.size();i++){
	  dev[i]->stop();
	}
	// desactivamos el flag para que se detenga el thread del mainloop
	alive=false;
      }else if (s.compare("help")==0 || s.compare("?")==0 || s.compare("h")==0){
	// si es una peticion de ayuda, imprimimos la lista de comandos
	printf("Ayuda de TPFCServer, lista de comandos:\n");
	printf("Si la linea empieza con '#' será considerada un comentario y por lo tanto, ignorada.\n");
	printf("help, ?, h -> muestra la lista de comandos disponibles.\n");
	printf("Exit, quit, q -> finalizar el servidor.\n\n");
      }else{ // si hemos llegado aqui sin reconocer la orden avisamos de que es incorrecta
	printf("'%s' no es una orden valida. Escribe '?' para obtener ayuda.\n",s.c_str());
      }
      
    }

    // si se acaba el input (por ejemplo al hacer un ./tpfcserver <cfg
    // podemos llegar aqui con alive = true, este bucle se encarga
    // de que el servidor se siga ejecutando
    while (alive){
      // largos periodos (1 min) de sleep para no consumir cpu
      vrpn_SleepMsecs(60000);
    }

    // esperamos a que pare el mainloop (si hay servidor)
    if (connection!=NULL){
      printf("saliendo\n");
      pthread_join( mainlooper, NULL);
      printf("saliendo\n");
    }
    // finalizamos
    return 0;
}

/*
TO DO LIST:
- Quitar la chapuza del if (d->camera()==0) cvWaitKey( 10 ); en el opencv
- Ver por que mostrar 2 ventanas del facedetect acaba provocando un segfault
- eliminar el mensaje de error de vrpn tracker cuando no se envian reports en >10 secs
*/