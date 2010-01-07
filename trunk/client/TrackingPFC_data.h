 #ifndef TRACKINGPFC_DATA_
#define TRACKINGPFC_DATA_

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

enum TPFCdatatype { TPFCDATA2D, TPFCDATA2DSIZE, TPFCDATA3D, TPFCDATA3DORI};

class TrackingPFC_data{
  // struct basico de la información relativa a un punto
  public:

    struct datachunk{
      private:
      float* data; // float[] que guarda los datos de posicion y orientacion
      clock_t time; // time
      int tag; // int reservado para que se puedan guardar estados
      int count; // numero de report (para facilitar la identificación de los datachunks);
      bool real; // flag de si es un reporte real o un dato ficticio (como por ejemplo la posicion inicial)
		 // asi se pueden ignorar los datos ficticios a la hora de trabajar con los datos
      bool valid; // flag de validez de los datos
      datachunk* next; // puntero al siguiente datachunk del mismo report (para reports con mas de 1 punto)
      
      public:
      // constructora
      datachunk(float* f, int c, bool r = true, datachunk* n=NULL);
      // copiadora
      datachunk(datachunk* d, int dsize);
      // destructora
      ~datachunk();
      
      // consultoras y escritoras
      // las que tienen un argumento es porque son individuales de cada punto
      // las que no lo tienen es porque deberian ser identicas en todos los puntos
      const float* getdata(int n =0); // devuelve los datos del punto n 
      clock_t gettime(int n =0); // devuelve el time
      int gettag(int n = 0); // devuelve el tag del punto n
      int getcount(); // devuelve el numero de report
      bool getreal(); // los datos son reales o artificiales
      bool getvalid(); // los datos son validos, o el report es vacio
      int size();  // cantidad de puntos en el report
      void setvalid(bool);
      void settag(int,int n = 0);

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
    // añade una nueva posición (copiando la orientación de la anterior posicion, si la hay)
    // funciona tb para datos 2d, simplemente ignora el 3r float
    void setnewpos(float, float, float f = 0.0);
    // añade un nuevo report con los datos de d, d debe tener tamaño dsize
    void setnewdata(float* d, bool real = true);

    // Escritoras avanzadas
    // añadir información de otro punto adicional al ultimo report
    void setmoredata(float* d, bool real = true);
    // añadir un nuevo report vacio y con el flag de datos no validos
    void setnodata(bool real = true);

    // Consultoras avanzadas
    // devuelve una copia del ultimo datachunk 
    datachunk* getlastdata();
};
#endif /*TRACKINGPFC_DATA_*/