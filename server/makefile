INCLUDE = -I/usr/local/include/opencv
LIBDIR  = -L/home/plaga/bin/ARToolKit/lib

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2 -g
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lvrpnserver -lvrpnatmel -lpthread  -lquat -ltpfcc -lcwiid -lcv -lhighgui -lARgsub -lARvideo -lAR -lglut -lGL -lgstreamer-0.10

all: tpfcserver calibrate

tpfcdev.o : TPFC_device.C TPFC_device.h makefile
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3df2d.o : TPFC_device_3dfrom2d.C TPFC_device_3dfrom2d.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3dmerge.o : TPFC_device_3dmerge.C TPFC_device_3dmerge.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3dmod.o : TPFC_device_3dmod.C TPFC_device_3dmod.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3dpattern.o : TPFC_device_3dpattern.C TPFC_device_3dpattern.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdev3dstereo.o : TPFC_device_3dstereo.C TPFC_device_3dstereo.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdevartoolkit.o : TPFC_device_artoolkit.C TPFC_device_artoolkit.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdevopencvface.o : TPFC_device_opencv_face.C TPFC_device_opencv_face.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcdevwii.o : TPFC_device_wiimote.C TPFC_device_wiimote.h tpfcdev.o
	$(CC) $(CFLAGS) -c -o $@ $(LIBDIR) $< $(LIBRARIES)

calibrate: calibrate.C makefile
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcserver : TPFC_server.C tpfcdev.o tpfcdev3df2d.o tpfcdev3dmerge.o tpfcdev3dmod.o tpfcdev3dpattern.o tpfcdev3dstereo.o tpfcdevartoolkit.o tpfcdevopencvface.o tpfcdevwii.o
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $^ $(LIBRARIES)
	mkdir -p ~/.trackingpfc
	cp cfgs/*.tpfc ~/.trackingpfc
	cp cfgs/*.cfg ~/.trackingpfc	

install : tpfcserver calibrate
	cp tpfcserver /usr/bin/tpfcserver
	



# to clean up:
clean:
	rm -f *~ *.o *.so* tpfcserver calibrate cfgs/*~

