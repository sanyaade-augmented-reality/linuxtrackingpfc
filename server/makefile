INCLUDE = -I/usr/local/include/opencv 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lvrpnserver -lvrpnatmel -lpthread  -lquat -ltpfcc -lcwiid -lcv -lhighgui

all: tpfcserver

tpfcdevopencvface.o : TPFC_device_opencv_face.C TPFC_device_opencv_face.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3df2d.o : TPFC_device_3dfrom2d.C TPFC_device_3dfrom2d.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3dstereo.o : TPFC_device_3dstereo.C TPFC_device_3dstereo.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdevwii.o : TPFC_device_wiimote.C TPFC_device_wiimote.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev.o : TPFC_device.C TPFC_device.h
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcserver : TPFC_server.C tpfcdev.o tpfcdevopencvface.o tpfcdev3df2d.o tpfcdev3dstereo.o tpfcdevwii.o
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $^ $(LIBRARIES)


# to clean up:
clean:
	rm *~ *.o *.so* tpfcserver

