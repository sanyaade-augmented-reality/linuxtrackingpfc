#include "TPFC_device_3dstereo.h" 


TPFC_device_3dstereo::TPFC_device_3dstereo(int ident, TPFC_device* s1, TPFC_device* s2):TPFC_device(ident){

  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3D);

  lock = new pthread_mutex_t(); // inicializamos el semaforo

  // guardamos un puntero a las fuentes
  sources= new TPFC_device*[2];
  sources[0]= s1;
  sources[1]= s2;
  // y creamos un vector donde guardar los reports
  lastdata = new TrackingPFC_data::datachunk*[2];
  lastdata[0]=NULL;
  lastdata[1]=NULL;

  // marcamos a falso el flag de calibrado
  calibrated = false;
  // este dispositivo estara en pausa (ignorando los reports) hasta que no empiece la calibración
  running=PAUSE;

  // registramos este dispositivo en la lista de listeners que vamos a usar como input
  s1-> report_to(this);
  s2-> report_to(this);

  // calibramos
  calibrate();
}

TPFC_device_3dstereo::~TPFC_device_3dstereo(){
  free(data);
  if (lastdata[0]!=NULL);
    free(lastdata[0]);
  if (lastdata[1]!=NULL);
    free(lastdata[1]);
}

// funcion auxiliar para comparar 2 floats, para usar con qsort
int comparefloats (const void * a, const void * b)
{
  if (*(float*)a == *(float*)b)
    return 0;
  else if  (*(float*)a < *(float*)b)
    return -1;
  else
    return 1;
}


void TPFC_device_3dstereo::calibrate(){
  calib_samples=500; // de 50 a 500
  calib_dots=2; // de 1 a 3
  int progressinc=calib_samples/50;// cada cuantos samples es un 2%
  // numero total de puntos usados en el calibrado
  // si estamos usando las gafas u otro aparato con 2 marcadores
  // al realizar la toma tendremos 4 puntos
  // si usamos 3 marcadores o 1 solo marcador, solo tendremos 3, que son los minimos necesarios
  int totaldots=(calib_dots==2)?4:3;

  calib_lock = new pthread_mutex_t(); // inicializamos el semaforo
  // creamos el buffer de datos
  calib_data = (float*)malloc(calib_samples*calib_dots*4*sizeof(float));

  // y declaramos la matriz donde guardaremos temporalmente los datos para el calibrado
  float angles[totaldots][2][2];
  int fase = 0; // conatador de fases en las que se divide el calibrado

  while (fase*calib_dots<3){
     // inicializamos el indice
    calib_count=0;
    // Mensajes de aviso con cuenta atrás para el usuario
    printf("Preparando para obtener datos en...\n");
    for (int i =5; i>0;i--){
      printf("%i ",i);
      fflush(stdout);
      vrpn_SleepMsecs(1000);
    }
    /*printf("5 ");fflush(stdout);
    vrpn_SleepMsecs(1000);
    printf("4 ");fflush(stdout);
    vrpn_SleepMsecs(1000);
    printf("3 ");fflush(stdout);
    vrpn_SleepMsecs(1000);
    printf("2 ");fflush(stdout);
    vrpn_SleepMsecs(1000);
    printf("1 ");fflush(stdout);
    vrpn_SleepMsecs(1000);*/
    printf("Adquiriendo datos\n");
    // activamos el flag de running para que report_from empiece a recojer datos
    running=RUN;
    int progress=progressinc;
    // este bucle espera a que se llene el buffer de datos
    // si no se llena (los sensores no van), la aplicacion se quedará en un bucle infinito
    // mientras funcione, aunque sea lentamente, irá imprimiendo un punto por cada 2% de buffer llenado
    while (calib_count < calib_samples){
      // sleep para no ocupar la cpu
      vrpn_SleepMsecs(1);
      // comprobamos que no haya que escribir un nuevo punto
      if (calib_count>=progress){
	// actualizamos el contador donde hay que escribir un nuevo punto
	progress+=progressinc;
	printf(".");
	// flush para asegurar que el punto se escribe al momento
	fflush(stdout);
      }
    }
    // pausamos la recogida de datos para ahorrar cpu
    running=PAUSE;
    printf("\nDatos obtenidos, procesando...\n");

    float* samp;
    float acum, max, min, med;
    // recorremos el buffer obteniendo todos los datos relativos a cada conjunto <punto, sensor, orientacion>
    for (int d=0; d<calib_dots;d++){
      for (int sn=0; sn<2;sn++){
	for (int xy=0; xy<2;xy++){
	  // obtenemos el buffer correspondiente
	  samp=getsamples(xy, sn, d);
	  // reseteamos los contadores
	  acum=0.0;
	  max=samp[0];
	  min=samp[0];
	  // y recorremos los buffers sumando el valor al acumulador y actualizando max y min
	  for (int i =0; i<calib_samples;i++){
	    acum+=samp[i];
	    if (samp[i]<min)
	      min=samp[i];
	    if (samp[i]>max)
	      max=samp[i];
	  }
	  // por ultimo calculamos la media
	  med=acum/calib_samples;
	  // y la guardamos en la matriz de angulos
	  angles[d+fase*calib_dots][sn][xy]=med;
	}
      }
    }
  fase++;// incrementamos el contador de fase
  }// while (fase*calib_dots<3){

  printf("Calculando posición de los sensores\n");
  // llegados a este punto tenemos angles[3..4][2][2] llena, con
  // los puntos ordenados de mayor a menos grado horizontal, y de mayor
  // a menor vertical en caso de empate cuando tenemos 3 puntos
  // o en caso de tener 4, los 2 primeros son los 2 superiores
  
  
  
  printf("Calibración finalizada\n");
  calibrated = true;
  free(calib_data);
  running=RUN;
}


