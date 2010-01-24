 #ifndef TRACKINGPFC_DATA_
#define TRACKINGPFC_DATA_

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

class TrackingPFC_data{
  
  public:
    // tipo de datos que se guardan en los buffers
    enum TPFCdatatype { TPFCDATA2D, TPFCDATA2DSIZE, TPFCDATA3D, TPFCDATA3DORI};
    // struct basico de la información relativa a un punto
    struct datachunk{
      private:
      double* data; // double[] que guarda los datos de posicion y orientacion
      clock_t time; // time
      int tag; // int reservado para que se puedan guardar estados
      int count; // numero de report (para facilitar la identificación de los datachunks);
      bool real; // flag de si es un reporte real o un dato ficticio (como por ejemplo la posicion inicial)
		 // asi se pueden ignorar los datos ficticios a la hora de trabajar con los datos
      bool valid; // flag de validez de los datos
      datachunk* next; // puntero al siguiente datachunk del mismo report (para reports con mas de 1 punto)
      
      public:
      // constructora
      datachunk(double* f, int c, bool r = true, datachunk* n=NULL);
      // copiadora
      datachunk(datachunk* d, int dsize);
      // destructora
      ~datachunk();
      
      // consultoras y escritoras
      // las que tienen un argumento es porque son individuales de cada punto
      // las que no lo tienen es porque deberian ser identicas en todos los puntos
      const double* getdata(int n =0); // devuelve los datos del punto n 
      clock_t gettime(int n =0); // devuelve el time
      int gettag(int n = 0); // devuelve el tag del punto n
      int getcount(); // devuelve el numero de report
      bool getreal(); // los datos son reales o artificiales
      bool getvalid(); // los datos son validos, o el report es vacio
      int size();  // cantidad de puntos en el report (si no es valido devuelve 0)
      void setvalid(bool);
      void settag(int,int n = 0);
      void append(datachunk*); // añade un nuevo datachunk

      private:
      // auxiliares
      datachunk* getchunk(int); // obtiene el punto n desde el primero.
      
    };

  private:
    // tipo de datos almacenados y tamaño del buffer
    TPFCdatatype type; // tipo
    int size;	// cantidad de reports del buffer
    int dsize;	// tamaño del vector de datos (relativos a posición y orientacion)
    
    // Datos almacenados
    datachunk** data; //vector de apuntadores a datachunk, debe tener tamaño size
    // indices que controlan el bufer
    int ind; // indice que apunta al primer dato valido del buffer
    int count; // total de datos recibidos

    pthread_mutex_t* lock; // semaforo para la exclusión mutua

  public:
    // constructora y destructora
    TrackingPFC_data(TPFCdatatype type=TPFCDATA3DORI, int size=1);
    ~TrackingPFC_data();

    //devuelve el tipo de datos almacenado
    TPFCdatatype datatype();
    // devuelve el tamaño de los datos que se guardan (de cuanto es el vector que se devuelve)
    int datasize();

    // consultoras y escritoras simples (para reports con 1 solo punto por report)
    // devuelve el vector de floats de posicion/orientacion de la ultima posicion reportada
    float* getlastpos();
    double* getlastposd();
    // añade una nueva posición (copiando la orientación de la anterior posicion, si la hay)
    // funciona tb para datos 2d, simplemente ignora el 3r parametro
    int setnewpos(double, double, double f = 0.0);
    // añade un nuevo report con los datos de d, d debe tener tamaño dsize
    int setnewdata(const float* d, bool real = true);
    int setnewdata(const double* d, bool real = true);

    // Escritoras avanzadas
    // añadir información de otro punto adicional al ultimo report
    void setmoredata(const float* d, bool real = true);
    void setmoredata(const double* d, bool real = true);
    // añadir un nuevo report vacio y con el flag de datos no validos
    int setnodata(bool real = true);
    // añadir tag a algun datachunk del ultimo report
    void settag(int tag, int n=0);

    // obtiene el count actual
    int getcount();
    // obtiene el count actual
    bool isreal();

    // Consultoras avanzadas
    // devuelve una copia del ultimo datachunk 
    datachunk* getlastdata();
    // devuelve una copia del datachunk con count =c (o null si no lo tenemos)
    datachunk* getdata(int c);
};
#endif /*TRACKINGPFC_DATA_*/