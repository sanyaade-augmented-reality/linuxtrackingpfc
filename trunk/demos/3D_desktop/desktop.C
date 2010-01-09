#include <GL/glut.h>
#include <TrackingPFC_client.h>
 #include <GL/glu.h>
 #include <png.h>
 #include <cstdio>
 #include <string>
 
 #define TEXTURE_LOAD_ERROR 0


using namespace std;
 /** loadTexture
  * 	loads a png file into an opengl texture object, using cstdio , libpng, and opengl.
  * 
  * 	\param filename : the png file to be loaded
  * 	\param width : width of png, to be updated as a side effect of this function
  * 	\param height : height of png, to be updated as a side effect of this function
  * 
  * 	\return GLuint : an opengl texture id.  Will be 0 if there is a major error,
  * 					should be validated by the client of this function.
  * 
  */
 GLuint loadTexture(const string filename, int &width, int &height) 
 {
   //header for testing if it is a png
   png_byte header[8];
 
   //open file as binary
   FILE *fp = fopen(filename.c_str(), "rb");
   if (!fp) {
     return TEXTURE_LOAD_ERROR;
   }
 
   //read the header
   fread(header, 1, 8, fp);
 
   //test if png
   int is_png = !png_sig_cmp(header, 0, 8);
   if (!is_png) {
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
 
   //create png struct
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
       NULL, NULL);
   if (!png_ptr) {
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //create png info struct
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
     png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //create png info struct
   png_infop end_info = png_create_info_struct(png_ptr);
   if (!end_info) {
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //png error stuff, not sure libpng man suggests this.
   if (setjmp(png_jmpbuf(png_ptr))) {
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //init png reading
   png_init_io(png_ptr, fp);
 
   //let libpng know you already read the first 8 bytes
   png_set_sig_bytes(png_ptr, 8);
 
   // read all the info up to the image data
   png_read_info(png_ptr, info_ptr);
 
   //variables to pass to get info
   int bit_depth, color_type;
   png_uint_32 twidth, theight;
 
   // get info about png
   png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
       NULL, NULL, NULL);
 
   //update width and height based on png info
   width = twidth;
   height = theight;
 
   // Update the png info struct.
   png_read_update_info(png_ptr, info_ptr);
 
   // Row size in bytes.
   int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
 
   // Allocate the image_data as a big block, to be given to opengl
   png_byte *image_data = new png_byte[rowbytes * height];
   if (!image_data) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
 
   //row_pointers is for pointing to image_data for reading the png with libpng
   png_bytep *row_pointers = new png_bytep[height];
   if (!row_pointers) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     delete[] image_data;
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
   // set the individual row_pointers to point at the correct offsets of image_data
   for (int i = 0; i < height; ++i)
     row_pointers[height - 1 - i] = image_data + i * rowbytes;
 
   //read the png into image_data through row_pointers
   png_read_image(png_ptr, row_pointers);
 
   //Now generate the OpenGL texture object
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, width, height, 0,
       GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   delete[] image_data;
   delete[] row_pointers;
   fclose(fp);
 
   return texture;
 }
 


TrackingPFC_client* track;

void init(void) 
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);

   //glEnable (GL_DEPTH_TEST);
   glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
   glEnable (GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   /*if (!loadTGA ("texture.tga", 13))
      printf ("texture.tga not found!\n");*/
    int x,y;
    GLuint t = loadTexture("desk.png", x, y);
    t = loadTexture("barsw.png", x, y);
    t = loadTexture("icons.png", x, y);
    //printf("x= %i\n",t);
}
   
void display(void)
{

   GLfloat znear =1.0;
   GLfloat zfar =100.0;
   
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   
   track->htadjustPerspective(1.6, znear, zfar);
   glMatrixMode (GL_MODELVIEW);
   

   glClear (GL_COLOR_BUFFER_BIT);
   glLoadIdentity ();             /* clear the matrix */
   
   track->setvirtualdisplaysize( 16.0);
   track->htgluLookAt (0, 0, 0,  0, 0, -1.0,  0.0, 1.0, 0.0);

  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, 1);
  //glColor3f(1.0, 1.0, 1.0);
  float prof = -2.0;
  float fact = 1.3;
  glBegin(GL_QUADS);
    glTexCoord2f (0.0f,0.0f);
    glVertex3f(-8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 0.0f);
    glVertex3f(8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 1.0f);
    glVertex3f(8.0*fact, 5.0*fact, prof);
    glTexCoord2f (0.0f, 1.0f);
    glVertex3f(-8.0*fact, 5.0*fact, prof);
  glEnd();

  glBindTexture (GL_TEXTURE_2D, 2);
  //glColor3f(1.0, 1.0, 1.0);
  prof = 0.0;
  fact = 1.0;
  glBegin(GL_QUADS);
    glTexCoord2f (0.0f,0.0f);
    glVertex3f(-8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 0.0f);
    glVertex3f(8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 1.0f);
    glVertex3f(8.0*fact, 5.0*fact, prof);
    glTexCoord2f (0.0f, 1.0f);
    glVertex3f(-8.0*fact, 5.0*fact, prof);
  glEnd();

  glBindTexture (GL_TEXTURE_2D, 3);
  //glColor3f(1.0, 1.0, 1.0);
  prof = 0.6;
  fact = 1.0;
  glBegin(GL_QUADS);
    glTexCoord2f (0.0f,0.0f);
    glVertex3f(-8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 0.0f);
    glVertex3f(8.0*fact, -5.0*fact, prof);
    glTexCoord2f (1.0f, 1.0f);
    glVertex3f(8.0*fact, 5.0*fact, prof);
    glTexCoord2f (0.0f, 1.0f);
    glVertex3f(-8.0*fact, 5.0*fact, prof);
  glEnd();


  glDisable (GL_TEXTURE_2D); 

  /*glColor3f(0.4, 0.0, 0.0);
  glBegin(GL_QUADS);
  glVertex3f(-8.0, -5.0, 0.0);
  glVertex3f(8.0, -5.0, 0.0);
  glVertex3f(8.0, -4.0, 0.0);
  glVertex3f(-8.0, -4.0, 0.0);
  glEnd();
  glBegin(GL_QUADS);
  glVertex3f(-8.0, 4.0, 0.0);
  glVertex3f(8.0, 4.0, 0.0);
  glVertex3f(8.0, 5.0, 0.0);
  glVertex3f(-8.0, 5.0, 0.0);
  glEnd();*/

  /*glColor3f(0.6, 0.0, 0.0);
  float prof2 = 0.2;
  glBegin(GL_QUADS);
    glVertex3f(-6.0, 2.0, prof2);
    glVertex3f(-5.0, 2.0, prof2);
    glVertex3f(-5.0, 3.0, prof2);
    glVertex3f(-6.0, 3.0, prof2);
  glEnd();
  glBegin(GL_QUADS);
    glVertex3f(-4.0, 2.0, prof2);
    glVertex3f(-3.0, 2.0, prof2);
    glVertex3f(-3.0, 3.0, prof2);
    glVertex3f(-4.0, 3.0, prof2);
  glEnd();
  glBegin(GL_QUADS);
    glVertex3f(-6.0, 0.0, prof2);
    glVertex3f(-5.0, 0.0, prof2);
    glVertex3f(-5.0, 1.0, prof2);
    glVertex3f(-6.0, 1.0, prof2);
  glEnd();
  glBegin(GL_QUADS);
    glVertex3f(-4.0, 0.0, prof2);
    glVertex3f(-3.0, 0.0, prof2);
    glVertex3f(-3.0, 1.0, prof2);
    glVertex3f(-4.0, 1.0, prof2);
  glEnd();
  */
  
  glutSwapBuffers(); //swap the buffers
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
	 glutLeaveGameMode(); //set the resolution how it was
	 delete(track);
         exit(0);
         break;
      default:
	//printf("Key %i not supported\n", key);
	break;
   }
}

int main(int argc, char** argv)
{
   //printf("%i \n",argc);
   char* resolution;
   if (argc>1){
       resolution= (char*)argv[1];
   }else{
       resolution= (char*)"1920x1200:32@60";
   }

   char* trkname = (char*)"Tracker0@localhost";
   // si se ha llamado con un parametro, asumimos que es un nombre de tracker alternativo
   // se espera un nombre valido y libre, si no lo es, la aplicación fallara
   if (argc>2)
     trkname=argv[2];

   track = new TrackingPFC_client(trkname);

   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   /*glutInitWindowSize (960, 600); 
   glutInitWindowPosition (0,0);
   glutCreateWindow (argv[0]);*/
   glutGameModeString( resolution ); //the settings for fullscreen mode
   glutEnterGameMode(); //set glut to fullscreen using the settings in the line above*/
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(glutPostRedisplay);
   glutMainLoop();
   return 0;
}
