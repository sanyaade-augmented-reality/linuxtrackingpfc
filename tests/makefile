#INCLUDE = -I/usr/include/ -I/usr/local/include/opencv -I/home/plaga/bin/vrpn_07_22/vrpn/server_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/client_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/ -I/home/plaga/bin/vrpn_07_22/quat
#LIBDIR  = -L/usr/X11R6/lib -L/usr/local/lib -L/home/plaga/bin/vrpn_07_22/vrpn/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/server_src/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/client_src/pc_linux -L/home/plaga/bin/vrpn_07_22/quat/pc_linux
INCLUDE = -I/usr/local/include/opencv 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -lcwiid -lcxcore -lcv -lhighgui -lcvaux -lml -lvrpnserver -lvrpnatmel -lpthread  -lquat  -lwiiuse

all: cube facedetect cube_face cube_wii wii_leds vrpn_wiimote_server vrpn_tracker_null_server vrpn_tracker_client cube_tracker vrpn_tracker_examplenull_server vrpn_tracker_moving_server vrpn_tracker_from_opencv vrpn_tracker_from_wii_1led

cube : cube.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

cube_face : cube_face.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

cube_tracker : cube_tracker.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

cube_wii : cube_wii.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

facedetect : facedetect.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_client : vrpn_tracker_client.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_examplenull_server : vrpn_tracker_examplenull_server.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_from_opencv : vrpn_tracker_from_opencv.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_from_wii_1led : vrpn_tracker_from_wii_1led.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_moving_server : vrpn_tracker_moving_server.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_tracker_null_server : vrpn_tracker_null_server.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

vrpn_wiimote_server : vrpn_wiimote_server.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES) 

wii_leds : wii_leds.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)


# to clean up:
clean:
	rm -f cube facedetect cube_face cube_wii wii_leds vrpn_wiimote_server vrpn_tracker_null_server vrpn_tracker_client cube_tracker vrpn_tracker_examplenull_server vrpn_tracker_moving_server vrpn_tracker_from_opencv vrpn_tracker_from_wii_1led *.o *~

