#ifndef TPFC_DEVICE_ARTOOLKIT_
#define TPFC_DEVICE_ARTOOLKIT_

#include "TPFC_device.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glut.h>

#include <AR/ar.h>
#include <AR/gsub.h>
#include <AR/video.h>


class TPFC_device_artoolkit : public TPFC_device{

  private:

    // thread encargado de la detección y funciones auxiliares
    pthread_t art_thread;
    static void* art_main(void * t);
    static int mainLoop(int patt_id, int count, TPFC_device*);
    static void draw( double trans[3][4]);
    static void cleanup(void);

    // variable auxiliar para la gestion de inicializaciones unicas
    // (para detectar cual es el primer device, que es el unico que debe hacerlo)
    static int firstinstance;

  public:
    // consctructora y creadora
    TPFC_device_artoolkit(int ident,int , char **);
    ~TPFC_device_artoolkit();

    // sobrecarga de stop (para parar el thread)
    void stop();

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_AARTOOLKIT_*/