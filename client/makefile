INCLUDE = -I/usr/include/ -I/home/plaga/bin/vrpn_07_22/vrpn/server_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/client_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/ -I/home/plaga/bin/vrpn_07_22/quat
LIBDIR  = -L/usr/local/lib -L/home/plaga/bin/vrpn_07_22/vrpn/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/server_src/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/client_src/pc_linux -L/home/plaga/bin/vrpn_07_22/quat/pc_linux

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lvrpnserver -lvrpnatmel -lpthread  -lquat

all: libtpfcc.so.1.0.1

tpfcc.o : TrackingPFC_client.C TrackingPFC_client.h
	$(CC) $(CFLAGS) -shared -fpic -c -o $@ $(LIBDIR) $< $(LIBRARIES)

libtpfcc.so.1.0.1 : tpfcc.o
	gcc $(CFLAGS) -shared -Wl,-soname,libtpfcc.so.1 -o $@ $(LIBDIR) $< -lc $(LIBRARIES)

install : libtpfcc.so.1.0.1
	cp libtpfcc.so.1.0.1 /usr/lib/
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so.1
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so

test : test.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $^ $(LIBRARIES) -ltpfcc


# to clean up:
clean:
	rm *~ test *.o *.so*

