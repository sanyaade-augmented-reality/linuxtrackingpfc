#include <stdio.h>

#include <vrpn_Connection.h>

#include "vrpn_Tracker.h"

static	unsigned long	duration(struct timeval t1, struct timeval t2)
{
	return (t1.tv_usec - t2.tv_usec) +
	       1000000L * (t1.tv_sec - t2.tv_sec);
}

int main (int argc, char ** argv) {

  int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
  vrpn_Connection * connection;
  
  char  con_name[1024];
  sprintf(con_name, ":%d", port);

  connection = vrpn_create_server_connection(con_name, NULL, NULL);

  vrpn_Tracker_Server * nt;

  if ((nt = new vrpn_Tracker_Server("Tracker0", connection)) == NULL)
  {
    fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
    return -1;
  }
  fprintf(stderr,"Created new Tracker\nUse 'vrpn_print_devices Tracker0@localhost' in vrpn/client_src/pc_linux to see results\n");
  
  struct timeval current_time;
  struct timeval timestamp;
  timestamp.tv_sec = 0;
  timestamp.tv_usec = 0;
  vrpn_float64 position[3];
  vrpn_float64 quaternion[4];
  position[0]=0.0;
  position[1]=0.0;
  position[2]=0.0;
  vrpn_float64 update_rate = 50.0;

  while (1) {
    // comprobar que toca de verdad enviar algun report
    vrpn_gettimeofday(&current_time, NULL);
    if ( duration(current_time,timestamp) >= 1000000.0/update_rate) {
      // Update the time
      timestamp.tv_sec = current_time.tv_sec;
      timestamp.tv_usec = current_time.tv_usec;

      nt->report_pose(0,current_time, position, quaternion);
      nt->mainloop();
      connection->mainloop();
      // Sleep so we don't eat the CPU
      vrpn_SleepMsecs(1);
    }
  }



}