// funcion para añadir los datos de una muestra al buffer
//en d recibimos los datos en orden: d1(x1, y1, x2, y2), d2(x1, y1, x2, y2)...
void TPFC_device_3dstereo::addsample(float* d){
  pthread_mutex_lock( calib_lock ); // obtenemos acceso exclusivo
  // comprobamos que no este lleno el buffer
  if (calib_count<calib_samples){
    // version que no comprueba la ordenacion antes de insertar
    /*for (int i =0;i<calib_dots;i++){
      calib_data[calib_samples*4*i+calib_count]=d[i*4];
      calib_data[calib_samples*4*i+calib_samples+calib_count]=d[i*4+1];
      calib_data[calib_samples*4*i+calib_samples*2+calib_count]=d[i*4+2];
      calib_data[calib_samples*4*i+calib_samples*3+calib_count]=d[i*4+3];
    }*/
    
    //version que ordena
    float* aux; // puntero auxiliar para escribir en el buffer
    float dots[calib_dots][2]; // matriz auxiliar donde guardaremos temporalmente los puntos
    int dm =0; // desplazamiento en d debido al sensor que queramos
    for (int sn =0; sn<2; sn++){ // cada sensor individualmente
      // primero cargamos todos los puntos en dots[][]
      for (int dn=0; dn<calib_dots;dn++){
	dots[dn][0]=d[dn*4+dm];
	dots[dn][1]=d[dn*4+dm+1];
      }
      // si se esta calibrando con 2 o 3 puntos necesitaremos ordenarlos
      // esta ordenación sera por el angulo horizontal (x), de mayor a menor
      // y en caso de empate irá primero aquel con un angulo vertical mayor
      if (calib_dots>1){
	// ya que como maximo tenemos 2 o 3 puntos, usar condicionales será mas simple
	// que usar algoritmos mas complejos
	if (dots[1][0]>dots[0][0]){
	  int temp;
	  temp=dots[0][0];
	  dots[0][0]=dots[1][0];
	  dots[1][0]=temp;
	}
	if (calib_dots==3){
	  if (dots[2][0]>dots[0][0]){
	    int temp;
	    temp=dots[0][0];
	    dots[0][0]=dots[2][0];
	    dots[2][0]=temp;
	  }
	  if (dots[2][0]>dots[1][0]){
	    int temp;
	    temp=dots[1][0];
	    dots[1][0]=dots[2][0];
	    dots[2][0]=temp;
	  }
	}
      }
      // guardamos los dots en el buffer
      for (int dn=0; dn<calib_dots;dn++){
	aux=getsamples(0,sn,dn);
	aux[calib_count]=dots[dn][0];
	aux=getsamples(1,sn,dn);
	aux[calib_count]=dots[dn][1];
      }
      dm=2;// incrementamos antes de salir el desplazamiento necesario para acceder a los datos del sensor correcto
    }
    // fin de la version que ordena

      
    calib_count++;
  }
  pthread_mutex_unlock( calib_lock ); // liberamos el acceso exclusivo
}

