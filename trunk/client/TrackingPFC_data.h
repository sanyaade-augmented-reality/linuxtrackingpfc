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
      float* data; // float[] que guarda los datos de posicion y orientacion
      clock_t time; // time
      int tag; // int reservado para que se puedan guardar estados
      int count; // numero de report (para facilitar la identificación de los datachunks);
      bool real; // flag de si es un reporte real o un dato ficticio (como por ejemplo la posicion inicial)
		 // asi se pueden ignorar los datos ficticios a la hora de trabajar con los datos
      bool valid; // flag de validez de los datos
      datachunk* next; // puntero al siguiente datachunk del mismo report (para reports con mas de 1 punto)

      // constructora
      datachunk(float* f, int c, bool r = true, datachunk* n=NULL){
	data= f;
	time=clock();
	count = c;
	tag=0;
	valid=true;
	real=r;
	next=n;
      }
      // destructora
      ~datachunk(){
	// si hay datos, los destruimos
	if (data!=NULL)
	  free(data);
	// si hay mas datachunks en el mismo report, los destruimos
	if (next!=NULL)
	  free(next);
      }
      
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

    // añadir información de otro punto adicional al ultimo report
    void setmoredata(float* d, bool real = true);
};
#endif /*TRACKINGPFC_DATA_*/