#include <stdio.h>

#include <vrpn_Connection.h>

#include "vrpn_Tracker.h"


int main (int argc, char ** argv) {

  int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
  vrpn_Connection * connection;
  
  char  con_name[1024];
  sprintf(con_name, ":%d", port);

  connection = vrpn_create_server_connection(con_name, NULL, NULL);

  vrpn_Tracker * nt;

  if ((nt = new vrpn_Tracker_NULL("Tracker0", connection, 2, 2.0)) == NULL)
  {
    fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
    return -1;
  }
  fprintf(stderr,"Created new NULL Tracker\nUse 'vrpn_print_devices Tracker0@localhost' in vrpn/client_src/pc_linux to see results\n");
  

  while (1) {
    nt->mainloop();
    connection->mainloop();
    // Sleep so we don't eat the CPU
    vrpn_SleepMsecs(1);
  }



}
