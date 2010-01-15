#ifndef TPFC_DEVICE_3DFROM2D_
#define TPFC_DEVICE_3DFROM2D_

#include "TPFC_device.h"

class TPFC_device_3dfrom2d : public TPFC_device{
   public:
    enum deeptype {FIJA, ROTACION, APROXSIZE, ONLYSIZE};// tipos de calculo de profundidad

   private:
    TPFC_device* source;
    // flags de control
    bool merge; // a cierto -> unificar todos los datos de un report y hacer la media
		// falso -> enviar cada dato de un report como un sensor diferente
    deeptype deep; // como se infiere la profundidad?
    double dist; // variable auxiliar para el calculo de profundidad, distinto uso segun deep
		// FIJA -> distancia al plano de la pantalla, en metros
		// ROTACION -> radio de rotación
		// APROXSIZE -> factor necesario para inferir la distancia dado un SIZE
    double lastvaliddist; // variable que guarda la ultima distancia valida calculada con APROXSIZE


   // función auxiliar que añade los datos segun el tipo de deep
   // si new==true se usara setdata, si ==false, se usara setmoredata (no se empezara report nuevo)
   void setdata(double, double, bool newrep = true, double obssize=0.0);

   public:
    // consctructora y creadora
    TPFC_device_3dfrom2d(int ident, TPFC_device* source);
    ~TPFC_device_3dfrom2d();
    // sobrecarga de report from, que en este caso es la que realizará los calculos del device
    void report_from(TPFC_device*);
    
    // Opciones
    void setmerge(bool); // merge a cierto o falso
    void setdeep(deeptype, double); // inferencia de profundidad los parametros son (deep, dist)

    // funcion que devuelve en un string la información relativa al dispositivo
    string info();

    // funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
    // devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
    // debe ser definida por todas las clases que hereden de device
    // (aunque no se puede hacer virtual ya que es estatica)
    static string checksource(TPFC_device*);
};

#endif /*TPFC_DEVICE_3DFROM2D_*/