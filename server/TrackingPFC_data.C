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
  // aunque para el tipo de datos se guarden menos de 7 doubles, setnewdata se encarga de copiar
  // solo los necesarios
  double* defaultdata = new double[7];
  defaultdata[0]=0.0;
  defaultdata[1]=0.0;
  defaultdata[2]=0.5;
  defaultdata[3]=0.0;
  defaultdata[4]=0.0;
  defaultdata[5]=0.0;
  defaultdata[6]=1.0;
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

// consultora simple que devuelve un vector de doubles con los datos de la ultima posicion (+orientacion)
float* TrackingPFC_data::getlastpos(){
  // creamos el vector que devolveremos
  float* res= new float[dsize];
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  const double* aux= data[ind]->getdata();
  // copiamos los datos
  for (int i =0; i<dsize; i++){
    res[i]=aux[i];
  }
  pthread_mutex_unlock( lock ); // liberamos el acceso
  return res;
}
// consultora simple que devuelve un vector de doubles con los datos de la ultima posicion (+orientacion)
double* TrackingPFC_data::getlastposd(){
  // creamos el vector que devolveremos
  double* res= new double[dsize];
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  const double* aux= data[ind]->getdata();
  // copiamos los datos
  for (int i =0; i<dsize; i++){
    res[i]=aux[i];
  }
  pthread_mutex_unlock( lock ); // liberamos el acceso
  return res;
}

// consultora avanzada que devuelve una copia del ultimo datachunk
TrackingPFC_data::datachunk* TrackingPFC_data::getlastdata(){
  datachunk* res;
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // copiamos los datos
  res = new datachunk(data[ind], dsize);
  pthread_mutex_unlock( lock ); // liberamos el acceso
  //y los devolvemos
  return res;
}

// consultora avanzada que devuelve una copia del datachunk con count = c
TrackingPFC_data::datachunk* TrackingPFC_data::getdata(int c){
  if (c>count)
    return NULL;
  if (c<=count-size)
    return NULL;
  datachunk* res=NULL;
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // calculamos el indice donde estaria ese count sabiendo el actual
  int diff = count-c;
  // copiamos los datos
  res = new datachunk(data[(ind-diff+size)%size], dsize);
  pthread_mutex_unlock( lock ); // liberamos el acceso
  // comprobamos que el numero coincida
  if (res->getcount()!=c){
    printf("error al recuperar los datos %i, se han obtenido %i.\n",c,res->getcount());
    res=NULL;
  }
  //y los devolvemos
  return res;
}

// copia la orientacion de los ultimos datos (si la hay), modificando la posición.
// aunque está pensado para funcionar con datos de tamaño 3 o mas, funcionara con los de 2
// ignorando el 3r argumento
int TrackingPFC_data::setnewpos(double x, double y, double z){
  double* aux;
  // si los datos tienen orientacion, obtenemos la ultima posición conocida para poder copiarla
  if (dsize>3)
    aux = getlastposd();
  // si no, simplemente creamos un vector auxiliar (para no llamar a getlastpost, que bloquea el acceso a los datos)
  else 
    aux = new double[dsize];
  // y rellenamos el vector
  aux[0]=x;
  aux[1]=y;
  if (dsize >2)
    aux[2]=z;
  // por ultimo, guardamos esos nuevos datos
  setnewdata(aux);
  return count;
}

