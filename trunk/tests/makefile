INCLUDE = -I/usr/include/ -I/usr/local/include/opencv
LIBDIR  = -L/usr/X11R6/lib -L/usr/local/lib 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -lcwiid -lcxcore -lcv -lhighgui -lcvaux -lml

all: cube facedetect cube_face cube_wii wii_leds

cube : cube.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

facedetect : facedetect.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

cube_face : cube_face.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

cube_wii : cube_wii.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

wii_leds : wii_leds.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)  

# to clean up:
clean:
	rm cube facedetect cube_face cube_wii wii_leds *.o *.c~ 

