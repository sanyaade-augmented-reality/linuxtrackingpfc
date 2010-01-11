#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"
#include "TPFC_device_3dstereo.h"

#include <vector>

#include <iostream>
#include <fstream>

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
// (por Martin Gieseking, encontrada en http://bytes.com/topic/c/answers/132109-string-integer)
int str2int (const string &str) {
  stringstream ss(str);
  int n;
  ss >> n;
  return n;
}

// funcion auxiliar para parsear la entrada
// (encontrada en http://www.infernodevelopment.com/perfect-c-string-explode-split)
void StringExplode(string str, string separator, vector<string>* results){
    int found;
    found = str.find_first_of(separator);
    while(found != string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
}








int main( int argc, char** argv ){

    // incializaciones
    connection=NULL;
    alive = true;
    vector<TPFC_device*> dev; // lista de dispositivos

    string s; // buffer para el string la entrada
    vector<string> input; // buffer para los tonkens resultantes de procesar la entrada
    vector<string> loader; // buffer que sustituye a la entrada cuando se lee de un fichero
    vector<string>::iterator loaderit; // iterador sobre loader


    bool readinput = true; // flag que marca que debemos seguir leyendo de la entrada

    //if (argc>1){
    
    // bucle principal, lee una linea del input a no ser que se haya recibido la orden de parar
    while (alive && readinput && (loader.size()>0 || getline(cin, s)) ){
      // comprobamos si habia algo que leer en loader
      if (loader.size()>0){
	//printf("lol %i\n",loader.size());
	s=*loaderit; // actualizamos la string de entrada
	loaderit++; // incrementamos el iterador
	if (loaderit==loader.end()){ // si hemos llegado al final, borramos loader
	  loader.clear();
	}
      }
      
      // ignorar la linea si es un comentario (empieza por #) o esta vacia
      if ( (s.substr(0,1)).compare("#")==0 || s.compare("")==0){
	// No hacemos nada
      }else{
	// Hay instrucciones
	// separamos la entrada en tokens sueltos
	input.clear();
	StringExplode(s, " ", &input);

	// Nuevos devices
	if ( input[0].compare("device")==0 || input[0].compare("dev")==0){
	  if (input.size()<2){
	    printf("No se ha especificado el tipo de dispositivo a crear.\n");
	  }else

	  if ( input[1].compare("opencvfacedetect")==0 || input[1].compare("face")==0 ){
	    if (input.size()!=3){
	      printf("Device OpenCV Facedetect requiere el numero de camara.\n");
	    }else{
	      dev.push_back( new TPFC_device_opencv_face(dev.size(),str2int(input[2]) ) );
	      printf("Añadido dispositivo %i: OpenCV Facedetect\n",dev.size()-1);
	    }
	  }else

	  if ( input[1].compare("3dfrom2d")==0 || input[1].compare("3f2")==0){
	    // comprobamos que tengamos el parametro adicional necesario
	    // comprobamos que exista
	    if (input.size()!=3){
	      printf("Los dispositivos 3dfrom2d requieren el numero de id del dispositivo fuente\n");
	    }else{
	      int source = str2int( input[2] );
	      // comprobamos que sea valido
	      if (source<0 || source >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n", source);
	      }else{ //si lo es, creamos el nuevo dispositivo
		dev.push_back( new TPFC_device_3dfrom2d(dev.size(),dev[source]) );
		//((TPFC_device_3dfrom2d*)dev[1])->setdeep(TPFC_device_3dfrom2d::FIJA, 0.5);
		printf("Añadido dispositivo %i: 3dfrom2d con fuente %i\n",dev.size()-1, source);
	      }
	    }
	  }else

	  if ( input[1].compare("3dstereo")==0 || input[1].compare("stereo")==0){
	    // comprobamos que tengamos los parametro adicional necesario
	    if (input.size()!=4){
	      printf("Los dispositivos 3dstereo requieren el numero de id de los dispositivo fuente\n");
	    }else{
	      int source1 = str2int( input[2] );
	      int source2 = str2int( input[3] );
	      // comprobamos que sean validos y diferentes
	      if (source1<0 || source1 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source1);
	      }else if (source2<0 || source2 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source2);
	      }else{ //si lo son, creamos el nuevo dispositivo
		dev.push_back( new TPFC_device_3dstereo(dev.size(),dev[source1],dev[source2] ) );
		printf("Añadido dispositivo %i: 3dstereo con fuentes %i %i\n",dev.size()-1, source1, source2);
	      }
	    }
	  }else


	  if ( input[1].compare("wiimote")==0 || input[1].compare("wii")==0){
	    dev.push_back( new TPFC_device_wiimote(dev.size()) );
	    printf("Añadido dispositivo %i: Wiimote\n",dev.size()-1);


	  }else
	  
	    printf("'%s' no es un tipo de dispositivo valido.\n", input[1].c_str() );
	  
	}else
	

	// añadir tracker
	if ( input[0].compare("addtracker")==0 || input[0].compare("addt")==0){
	  // comprobamos primero que haya un device al que añadir el tracker
	  if (dev.size()==0){
	    printf("Para usar addtracker, primero se debe haber añadido un dispositivo.\n");
	  }else{
	    // comprobamos que el numero sea el esperado
	    if (input.size()==1 || input.size()>3){
	      printf("addtracker necesita uno o 2 parametros adicionales: el nombre del tracker y el numero de sensores.\n");
	    }else{
	      // si solo tenemos 2 parametros llamamos a la funcion default
	      if (input.size()==2)
		settracker(dev[dev.size()-1], input[1].c_str() );
	      else // si tenemos 3, incluimos el parametro
		settracker(dev[dev.size()-1], input[1].c_str() , str2int(input[2]) );
	      printf("Añadido tracker, con nombre '%s'\n", input[1].c_str() );
	    }
	  }
	}else
	

	// Listar dispositivos
	if ( input[0].compare("list")==0){
	  if (dev.size()==0)
	    printf("No hay dispositivos que listar.\n");
	  else
	    printf("%i dispositivos encontrados:\n", dev.size());
	  for (int i =0 ; i<dev.size();i++)
	    printf("Device %i; %s\n",i,(dev[i]->info()).c_str() );
	} else

	// cargar un archivo de configuracion
	if ( input[0].compare("load")==0){
	  
	  fstream indata; // indata is like cin
	  
	  char filename[200];
	  // formateamos el nombre del archivo
	  sprintf(filename, "%s/.trackingpfc/%s.tpfc",getenv ("HOME"),input[1].c_str());
	  indata.open(filename); // abrimos
	  if(!indata) { // Si no se puede abrir avisamos
	      printf( "No se ha podido abrir el archivo '%s'.\n", filename);
	  }else{ // si se puede...
	    string l; // string auxiliar
	    while ( !indata.eof() ) { //sigue leyendo hasta el EOF
	      getline(indata,l); // obtenemos una linea
	      loader.push_back(l); // la guardamos en el buffer 
	    }
	    indata.close();
	    loaderit = loader.begin(); // colocamos el iterador al principio del vector
	  }
	} else

	// Fin de programa
	if (s.compare("exit")==0 || s.compare("quit")==0 || s.compare("q")==0){
	  // paramos los devices
	  for (int i =0; i<dev.size();i++){
	    dev[i]->stop();
	  }
	  // desactivamos el flag para que se detenga el thread del mainloop
	  alive=false;
	}else
	// Dejar de aceptar inputs (daemon mode)
	if (s.compare("daemon")==0){
	  // bajamos el flag de read input
	  readinput=false;
	  printf("Entrando en modo daemon. El servidor ya no aceptará mas comandos. (Ctrl+c para matar el proceso)\n");
	}else
	if (s.compare("help")==0 || s.compare("?")==0 || s.compare("h")==0){
	  // si es una peticion de ayuda, imprimimos la lista de comandos
	  printf("Ayuda de TPFCServer, lista de comandos:\n");
	  printf("help (?, h) -> muestra la lista de comandos disponibles.\n");
	  printf("device (dev) <tipo> -> crea un nuevo dispositivo. los posibles tipos son:\n");
	  printf("     opencvfacedetect (face) <numero de dispositivo de video a usar>\n");
	  printf("     wiimote (wii)\n");
	  printf("     3dfrom2d (3f2) <id del dispositivo fuente>\n");
	  printf("     3dstereo (stereo) <id del 1r disp.o fuente> <id del 2o disp.o fuente>\n");
	  printf("addtracker (addt) <nombre> [numero de sensores] -> añade un tracker al ultimo dispositivo creado.\n");
	  printf("list -> lista los dispositivos configurados en el servidor.\n");
	  printf("daemon -> Pone el servidor en modo daemon (dejará de aceptar comandos).\n");
	  printf("Exit (quit, q) -> finalizar el servidor.\n");
	  printf("Si la linea empieza con '#' será considerada un comentario y por lo tanto, ignorada.\n");
	  printf("Leyenda: (alias de los comandos), <parametros obligatorios>, [<parametros opcionales>].\n");
	  printf("Se pueden encontrar ejemplos de uso en las configuraciones de ejemplo en la carpeta cfg.\n");
	  printf("\n");
	}else{ // si hemos llegado aqui sin reconocer la orden avisamos de que es incorrecta
	  printf("'%s' no es una orden valida. Escribe '?' para obtener ayuda.\n",s.c_str());
	}
      }
    }

    // si se acaba el input (por ejemplo al hacer un ./tpfcserver <cfg )
    // o al entrar en modo daemon, 
    // podemos llegar aqui con alive = true, este bucle se encarga
    // de que el servidor se siga ejecutando
    while (alive){
      // largos periodos (1 min) de sleep para no consumir cpu
      vrpn_SleepMsecs(60000);
    }

    // esperamos a que pare el mainloop (si hay servidor)
    if (connection!=NULL){
      pthread_join( mainlooper, NULL);
    }
    // finalizamos
    return 0;
}

/*
TO DO LIST:
- Quitar la chapuza del if (d->camera()==0) cvWaitKey( 10 ); en el opencv
- Ver por que mostrar 2 ventanas del facedetect acaba provocando un segfault
- eliminar el mensaje de error de vrpn tracker cuando no se envian reports en >10 secs
- añadir comandos load y save al servidor
*/