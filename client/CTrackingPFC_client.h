typedef void CTrackingPFC_client;
#ifdef __cplusplus
extern "C" {
#endif

/*class TrackingPFC_client{
  public:
    // Creadoras y destructora
    //TrackingPFC_client(); // No hace falta, la de abajo la suple
    TrackingPFC_client(const char* tname="Tracker0@localhost", void(TrackingPFC_client*)=NULL);
    ~TrackingPFC_client();
    
    // Devuelve el tamaño de la pantalla (horizontal)
    float getDisplaySizex();
    //float getDisplaySizey();
    //float getDisplayRatio();

    // consultoras y escritoras de los datos
    float* getlastpos();
    
};*/

// Implementacion de las funciones basicas en C

CTrackingPFC_client* tpfcclient(const char* tname);
void tpfccdelete(CTrackingPFC_client* c);
float tpfccgetDisplaySizex(CTrackingPFC_client* c);
float* tpfccgetlastpos(CTrackingPFC_client* c);

#ifdef __cplusplus
}
#endif