// añade un nuevo chunk a los datos, con tiempo actual y los datos de d
// no comprueba que los datos sean del tamaño correcto (que debe ser ==dsize)
int TrackingPFC_data::setnewdata(const float* d, bool real , int tag){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // aumentamos el indice
  ind=(ind+1)%size;
  // incrementamos el contador
  count++;
  // destruimos el datachunk anterior
  if (count>size)
    free(data[ind]);
  // creamos un nuevo vector para los datos
  double* aux = new double[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // creamos un datachunk nuevo insertandolo en el buffer
  data[ind]= new datachunk(aux, count, real, NULL, tag);
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
  return count;
}
int TrackingPFC_data::setnewdata(const double* d, bool real, int tag){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // aumentamos el indice
  ind=(ind+1)%size;
  // incrementamos el contador
  count++;
  // destruimos el datachunk anterior
  if (count>size)
    free(data[ind]);
  // creamos un nuevo vector para los datos
  double* aux = new double[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // creamos un datachunk nuevo insertandolo en el buffer
  data[ind]= new datachunk(aux, count, real, NULL, tag);
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
  return count;
}

// añade informacion sobre otro punto al ultimo report
void TrackingPFC_data::setmoredata(const float* d, bool real, int tag){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // no debemos incrementar ind ni count ya que estamos ante el mismo report.
  // creamos un nuevo vector para los datos
  double* aux = new double[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // añadimos el nuevo datachunk al report
  data[ind]->append( new datachunk(aux, count, real, NULL, tag));
  
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
}
void TrackingPFC_data::setmoredata(const double* d, bool real, int tag){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // no debemos incrementar ind ni count ya que estamos ante el mismo report.
  // creamos un nuevo vector para los datos
  double* aux = new double[dsize];
  // y lo rellenamos
  for (int i =0; i<dsize;i++)
    aux[i]=d[i];
  // añadimos el nuevo datachunk al report
  data[ind]->append( new datachunk(aux, count, real, NULL, tag));
  
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
}

// añade un nuevo chunk a los datos, vacio y con el flag de datos no validos
int TrackingPFC_data::setnodata(bool real){
  pthread_mutex_lock( lock ); // obtenemos acceso exclusivo
  // aumentamos el indice
  ind=(ind+1)%size;
  // incrementamos el contador
  count++;
  // destruimos el datachunk anterior
  if (count>size)
    free(data[ind]);
  // creamos un datachunk nuevo insertandolo en el buffer
  data[ind]= new datachunk(NULL, count, real);
  // y marcamos el flag de valid
  data[ind]->setvalid(false);
  pthread_mutex_unlock( lock ); // liberamos el acceso exclusivo
  return count;
}
// añadir tag a algun datachunk del ultimo report
void TrackingPFC_data::settag(int tag, int n){
  data[ind]->settag(tag,n);
}

// devuelve el tipo
TrackingPFC_data::TPFCdatatype TrackingPFC_data::datatype(){
  return type;
}

// devuelve el tamaño de los datos segun el tipo
int TrackingPFC_data::datasize(){
  return dsize;
}
// devuelve el count actual
int TrackingPFC_data::getcount(){
  return count;
}

// devuelve un booleano segun si los datos son reales o artificiales
bool TrackingPFC_data::isreal(){
  return data[ind]->getreal();
}



// STRUCT DATACHUNK
// constructora
TrackingPFC_data::datachunk::datachunk(double* f, int c, bool r, datachunk* n, int t){
  data= f;
  time=clock();
  count = c;
  tag=t;
  valid=true;
  real=r;
  next=n;
}
// copiadora
TrackingPFC_data::datachunk::datachunk(datachunk* d, int dsize){
  if (d->data!=NULL){
    data = new double[dsize];
    for (int i =0; i<dsize;i++)
      data[i]=d->data[i];
  }else{
    data=NULL;
  }
  time=d->time;
  tag= d->tag;
  count= d->count;
  real= d->real;
  valid= d->valid;
  if (d->next==NULL)
    next=NULL;
  else
    next= new datachunk(d->next,dsize);
}
// destructora
TrackingPFC_data::datachunk::~datachunk(){
  // si hay datos, los destruimos
  if (data!=NULL)
    free(data);
  // si hay mas datachunks en el mismo report, los destruimos
  if (next!=NULL)
    free(next);
}

// Consultoras y escritoras
const double* TrackingPFC_data::datachunk::getdata(int n){
  datachunk* aux = getchunk(n);
  return aux->data;
}

// devuelve el time
clock_t TrackingPFC_data::datachunk::gettime(int n){
  datachunk* aux = getchunk(n);
  return aux->time;
}
// devuelve el tag del punto n
int TrackingPFC_data::datachunk::gettag(int n){
  datachunk* aux = getchunk(n);
  return aux->tag;
}
// devuelve el numero de report
int TrackingPFC_data::datachunk::getcount(){
  return count;
}
//los datos son reales o artificialesç
bool TrackingPFC_data::datachunk::getreal(){
  return real;
}
bool TrackingPFC_data::datachunk::getvalid(){
  return valid;
}

int TrackingPFC_data::datachunk::size(){
  if (valid){
    datachunk* aux= this;
    int n =1;
    while (aux->next!=NULL){
      aux=aux->next;
      n++;
    }
    return n;
  }else{
    return 0;
  }
}

void TrackingPFC_data::datachunk::setvalid(bool v){
  valid=v;
}
void TrackingPFC_data::datachunk::settag(int t, int n){
  datachunk* aux = getchunk(n);
  aux->tag=t;
}
TrackingPFC_data::datachunk* TrackingPFC_data::datachunk::getchunk(int n){
  datachunk* aux= this;
  for (int i =0; i<n;i++)
    aux=aux->next;
  return aux;
}

void TrackingPFC_data::datachunk::append(datachunk* n){
  datachunk* aux= this;
  while (aux->next!=NULL){
    aux=aux->next;
  }
  aux->next=n;
}
