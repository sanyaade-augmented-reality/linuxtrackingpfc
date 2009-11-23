#include <cwiid.h>
#include <vrpn_Connection.h>
#include "vrpn_Tracker.h"

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

vrpn_Tracker_Server * nt;
struct timeval current_time;
vrpn_float64 position[3];
vrpn_float64 quaternion[4];
vrpn_Connection * connection;

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);
cwiid_mesg_callback_t cwiid_callback;
cwiid_err_t err;
cwiid_wiimote_t *wiimote;	/* wiimote handle */




void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap){
	// comento para que no salga nada de errores
}

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state){
	if (cwiid_set_led(wiimote, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
	}
}
	
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode){
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp){
	int i, j;
	int valid_source;
	int xacum, yacum;
	float x,y;
	for (i=0; i < mesg_count; i++){
		switch (mesg[i].type) {
		  case CWIID_MESG_IR:
			  valid_source = 0;
			  xacum=0;
			  yacum=0;
			  for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				  if (mesg[i].ir_mesg.src[j].valid) {
					  valid_source++;
					  xacum+=mesg[i].ir_mesg.src[j].pos[CWIID_X];
					  yacum+=mesg[i].ir_mesg.src[j].pos[CWIID_Y];
				  }
			  }
			  if (valid_source>0) {
			    // el wiimote es de ~ 1024x768
			    x=512.0-(float(xacum)/float(valid_source));
			    y=(float(yacum)/float(valid_source))-384.0;
			    x=x/512.0;
			    y=y/384.0;
			    x=x/2.0;
			    y=y/2.0;
			    printf("%f, %f\n",x,y);
			    position[0]=x;
			    position[1]=y;
			    position[2]=0.5;
			    vrpn_gettimeofday(&current_time, NULL);
			    nt->report_pose(0,current_time, position, quaternion);
			    nt->mainloop();
			    connection->mainloop();
			  }
			  break;
		  default:
			  printf("Unknown Report");
			  break;
		}
	}
}

void detectwiimote(){ // toda esta funcion deberia ser no bloqueante
  if (wiimote) return;
  cwiid_set_err(err);

  // conectarse al primer wiimote que se encuentre
  bdaddr_t bdaddr = *BDADDR_ANY;// bluetooth device address

  // Connect to the wiimote s
  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
  while (!wiimote){
    if ( !(wiimote = cwiid_open(&bdaddr, 0)) && !(wiimote = cwiid_open(&bdaddr, 0)) ){
	    printf("waiting for a wiimote (press 1+2)...\n");
    }
  }

  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
	  fprintf(stderr, "Unable to set message callback\n");
  }
  if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
	  fprintf(stderr, "Error enabling messages\n");
  }
  unsigned char rpt_mode = 0;
  toggle_bit(rpt_mode, CWIID_RPT_IR);
  set_rpt_mode(wiimote, rpt_mode);
  printf("Wiimote connected!\n");
  
}

int main( int argc, char** argv ){
    int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
    char  con_name[1024];
    sprintf(con_name, ":%d", port);
    connection = vrpn_create_server_connection(con_name, NULL, NULL);
    if ((nt = new vrpn_Tracker_Server("Tracker0", connection)) == NULL){
      fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
      return -1;
    }
    
    detectwiimote();
    
    while(1){
      // Sleep so we don't eat the CPU
      vrpn_SleepMsecs(1);
    }
}