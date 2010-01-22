#include <stdio.h>
#include <vrpn_Tracker.h>
#include "TPFC_device.h"
#include "TPFC_device_artoolkit.h"
#include "TPFC_device_opencv_face.h"
#include "TPFC_device_wiimote.h"
#include "TPFC_device_3dfrom2d.h"
#include "TPFC_device_3dstereo.h"
#include "TPFC_device_3dmerge.h"
#include "TPFC_device_3dmod.h"
#include "TPFC_device_3dpattern.h"

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

// funcion auxiliar para pasar de str a int
inline int str2int (const string str) {
  return atoi(str.c_str());
}
// version alternativa para doubles (cuando estan expresados con comas y no con puntos)
inline double str2double (const string str) {
  vector<string> aux;
  StringExplode(str, ",", &aux);
  string aux2 = aux[0]+"."+aux[1];
  return atof(aux2.c_str());
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

    bool artcreated = false; // flag que impide crear mas de un device artoolkit

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
	    if (input.size()<3){
	      printf("Device OpenCV Facedetect requiere el numero de camara.\n");
	    }else{

	      if (input.size()==4 && input[3].compare("on")==0)
		dev.push_back( new TPFC_device_opencv_face(dev.size(),str2int(input[2]), false ) );
	      else
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
	    if (input.size()<4){
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
		  // si hay un 5o parametro, calculamos la distancia, si no dejamos la distancia
		  // por defecto para wiimotes
		  float distance = (input.size()==5)?str2int(input[4])/1000.0:0.036;
		  dev.push_back( new TPFC_device_3dstereo(dev.size(),dev[source1],dev[source2] ,distance) );
		  printf("Añadido dispositivo %i: 3dstereo con fuentes %i %i\n",dev.size()-1, source1, source2);
		  devadded = true;
		}
	      }
	    }
	  }else

	  if ( input[1].compare("artoolkit")==0 || input[1].compare("art")==0){
	    if (artcreated){
	      // solo se permite una instancia, asi que avisamos al usuario
	      printf("No se permite crear mas de un dispositivo del tipo artoolkit.\n");
	    }else{
	      // añadimos el dispositivo
	      dev.push_back( new TPFC_device_artoolkit(dev.size(), argc, argv) );
	      devadded = true; // flag de dispositivo creado 
	      artcreated=true; // flag de art creado (para no permitir mas de una instancia
	    }

	  }else


	  if ( input[1].compare("wiimote")==0 || input[1].compare("wii")==0){
	    if (input.size()==2){ // no tenemos direccion BT
	      dev.push_back( new TPFC_device_wiimote(dev.size()) );
	    }else{ // tenemos direccion BT
	      dev.push_back( new TPFC_device_wiimote(dev.size(), input[2]) );
	    }
	    string btaux = ( (TPFC_device_wiimote*)dev[dev.size()-1] )->btaddress();
	    printf("Añadido dispositivo %i: Wiimote (%s)\n",dev.size()-1, btaux.c_str());
	    devadded=true;
	    // por ultimo, si se habia llamado sin direccion, añadimos a la lista de comandos
	    // el comando que se deberia usar para volver a conectar ese wiimote
	    if (input.size()==2){
	      commands.push_back("# Version alternativa para volver a conectar el mismo mando:");
	      commands.push_back("# device wiimote "+btaux);
	    }
	  }else

	  


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


	  if ( input[1].compare("3dpattern")==0 || input[1].compare("3dpat")==0 ||
	       input[1].compare("pattern")==0 || input[1].compare("pat")==0){
	    // comprobamos que tengamos los parametro adicional necesario
	    if (input.size()!=5 && input.size()!=6){
	      printf("Los dispositivos 3dpattern requieren el numero de id del dispositivo fuente, el numero de puntos a buscar y la distancia que los separa, opcionalmente aceptan otro parametro adicional sobre si activar o no el modo autocompletar.\n");
	    }else{
	      int source = str2int( input[2] );
	      // comprobamos que el id sea valido
	      if (source<0 || source >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n", source);
	      }else{ //si lo es, comprobamos que el tipo sea valido
		string sourceok=TPFC_device_3dpattern::checksource(dev[source]);
		if (sourceok.compare("ok")!=0){
		  // si no lo es, devolvemos el error
		  printf("%s",sourceok.c_str());
		}else if ( str2int(input[3])!=2 && str2int(input[3])!=3){ // comprobamos que el numero de puntos sea valido
		  printf("El numero de puntos seleccionado no es valido, debe ser 2 o 3\n");
		}else{// si lo es, creamos el dispositivo
		  bool autocomp = (input.size()==6 && input[5].compare("off")==0)?true:false; // autocomp = on equivale a all=false, van al reves
		  dev.push_back( new TPFC_device_3dpattern(dev.size(),dev[source], str2int( input[3] ), (float)str2int( input[4] )/1000, autocomp) );
		  printf("Añadido dispositivo %i: 3dmod con fuente %i ",dev.size()-1, source);
		  printf("(%i puntos, %i mm)\n",str2int(input[3]), str2int(input[4]) );
		  devadded=true;
		}
	      }
	    }
	  }else

	  if ( input[1].compare("3dmerge")==0 || input[1].compare("merge")==0){
	    // comprobamos que tengamos los parametro adicional necesario
	    if (input.size()<4){
	      printf("Los dispositivos 3dmerge requieren el numero de id de los dispositivo fuente\n");
	    }else{
	      int source1 = str2int( input[2] );
	      int source2 = str2int( input[3] );
	      // comprobamos que los ids sean validos y diferentes
	      if (source1<0 || source1 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source1);
	      }else if (source2<0 || source2 >=dev.size()){
		printf("No se puede establecer la fuente: no existe un dispositivo con ID %i\n",source2);
	      }else{ //si lo son, comprobamos que los tipos de fuente sean correctos
		string sourceok1=TPFC_device_3dmerge::checksource(dev[source1]);
		string sourceok2=TPFC_device_3dmerge::checksource(dev[source2]);
		bool ok1=(sourceok1.compare("ok")==0);
		bool ok2=(sourceok2.compare("ok")==0);
		// si alguno no lo es, devolvemos el error
		if (!ok1){
		  printf("%s",sourceok1.c_str());
		}else if (!ok2){
		  printf("%s",sourceok2.c_str());
		}else{// si ambos son correctos, añadimos el dispositivo
		  dev.push_back( new TPFC_device_3dmerge(dev.size(),dev[source1],dev[source2] ) );
		  printf("Añadido dispositivo %i: 3dmerge con fuentes %i %i\n",dev.size()-1, source1, source2);
		  devadded = true;
		}
	      }
	    }
	  }else


	  printf("'%s' no es un tipo de dispositivo valido.\n", input[1].c_str() );
	  
	  // si hemos llegado aqui y devadded==true, es que el comando ha creado un dev
	  // lo guardamos en commands
	  if (devadded){
	    commands.push_back(s);
	  }
	  
	}else
	

	// Calibrado (3dmod)
	if ( input[0].compare("calibrate")==0 || input[0].compare("cal")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()!=2 && input.size()!=3 && input.size()!=8 && input.size()!=9 ){
	    printf("El comando calibrate requiere el numero de puntos del patron a usar (1, 2 o 3)\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==3)?str2int(input[2]):dev.size()-1;
	    auxid = (input.size()==9)?str2int(input[8]):auxid
;
	    // y comprobamos que sea del tipo adecuado como fuente
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,5)).compare("3dmod")!=0){
	      printf("No se puede aplicar calibrate al dispositivo %i, no es del tipo 3dmod\n", auxid);
	    }else {
	      // Cambiamos la opcion
	      if (input.size()<=3){ // llamada de calibrado normal
		double* aux=((TPFC_device_3dmod*)dev[auxid])->calibrate(str2int(input[1]), 0.5 );
		commands.push_back("# Version alternativa para usar los datos de este calibrado: ");
		char aux2[200];
		sprintf(aux2, "# calibrate %f %f %f %f %f %f %f", aux[0], aux[1], aux[2], aux[3], aux[4], aux[5], aux[6]);
		commands.push_back(aux2);
		free (aux);
	      }else{
		double* aux = new double[7];
		
		aux[0]=str2double(input[1]);
		aux[1]=str2double(input[2]);
		aux[2]=str2double(input[3]);
		aux[3]=str2double(input[4]);
		aux[4]=str2double(input[5]);
		aux[5]=str2double(input[6]);
		aux[6]=str2double(input[7]);
		printf("enviando %s %s %s\n", input[1].c_str(), input[2].c_str(), input[3].c_str());
		printf("enviando %f %f %f\n", aux[0], aux[1], aux[2]);
		// el numero de puntos es irrelevante cuando se llama con los datos
		((TPFC_device_3dmod*)dev[auxid])->calibrate(str2int(input[1]), 0,  aux );
		free (aux);
	      }
	      commands.push_back(s);
	      printf("Calibrado completado\n");

	    }
	  }
	}else

	// Opciones de escala (3dmod)
	if ( input[0].compare("setscale")==0 || input[0].compare("scale")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()>3 || input.size()<2){
	    printf("El comando setscale requiere como parametro el % a escalar\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==3)?str2int(input[2]):dev.size()-1;
	    // y comprobamos que sea del tipo adecuado como fuente
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,5)).compare("3dmod")!=0){
	      printf("No se puede aplicar setscale al dispositivo %i, no es del tipo 3dmod\n", auxid);
	    }else {
	      // Cambiamos la opcion
	      ((TPFC_device_3dmod*)dev[auxid])->setscale((float)str2int(input[1])/100 );
	      commands.push_back(s);
	      printf("Opcion cambiada correctamente\n");

	    }
	  }
	}else

	// Opciones de filtrado (3dmod)
	if ( input[0].compare("setfilter")==0 || input[0].compare("filter")==0|| input[0].compare("kalman")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()>2 || input.size()<1){
	    printf("El comando setfilter no requiere parametros\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==2)?str2int(input[1]):dev.size()-1;
	    // y comprobamos que sea del tipo adecuado como fuente
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,5)).compare("3dmod")!=0){
	      printf("No se puede aplicar setscale al dispositivo %i, no es del tipo 3dmod\n", auxid);
	    }else {
	      // Cambiamos la opcion
	      ((TPFC_device_3dmod*)dev[auxid])->addkalman();
	      commands.push_back(s);
	      printf("Filtro añadido correctamente\n");

	    }
	  }
	}else

	// Opciones de reorientacion (3dmod)
	if ( input[0].compare("setorientation")==0 || input[0].compare("setorient")==0 || input[0].compare("orient")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()>4 || input.size()<3){
	    printf("El comando setorientation requiere como parametros el tipo (none, all, untagged) y la direccion (forward, center).\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==4)?str2int(input[3]):dev.size()-1;
	    // si la primera opcion es "none", no se requiere 2a opcion, el 3r parametro puede ser un id
	    if (input[1].compare("none")==0 && input.size()==3)
	      auxid= str2int(input[2]);
	    // y comprobamos que sea del tipo adecuado como fuente
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,5)).compare("3dmod")!=0){
	      printf("No se puede aplicar setorientation al dispositivo %i, no es del tipo 3dmod\n", auxid);
	    }else {
	      // llamamos con la opcion adecuada:
	      if (input[1].compare("none")==0){
		((TPFC_device_3dmod*)dev[auxid])->setorientation(TPFC_device_3dmod::NONE);
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("all")==0 && input[2].compare("center")==0){
		((TPFC_device_3dmod*)dev[auxid])->setorientation(TPFC_device_3dmod::ALL, TPFC_device_3dmod::CENTER );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("all")==0 && input[2].compare("forward")==0){
		((TPFC_device_3dmod*)dev[auxid])->setorientation(TPFC_device_3dmod::ALL, TPFC_device_3dmod::FORWARD );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("untagged")==0 && input[2].compare("center")==0){
		((TPFC_device_3dmod*)dev[auxid])->setorientation(TPFC_device_3dmod::UNTAGGED, TPFC_device_3dmod::CENTER );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("untagged")==0 && input[2].compare("forward")==0){
		((TPFC_device_3dmod*)dev[auxid])->setorientation(TPFC_device_3dmod::UNTAGGED, TPFC_device_3dmod::FORWARD );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else{
		printf("Las opciones '%s %s' no son validas para setorientation, opciones posibles: <none, all, untagged> <forward, center>\n",input[1].c_str(),input[2].c_str());
	      }
	    }
	  }
	}else

	// Opciones de profundidad (3dfrom2d)
	if ( input[0].compare("setdeep")==0 || input[0].compare("deep")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()>4 || input.size()<3){
	    printf("El comando setdeep requiere como parametros el tipo (fija, rotacion, size, onlysize) y el valor de la distancia (o el parametro de size).\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==4)?str2int(input[3]):dev.size()-1;
	    // y comprobamos que sea del tipo adecuado como fuente
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,8)).compare("3dfrom2d")!=0){
	      printf("No se puede aplicar setdeep al dispositivo %i, no es del tipo 3dfrom2d\n", auxid);
	    }else {
	      // llamamos con la opcion adecuada:
	      if (input[1].compare("fija")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setdeep(TPFC_device_3dfrom2d::FIJA, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("rotacion")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setdeep(TPFC_device_3dfrom2d::ROTACION, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("size")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setdeep(TPFC_device_3dfrom2d::APROXSIZE, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("onlysize")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setdeep(TPFC_device_3dfrom2d::ONLYSIZE, (float)str2int(input[2])/1000 );
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else{
		printf("La opcion '%s' no es valida para setdeep, opciones posibles: fija, rotacion, size, onlysize\n",input[1].c_str());
	      }
	    }
	  }
	}else

	// Opciones de merge (3dfrom2d)
	if ( input[0].compare("setmerge")==0 || input[0].compare("merge")==0){
	  // comprobamos que el ultimo dispositivo sea valido
	  if (dev.size()==0){
	    printf("Aun no se ha creado ningun dispositivo, ignorando comando.\n");
	  }else
	  // comprobamos el numero de parametros
	  if (input.size()>3 || input.size()<2){
	    printf("El comando setmerge requiere un parametro (on, off), o dos (si se quiere aplicar a otro dispositivo)\n");
	  }else{
	    // Seleccionamos la id del device
	    int auxid = (input.size()==3)?str2int(input[2]):dev.size()-1;
	    // y comprobamos que sea del tipo correcto
	    string aux = dev[auxid]->info();
	    if ( (aux.substr(0,8)).compare("3dfrom2d")!=0){
	      printf("No se puede aplicar setmerge al dispositivo %i, no es del tipo 3dfrom2d\n",auxid);
	    }else {
	      // llamamos con la opcion adecuada:
	      if (input[1].compare("on")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setmerge(true);
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else if (input[1].compare("off")==0){
		((TPFC_device_3dfrom2d*)dev[auxid])->setmerge(false);
		commands.push_back(s);
		printf("Opcion cambiada correctamente\n");
	      }else{
		printf("La opcion '%s' no es valida para setmerge, opciones posibles: on, off\n",input[1].c_str());
	      }
	    }
	  }
	}else

	// añadir tracker
	if ( input[0].compare("addserver")==0 || input[0].compare("server")==0 || input[0].compare("addtracker")==0 || input[0].compare("tracker")==0){
	  // comprobamos primero que haya un device al que añadir el tracker
	  if (dev.size()==0){
	    printf("Para usar addserver, primero se debe haber añadido un dispositivo.\n");
	  }else{
	    // comprobamos que el numero sea el esperado
	    if (input.size()<2 || input.size()>4){
	      printf("addserver necesita de 1 a 3 parametros adicionales: el nombre del tracker, el numero maximo de sensores y el dispositivo en el que se instala.\n");
	    }else{
	      // calculamos la id del dispositivo segun el numero de parametros
	      int auxid = (input.size()==4)?str2int(input[3]):dev.size()-1;
	      // si solo tenemos 2 parametros llamamos a la funcion default
	      if (input.size()==2)
		settracker(dev[auxid], input[1].c_str() , 4);
	      else{ // si tenemos 3 o 4, incluimos el parametro
		settracker(dev[auxid], input[1].c_str() , str2int(input[2]) );
	      }
	      printf("Añadido servidor, con nombre '%s al dispositivo %i'\n", input[1].c_str() , auxid);
	      // añadimos el comando a commands
	      commands.push_back(s);
	    }
	  }
	}else
	
	// Listar comandos
	if ( input[0].compare("history")==0 || input[0].compare("hist")==0 ){
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
		printf( "Configuracion guardada en el archivo '%s'.\n", filename);
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
	  printf("\n");
	  printf("help (?, h) -> muestra la lista de comandos disponibles.\n");
	  printf("load <nombre> -> carga el script ~./trackingpfc/nombre.tpfc\n");
	  printf("save <nombre> -> guarda el script ~./trackingpfc/nombre.tpfc\n");
	  printf("\n");
	  printf("device (dev, d) <tipo> -> crea un nuevo dispositivo:\n");
	  printf("dev opencvfacedetect (face) <numero de dispositivo de video a usar> [multiusuario: on, off]\n");
	  printf("dev wiimote (wii)\n");
	  printf("dev artoolkit (art)\n");
	  printf("dev 3dfrom2d (3f2) <id del dispositivo fuente>\n");
	  printf("     setdeep (deep) <fija, rotacion, size, onlysize> <distancia en mm>\n");
	  printf("                    [id del dispositivo, el ultimo por defecto]-> \n");
	  printf("                    cambia la forma de calcular la profundidad\n");
	  printf("     setmerge (merge) <on, off> [id del dispositivo, el ultimo por defecto]\n");
	  printf(" 		      Activa o desactiva la opcion de juntar los puntos\n");
	  printf("dev 3dstereo (stereo) <id del 1r disp.o fuente> <id del 2o disp.o fuente>");
	  printf("                      [distancia entre sensores en mm]\n");
	  printf("dev 3dmerge (merge) <id de la fuente1> <id de la fuente2>.\n");
	  printf("dev 3dmod (mod) <id de la fuente>.\n");
	  printf("     setscale (scale) <% de escala> [id del device]\n");
	  printf("     setorientation (setorient, orient) <none, all, untagged> [center, forward]\n");
	  printf("                    [id del device]\n");
	  printf("     setfilter (filter, setkalman, kalman) [id del device]\n");
	  printf("     calibrate (cal) <numero de puntos> [id del device]\n");
	  printf("     calibrate (cal) <7 floats con la informacion del calibrado> [id del device]\n");
	  printf("dev 3dpattern (3dpat, pattern, pat) <id de la fuente> <numero de puntos>\n");
	  printf("		 <distancia entre los puntos (mm)> [autocompletar: on,off]\n");
	  printf("\n");
	  printf("addserver (server, addtracker, tracker) <nombre> [numero de sensores]\n");
	  printf("		 [id del dispositivo]-> añade un Servidor.\n");
	  printf("\n");
	  printf("list (l)-> lista los dispositivos configurados en el servidor.\n");
	  printf("history (hist) -> lista todos los comandos relevantes introducidos.\n");
	  printf("\n");
	  printf("pause (p) -> Pone en pausa los dispositivos creados hasta el momento.\n");
	  printf("unpause (u, run, r) -> quita la pausa.\n");
	  printf("Exit (quit, q) -> finalizar el servidor.\n");
	  printf("daemon -> Pone el servidor en modo daemon (dejará de aceptar comandos).\n");
	  printf("\n");
	  printf("Si la linea empieza con '#' será considerada un comentario y por lo tanto, ignorada.\n");
	  printf("Leyenda: (alias de los comandos), <parametros obligatorios>, [parametros opcionales].\n");
	  printf("Todos los comandos que tienen como parametro opcional una id de dispositivo, se aplicaran por defecto al ultimo dispositivo creado.\n");
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
