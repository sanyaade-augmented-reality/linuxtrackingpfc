INCLUDE = -I/home/plaga/bin/vrpn_07_22/vrpn/server_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/client_src/ -I/home/plaga/bin/vrpn_07_22/vrpn/ -I/home/plaga/bin/vrpn_07_22/quat
LIBDIR  = -L/home/plaga/bin/vrpn_07_22/vrpn/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/server_src/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/client_src/pc_linux -L/home/plaga/bin/vrpn_07_22/quat/pc_linux

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lGL -lglut -ltpfcc

all: cube

cube : cube.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

# to clean up:
clean:
	rm cube *.o *~

