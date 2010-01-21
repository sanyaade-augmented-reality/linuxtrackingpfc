#ifndef TPFC_DEVICE_OPENCV_FACE_
#define TPFC_DEVICE_OPENCV_FACE_

#include <cv.h>
#include <highgui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "TPFC_device.h"

// para file exist
#include <sys/stat.h>

// para poder leer del archivo de configuracion
#include <iostream>
#include <fstream>
#include <sstream>

class TPFC_device_opencv_face : public TPFC_device{
  private:
    // numero de camara a usar
    int cam;
    // flag de singleuser/multiuser
    bool singleuser;

    // Variables auxiliares para el recorte de frame
    int wrongframes; // numero de frames incorrectos
    CvPoint2D32f lastframepos; // centro de la cara del ultimo frame
    int lastradius;

    float focal_length[2];
    
    // variable auxiliar para la gestion de la ventana
    // (para detectar cual es el primer device, que es el unico que debe hacerlo)
    static int firstinstance;

    // Facedetect
    pthread_t facedetect_thread;
    static void* facedetect(void * t);
    static void detect_and_draw( IplImage* , double ,  CvMemStorage* , CvHaarClassifierCascade* ,
			 const char* , TPFC_device_opencv_face*);

    // funcion auxiliar para comprobar si existen los archivos
    static bool FileExists(string strFilename);
    // funcion auxiliar para partir strings
    static void StringExplode(string str, string separator, vector<string>* results);

  public:
    // consctructora y creadora
    TPFC_device_opencv_face(int ident, int cam, bool single = true, float fl=560.0);
    ~TPFC_device_opencv_face();
 
    // sobrecarga de stop (para parar el thread)
    void stop();
    // consultora de la id de la camara
    int camera();

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};



#endif /*TPFC_DEVICE_OPENCV_FACE_*/