// funcion para recuperar todas las muestras del bufer
// no comprueba que el buffer este lleno
// devuelve un puntero al buffer, no a una copia
float* TPFC_device_3dstereo::getsamples(int xy, int sn, int dot){
  // devolvemos simplemente el puntero a la posicion necesaria
  // al usarlo hay que tener cuidado de no pasar del rango 0..calib_samples-1
  return &calib_data[calib_samples*4*dot+calib_samples*(2*sn+xy)];
}

// devuelve el indice interno de la fuente (si esta en sources[0] o sources[1]
int TPFC_device_3dstereo::getsourcepos(TPFC_device* s){
  if (s==sources[0])
    return 0;
  else
    return 1;
}

void TPFC_device_3dstereo::report_from(TPFC_device* s){
  // comprobamos que no estemos en pausa
  if (working()){
    // guardamos estos datos en su buffer
    int sn = getsourcepos(s);
    if (lastdata[sn]!=NULL){
      // si habia datos, los eliminamos antes
      free(lastdata[sn]);
      lastdata[sn]=NULL;
    }
    lastdata[sn]= (s->getdata())->getlastdata();

    // comprobamos si los datos son validos

    if (calibrated){ // si ya esta el dispositivo calibrado


/*// obtenemos los datos
if (sourcedata->getvalid() == false){
  // si no son validos guardamos un chunk no valido en nuestros datos y damos un nullreport
  data->setnodata();
  nullreport();
}else{// si son validos...
  // obtenemos el numero de puntos del report
  int n = sourcedata->size();
  const float* aux;

  if (merge){ // Hay que unificar todos los datos
    // acumuladores
    float acumx=0;
    float acumy=0;
    // recorremos los puntos sumando los datos
    for (int i =0; i<n; i++){
      aux = sourcedata->getdata(i);
      acumx+=aux[0];
      acumy+=aux[1];
    }
    // obtenemos la media
    acumx=acumx/n;
    acumy=acumy/n;
    // guardamos los datos
    setdata(acumx, acumy);
  }else{ // Los datos se envian como sensores separados
    // obtenemos el primer punto
    aux = sourcedata->getdata();
    // y lo guardamos en un nuevo report
    setdata(aux[0], aux[1]);
    // recorremos los siguientes puntos (si los hay)
    for (int i =1; i<n; i++){
      // obtenemos y guardamos los datos de los puntos, pero añadiendolos al report existente
      aux = sourcedata->getdata(i);
      setdata(aux[0], aux[1], false);
    }
  }
  // sea cual sea la opción, una vez escritos los datos, reportamos
  report();
}*/




    }else{ // si no esta calibrado (lo estamos haciendo)
      // si hay datos, son validos y tienen el numero adecuado de punto, los pasamos al buffer de calibrado
      if (lastdata[0]!=NULL && lastdata[0]->getvalid() && lastdata[0]->size()==calib_dots &&
	  lastdata[1]!=NULL && lastdata[1]->getvalid() && lastdata[1]->size()==calib_dots){
	// creamos un vector auxiliar
	float* aux = new float[4*calib_dots];
	for (int i =0; i<calib_dots;i++){
	  aux[i*4+0]=(lastdata[0]->getdata(i))[0];
	  aux[i*4+1]=(lastdata[0]->getdata(i))[1];
	  aux[i*4+2]=(lastdata[1]->getdata(i))[0];
	  aux[i*4+3]=(lastdata[1]->getdata(i))[1];
	}
	addsample(aux);
	// liberamos la memoria del vector
	free(aux);
      }
    }// if calibrated
  }// if workikg
}

void TPFC_device_3dstereo::nullreport_from(TPFC_device* s){
  // redirigimos al report normal (ya se gestiona internamente la validez)
  report_from(s);
}

// Informacion sobre el dispositivo
string TPFC_device_3dstereo::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "3dstereo (usando de fuente los dispositivo %i y %i)", sources[0]->idnum(), sources[1]->idnum());
  return aux;
}