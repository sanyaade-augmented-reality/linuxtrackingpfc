#include "TPFC_device_3dfrom2d.h" 


TPFC_device_3dfrom2d::TPFC_device_3dfrom2d(int ident, TPFC_device* source):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TPFCDATA3DORI);
  // La opción de merge esta activada por defecto
  merge = true;
  // registramos este dispositivo en la lista de listeners del que vamos a usar como input
  source-> report_to(this);

}

TPFC_device_3dfrom2d::~TPFC_device_3dfrom2d(){
  free(data);
}

void TPFC_device_3dfrom2d::report_from(TPFC_device* s){
  
  // obtenemos los datos
  TrackingPFC_data::datachunk* sourcedata= (s->getdata())->getlastdata();
  // comprobamos si los datos son validos
  if (sourcedata->getvalid() == false){
    // si no son validos guardamos un chunk no valido en nuestros datos y damos un nullreport
    data->setnodata();
    nullreport();
  }else{// si son validos...
    int n = sourcedata->size();
    if (merge){ // Hay que unificar todos los datos
      float acumx=0;
      float acumy=0;
      const float* aux;
      for (int i =0; i<n; i++){
	aux = sourcedata->getdata(i);
	printf("3dfrom2d:     %f, %f\n", aux[0], aux[1]);
	acumx+=aux[0];
	acumy+=aux[1];
      }
      printf("3dfrom2d: %f, %f\n", acumx, acumy);
      acumx=acumx/n;
      acumy=acumy/n;
      printf("3dfrom2d: %f, %f\n", acumx, acumy);
      data->setnewpos(acumx, acumy, 0.5);
    }else{ // Los datos se envian como sensores separados
      
    }
    // sea cual sea la opción, una vez escritos los datos, reportamos
    report();
  }
}