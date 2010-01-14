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

class TPFC_device_opencv_face : public TPFC_device{
  private:
    pthread_t facedetect_thread; // thread que se encarga del facedetect
    int cam;


  public:
    // consctructora y creadora
    TPFC_device_opencv_face(int ident, int cam);
    ~TPFC_device_opencv_face(); 
    void stop();
    int camera();
    static void* facedetect(void * t);

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};



#endif /*TPFC_DEVICE_OPENCV_FACE_*/