typedef void CTrackingPFC_client;
#ifdef __cplusplus
extern "C" {
#endif

// Implementacion de las funciones basicas en C

CTrackingPFC_client* tpfcclient(const char* tname);
void tpfccdelete(CTrackingPFC_client* c);
float tpfccgetDisplaySizex(CTrackingPFC_client* c);
float tpfccgetDisplaySizey(CTrackingPFC_client* c);
int tpfccgetDisplayWidth(CTrackingPFC_client* c);
int tpfccgetDisplayHeight(CTrackingPFC_client* c);
float* tpfccgetlastpos(CTrackingPFC_client* c);

#ifdef __cplusplus
}
#endif
