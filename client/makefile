INCLUDE = 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = gcc
CXX= g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lvrpnserver -lvrpnatmel -lpthread  -lquat -lglut -lGL

all: libtpfcc.so.1.0.1

ctpfcc.o : CTrackingPFC_client.C CTrackingPFC_client.h tpfcc.o
	$(CXX) $(CFLAGS) -shared -fpic -c -o $@ $(LIBDIR) $< $(LIBRARIES)

tpfcc.o : TrackingPFC_client.C TrackingPFC_client.h
	$(CXX) $(CFLAGS) -shared -fpic -c -o $@ $(LIBDIR) $< $(LIBRARIES)

libtpfcc.so.1.0.1 : tpfcc.o ctpfcc.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libtpfcc.so.1 -o $@ $(LIBDIR) $^ -lc $(LIBRARIES)

install : libtpfcc.so.1.0.1
	cp libtpfcc.so.1.0.1 /usr/lib/
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so.1
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so
	cp *.h /usr/include/


# to clean up:
clean:
	rm -f *~ *.o *.so*

