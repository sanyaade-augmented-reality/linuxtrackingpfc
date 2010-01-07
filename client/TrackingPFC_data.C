#include "TrackingPFC_data.h"

// Constructora
TrackingPFC_data::TrackingPFC_data(TPFCdatatype t, int s){
  lock = new pthread_mutex_t(); // inicializamos el semaforo
  type = t; // guardamos el tipo de datos
  size = s; // guardamos el tamaño del buffer

  // configuramos el tamaño de los datos segun su tipo
  if (type==TPFCDATA2D) dsize=2;
  else if (type==TPFCDATA2DSIZE) dsize=3;
  else if (type==TPFCDATA3D) dsize= 3;
  else if (type==TPFCDATA3DORI) dsize= 7;

  ind =-1;  // ponemos el indice a -1 (al rellenar la primera posición se pondrá a 0)
  count =0; // 0 datos recibidos
  data = new datachunk*[size]; // creamos el vector de apuntadores a los datachunks
  
  // ponemos los datos por defecto
  // aunque para el tipo de datos se guarden menos de 7 floats, setnewdata se encarga de copiar
  // solo los necesarios
  float* defaultdata = new float[7];
  defaultdata[0]=0.0;
  defaultdata[1]=0.0;
  defaultdata[2]=0.5;
  defaultdata[3]=0.0;
  defaultdata[4]=0.0;
  defaultdata[5]=0.0;
  defaultdata[6]=0.0;
  setnewdata(defaultdata, false);
}

// Destructora
TrackingPFC_data::~TrackingPFC_data(){
  // libreramos el espacio de memoria de los datachunks
  for (int i =0; i < size; i++)
    free(data[i]);
  // y por ultimo destruimos el vector
  free(data);
}

// consultora simple que devuelve un vector de floats con los datos de la ultima posicion (+orientacion)
float* TrackingPFC_data::getlastpos(){
  // creamos el vector que devolveremos
  float* res= new float[dsize];
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // copiamos los datos
  for (int i =0; i<dsize; i++){
    res[i]=data[ind]->data[i];
  }
  pthread_mutex_unlock( lock ); // liberamos el acceso
  return res;
}

// copia la orientacion de los ultimos datos (si la hay), modificando la posición.
// aunque está pensado para funcionar con datos de tamaño 3 o mas, funcionara con los de 2
// ignorando el 3r argumento
void TrackingPFC_data::setnewpos(float x, float y, float z){
  float* aux;
  // si los datos tienen orientacion, obtenemos la ultima posición conocida para poder copiarla
  if (dsize>3)
    aux = getlastpos();
  // si no, simplemente creamos un vector auxiliar (para no llamar a getlastpost, que bloquea el acceso a los datos)
  else 
    aux = new float[dsize];
  // y rellenamos el vector
  aux[0]=x;
  aux[1]=y;
  if (dsize >2)
    aux[2]=z;
  // por ultimo, guardamos esos nuevos datos
  setnewdata(aux);
}

// añade un nuevo chunk a los datos, con tiempo actual y los datos de d
// no comprueba que los datos sean del tamaño correcto (que debe ser ==dsize)
void TrackingPFC_data::setnewdata(float* d, bool real){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // aumentamos el indice
  ind=(ind+1)%size;
  // incrementamos el contador
  count++;
  // destruimos el datachunk anterior
  if (count>size)
    free(data[ind]);
  // creamos un nuevo vector para los datos
  float* aux = new float[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // creamos un datachunk nuevo insertandolo en el buffer
  data[ind]= new datachunk(aux, count, real);
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
}

// añade informacion sobre otro punto al ultimo report
void TrackingPFC_data::setmoredata(float* d, bool real){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // no debemos incrementar ind ni count ya que estamos ante el mismo report.
  // creamos un nuevo vector para los datos
  float* aux = new float[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // creamos un datachunk nuevo insertandolo en el buffer, el datachunk que antes ocupaba
  // data[ind] ahora esta en data[ind]->next
  data[ind]= new datachunk(aux, count, real, data[ind]);
  
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
}

// devuelve el tipo
TPFCdatatype TrackingPFC_data::datatype(){
  return type;
}

// devuelve el tamaño de los datos segun el tipo
inline int TrackingPFC_data::datasize(){
  return dsize;
}