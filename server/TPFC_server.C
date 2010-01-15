#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"
#include "TPFC_device_3dstereo.h"
#include "TPFC_device_3dmod.h"

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

// funcion auxiliar para comprobar si un archivo existe
// (encontrada en http://www.techbytes.ca/techbyte103.html)
#include <sys/stat.h>

bool FileExists(string strFilename) {
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }
  
  return(blnReturn);
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
    vector<string> commands; // buffer que guarda los comandos introducidos hasta el momento


    bool readinput = true; // flag que marca que debemos seguir leyendo de la entrada

    // comprobamos si teniamos como parametro un nombre de archivo
    if (argc>1){
      // en vez de abrir el archivo, cargamos la instruccion "load <archivo>" en loader
      // para que el load lo haga el bucle principal
      s="load ";
      s.append(argv[1]);
      loader.push_back(s); // cargamos en loader
      loaderit=loader.begin(); // inicializamos el iterador
    }
    
    // bucle principal, lee una linea del input a no ser que se haya recibido la orden de parar
    // si es necesario, imprimimos ">" para que el usuario sepa que esperamos su entrada
      if (alive && loader.size()==0){
	printf(">");
	fflush(stdout);
      }
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
	if ( input[0].compare("device")==0 || input[0].compare("dev")==0|| input[0].compare("d")==0){
	  bool devadded=false; //flag de dispositivo añadido
	  if (input.size()<2){
	    printf("No se ha especificado el tipo de dispositivo a crear.\n");
	  }else

	  if ( input[1].compare("opencvfacedetect")==0 || input[1].compare("face")==0 ){
	    if (input.size()!=3){
	      printf("Device OpenCV Facedetect requiere el numero de camara.\n");
	    }else{
	      dev.push_back( new TPFC_device_opencv_face(dev.size(),str2int(input[2]) ) );
	      printf("Añadido dispositivo %i: OpenCV Facedetect\n",dev.size()-1);
	      devadded=true;
	    }
	  }else

	  if ( input[1].compare("3dfrom2d")==0 || input[1].compare("3f2")==0){
	    // comprobamos que tengamos el parametro adicional necesario
	    if (input.size()!=3){
	      printf("Los dispositivos 3dfrom2d requieren el numero de id del dispositivo fuente\n");
	    }else{
	      int source = str2int( input[2] );
	      // comprobamos que el id sea valido
	      if (source<0 || source >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n", source);
	      }else{ //si lo es, comprobamos que el tipo sea valido
		string sourceok=TPFC_device_3dfrom2d::checksource(dev[source]);
		if (sourceok.compare("ok")!=0){
		  // si no lo es, devolvemos el error
		  printf("%s",sourceok.c_str());
		}else{// si lo es, creamos el dispositivo
		  dev.push_back( new TPFC_device_3dfrom2d(dev.size(),dev[source]) );
		  printf("Añadido dispositivo %i: 3dfrom2d con fuente %i\n",dev.size()-1, source);
		  devadded=true;
		}
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
	      // comprobamos que los ids sean validos y diferentes
	      if (source1<0 || source1 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source1);
	      }else if (source2<0 || source2 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source2);
	      }else{ //si lo son, comprobamos que los tipos de fuente sean correctos
		string sourceok1=TPFC_device_3dstereo::checksource(dev[source1]);
		string sourceok2=TPFC_device_3dstereo::checksource(dev[source2]);
		bool ok1=(sourceok1.compare("ok")==0);
		bool ok2=(sourceok2.compare("ok")==0);
		// si alguno no lo es, devolvemos el error
		if (!ok1){
		  printf("%s",sourceok1.c_str());
		}else if (!ok2){
		  printf("%s",sourceok2.c_str());
		}else{// si ambos son correctos, añadimos el dispositivo
		  dev.push_back( new TPFC_device_3dstereo(dev.size(),dev[source1],dev[source2] ) );
		  printf("Añadido dispositivo %i: 3dstereo con fuentes %i %i\n",dev.size()-1, source1, source2);
		  devadded = true;
		}
	      }
	    }
	  }else


	  if ( input[1].compare("wiimote")==0 || input[1].compare("wii")==0){
	    dev.push_back( new TPFC_device_wiimote(dev.size()) );
	    printf("Añadido dispositivo %i: Wiimote\n",dev.size()-1);
	    devadded=true;
	  }else{


	  if ( input[1].compare("3dmod")==0 || input[1].compare("mod")==0){
	    // comprobamos que tengamos el parametro adicional necesario
	    if (input.size()!=3){
	      printf("Los dispositivos 3dmod requieren el numero de id del dispositivo fuente\n");
	    }else{
	      int source = str2int( input[2] );
	      // comprobamos que el id sea valido
	      if (source<0 || source >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n", source);
	      }else{ //si lo es, comprobamos que el tipo sea valido
		string sourceok=TPFC_device_3dmod::checksource(dev[source]);
		if (sourceok.compare("ok")!=0){
		  // si no lo es, devolvemos el error
		  printf("%s",sourceok.c_str());
		}else{// si lo es, creamos el dispositivo
		  dev.push_back( new TPFC_device_3dmod(dev.size(),dev[source]) );
		  printf("Añadido dispositivo %i: 3dmod con fuente %i\n",dev.size()-1, source);
		  devadded=true;
		}
	      }
	    }
	  }else


	    printf("'%s' no es un tipo de dispositivo valido.\n", input[1].c_str() );
	  }
	  // si hemos llegado aqui y devadded==true, es que el comando ha creado un dev
	  // lo guardamos en commands
	  if (devadded){
	    commands.push_back(s);
	  }
	  
	}else
	

	// Opciones de profundidad
	if ( input[0].compare("setdeep")==0 || input[0].compare("deep")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()!=3){
	    printf("El comando setdeep requiere como parametros el tipo (fija, rotacion, size, onlysize) y el valor de la distancia (o el parametro de size).\n");
	  }else{
	    string aux = dev[dev.size()-1]->info();
	    if ( (aux.substr(0,8)).compare("3dfrom2d")!=0){
	      printf("No se puede aplicar setdeep al ultimo dispositivo, no es del tipo 3dfrom2d\n");
	    }else {
	      // llamamos con la opcion adecuada:
	      if (input[1].compare("fija")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setdeep(TPFC_device_3dfrom2d::FIJA, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("rotacion")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setdeep(TPFC_device_3dfrom2d::ROTACION, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("size")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setdeep(TPFC_device_3dfrom2d::APROXSIZE, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("onlysize")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setdeep(TPFC_device_3dfrom2d::ONLYSIZE, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else{
		printf("La opcion '%s' no es valida para setdeep, opciones posibles: fija, rotacion, size, onlysize\n",input[1].c_str());
	      }
	    }
	  }
	}else

	// Opciones de merge
	if ( input[0].compare("setmerge")==0 || input[0].compare("merge")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()!=2){
	    printf("El comando setmerge requiere un parametro (on, off).\n");
	  }else{
	    string aux = dev[dev.size()-1]->info();
	    if ( (aux.substr(0,8)).compare("3dfrom2d")!=0){
	      printf("No se puede aplicar setmerge al ultimo dispositivo, no es del tipo 3dfrom2d\n");
	    }else {
	      // llamamos con la opcion adecuada:
	      if (input[1].compare("on")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setmerge(true);
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("off")==0){
		((TPFC_device_3dfrom2d*)dev[dev.size()-1])->setmerge(false);
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else{
		printf("La opcion '%s' no es valida para setmerge, opciones posibles: on, off\n",input[1].c_str());
	      }
	    }
	  }
	}else

	// añadir tracker
	if ( input[0].compare("addtracker")==0 || input[0].compare("addt")==0){
	  // comprobamos primero que haya un device al que añadir el tracker
	  if (dev.size()==0){
	    printf("Para usar addtracker, primero se debe haber añadido un dispositivo.\n");
	  }else{
	    // comprobamos que el numero sea el esperado
	    if (input.size()==1 || input.size()>3){
	      printf("addtracker necesita 1 o 2 parametros adicionales: el nombre del tracker y el numero de sensores.\n");
	    }else{
	      // si solo tenemos 2 parametros llamamos a la funcion default
	      if (input.size()==2)
		settracker(dev[dev.size()-1], input[1].c_str() );
	      else // si tenemos 3, incluimos el parametro
		settracker(dev[dev.size()-1], input[1].c_str() , str2int(input[2]) );
	      printf("Añadido tracker, con nombre '%s'\n", input[1].c_str() );
	      // añadimos el comando a commands
	      commands.push_back(s);
	    }
	  }
	}else
	
	// Listar comandos
	if ( input[0].compare("commands")==0 || input[0].compare("c")==0){
	  if (commands.size()==0)
	    printf("No se han introducido comandos relevantes validos por el momento.\n");
	  else
	    printf("%i comandos introducidos:\n", commands.size());
	  for (int i =0 ; i<commands.size();i++){
	    printf("%s\n",commands[i].c_str() );
	  }

	} else

	// Listar dispositivos
	if ( input[0].compare("list")==0 ||input[0].compare("l")==0){
	  if (dev.size()==0)
	    printf("No hay dispositivos que listar.\n");
	  else
	    printf("%i dispositivos encontrados:\n", dev.size());
	  for (int i =0 ; i<dev.size();i++)
	    printf("Device %i; %s\n",i,(dev[i]->info()).c_str() );
	} else

	// cargar un archivo de configuracion
	if ( input[0].compare("load")==0){
	  if (dev.size()>0){
	    printf("No se pueden cargar archivos una vez se ha creado algun dispositivo.\n");
	  }else if (input.size()==1){
	    printf("El comando load requiere el nombre de archivo a abrir.\n");
	  }else{
	    fstream indata;
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
	  }
	} else

	// Guardar un archivo de configuracion
	if ( input[0].compare("save")==0){
	  if (input.size()==1){
	    printf("El comando save requiere el nombre con el que quieres guardar la configuracion\n");
	  }else if (commands.size()==0){
	    printf("No se han introducido comandos relevantes validos por el momento, No hay nada que guardar\n");
	  }else{
	    ofstream outdata; // indata is like cin
	    char filename[200];
	    // formateamos el nombre del archivo
	    sprintf(filename, "%s/.trackingpfc/%s.tpfc",getenv ("HOME"),input[1].c_str());
	    bool overwrite = true; // flag de escritura
	    if (FileExists(filename)){
	      printf( "Ya existe el archivo '%s'. Deseas sobreescribirlo? (y/n)\n", filename);
	      char r='a';
	      while (r!='y' && r != 'n')
		r=getchar();
	      if (r=='n')
		overwrite=false;
	    }
	    if (overwrite){// si vamos a escribir el archivo
	      outdata.open(filename); // abrimos
	      if(!outdata) { // Si no se puede abrir avisamos
		  printf( "No se ha podido abrir el archivo '%s'.\n", filename);
	      }else{
		for (int i =0; i<commands.size();i++){
		  outdata << commands[i];
		  outdata << "\n";
		}
		outdata.close();
		printf( "Configuracion guardada el archivo '%s'.\n", filename);
	      }
	    }
	    
	  }
	} else

	// Pause
	if (s.compare("pause")==0 || s.compare("p")==0){
	  // pausamos los device
	  for (int i =0; i<dev.size();i++){
	    dev[i]->pause();
	  }
	}else

	// UnPause
	if (s.compare("unpause")==0 || s.compare("u")==0 || s.compare("run")==0 || s.compare("r")==0){
	  // pausamos los device
	  for (int i =0; i<dev.size();i++){
	    dev[i]->unpause();
	  }
	}else

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
	  printf("load <nombre> -> carga el script ~./trackingpfc/nombre.tpfc");
	  printf("save <nombre> -> guarda el script ~./trackingpfc/nombre.tpfc");
	  printf("device (dev, d) <tipo> -> crea un nuevo dispositivo. los posibles tipos son:\n");
	  printf("dev opencvfacedetect (face) <numero de dispositivo de video a usar>\n");
	  printf("dev wiimote (wii)\n");
	  printf("dev 3dfrom2d (3f2) <id del dispositivo fuente>\n");
	  printf("     setdeep (deep) <fija, rotacion, size, onlysize> <distancia en mm> -> cambia la forma de calcular la profundidad\n");
	  printf("     setmerge (merge) <on, off> -> Activa o desactiva la opcion de juntar los puntos\n");
	  printf("dev 3dstereo (stereo) <id del 1r disp.o fuente> <id del 2o disp.o fuente>\n");
	  printf("dev 3dmod (mod) <id de la fuente>.\n");
	  printf("addtracker (addt) <nombre> [numero de sensores] -> añade un tracker al ultimo dispositivo creado.\n");
	  printf("list (l)-> lista los dispositivos configurados en el servidor.\n");
	  printf("daemon -> Pone el servidor en modo daemon (dejará de aceptar comandos).\n");
	  printf("commands (c) -> lista todos los comandos relevantes realizados hasta el momento.\n");
	  printf("pause (p) -> Pone en pausa los dispositivos creados hasta el momento.\n");
	  printf("unpause (u, run, r) -> quita la pausa (pone en funcionamiento) todos los devices creados hasta el momento\n");
	  printf("Exit (quit, q) -> finalizar el servidor.\n");
	  printf("Si la linea empieza con '#' será considerada un comentario y por lo tanto, ignorada.\n");
	  printf("Leyenda: (alias de los comandos), <parametros obligatorios>, [<parametros opcionales>].\n");
	  printf("Se pueden encontrar ejemplos de uso en las configuraciones de ejemplo en la carpeta cfg.\n");
	  printf("\n");
	}else{ // si hemos llegado aqui sin reconocer la orden avisamos de que es incorrecta
	  printf("'%s' no es una orden valida. Escribe '?' para obtener ayuda.\n",s.c_str());
	}
      }
      // si es necesario, imprimimos ">" para que el usuario sepa que esperamos su entrada
      if (alive && loader.size()==0){
	printf(">");
	fflush(stdout);
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