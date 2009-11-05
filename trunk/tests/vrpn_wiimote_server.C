#include <stdio.h>

#include <vrpn_Connection.h>

#include "vrpn_WiiMote.h"


int main (int argc, char ** argv) {

  int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
  vrpn_Connection * connection;
  
  char  con_name[1024];
  sprintf(con_name, ":%d", port);

  connection = vrpn_create_server_connection(con_name, NULL, NULL);

  vrpn_WiiMote* wm;

  if ((wm = new vrpn_WiiMote("Wiimote0", connection, 0)) == NULL)
  {
    fprintf(stderr,"Can't create new vrpn_WiiMote\n");
    return -1;
  }
  fprintf(stderr,"Use 'vrpn_print_devices Wiimote0@localhost' in vrpn/client_src/pc_linux to see results\n");

  while (1) {
    wm->mainloop();
    connection->mainloop();
    // Sleep so we don't eat the CPU
    vrpn_SleepMsecs(1);
  }



}
