#define CV_NO_BACKWARD_COMPATIBILITY

#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include <stdarg.h>
#include <GL/glut.h>


#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

#ifdef _EiC
#define WIN32
#endif

static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
static CvHaarClassifierCascade* nested_cascade = 0;
int use_nested_cascade = 0;





// *******************************************************************
   // posiciones del observador
   GLfloat obsx;
   GLfloat obsy;
   GLfloat obsz;
   GLint framen;
   GLchar mensaje[100];

void init(void){
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);
   //glViewport (0, 0, (GLsizei) w, (GLsizei) h); // al estar usando fullscreen esto ya esta definido?
   obsx = 0.0;
   obsy = 0.0;
   obsz = 16.0;
}
 
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void output(float x, float y, char *string){
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}

  
void display(void){

   GLfloat znear =1.0;
   GLfloat zfar =100.0;
   GLfloat frleft=znear*(-8.0-obsx)/obsz;
   GLfloat frright=znear*(8.0-obsx)/obsz;
   GLfloat frup=znear*(-5.0-obsy)/obsz;
   GLfloat frdown=znear*(5.0-obsy)/obsz;

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glFrustum (frleft, frright, frup, frdown, znear, zfar);
   glMatrixMode (GL_MODELVIEW);

   glClear (GL_COLOR_BUFFER_BIT);
   glLoadIdentity ();             /* clear the matrix */

           /* viewing transformation  */
   gluLookAt (obsx, obsy, obsz, obsx, obsy, obsz-1.0, 0.0, 1.0, 0.0);

  glLineWidth(1.0);
  float i;
  for (i=-50.0;i<=0.0;i+=2.0){
   glBegin(GL_LINE_LOOP);
    if (i==0.0) glColor3f(0.0, 1.0, 0.0);
    else if (i==-2.0) glColor3f(1.0, 0.7, 0.7); 
    else glColor3f(0.7, 0.7, 1.0);
    glVertex3f(-8.0, -5.0, i);
    glVertex3f(8.0, -5.0, i);
    glVertex3f(8.0, 5.0, i);
    glVertex3f(-8.0, 5.0, i);
   glEnd();
  }
   glBegin(GL_LINES);
    glColor3f(0.7, 0.7, 1.0);
    glVertex3f(-8.0, -5.0, -30.0);
    glVertex3f(-8.0, -5.0, 30.0);
    glVertex3f(8.0, -5.0, -30.0);
    glVertex3f(8.0, -5.0, 30.0);
    glVertex3f(-8.0, 5.0, -30.0);
    glVertex3f(-8.0, 5.0, 30.0);
    glVertex3f(8.0, 5.0, -30.0);
    glVertex3f(8.0, 5.0, 30.0);
   glEnd();

   glColor3f(1.0, 0.0, 0.0);
   glLineWidth(3.0);
   glutWireCube (4.0);

   glColor3f(1.0, 1.0, 1.0);
   char buffer[160];
   sprintf(buffer, "X %f", obsx);   
   output(-7,-3.5,buffer ); 
   sprintf(buffer, "Y %f", obsy);   
   output(-7,-4.0,buffer ); 
   sprintf(buffer, "Z %f", obsz);   
   output(-7,-4.5,buffer );
  
   sprintf(buffer, "Frame %i", framen);   
   output(5.5,-4.5,buffer );
   framen++;
    
   output(-7.0, 4.5, mensaje );

   glutSwapBuffers(); //swap the buffers
    
}

void keyboard(unsigned char key, int x, int y){
   switch (key) {
      case 27:
	 glutLeaveGameMode(); //set the resolution how it was
         exit(0);
         break;
      case 119: // w
	 obsz-=0.1;
	 glutPostRedisplay();
	 break;
      case 115: // s
	 obsz+=0.1;
	 glutPostRedisplay();
	 break;
      case 97: // a
	 obsx-=0.1;
	 glutPostRedisplay();
	 break;
      case 100: // d
	 obsx+=0.1;
	 glutPostRedisplay();
	 break;
      case 120: // x
	 obsy-=0.1;
	 glutPostRedisplay();
	 break;
      case 32: // space
	 obsy+=0.1;
	 glutPostRedisplay();
	 break;
      case 99: // c
	//detectwiimote();
	break;
      default:
	printf("Key %i not supported\n", key);
	break;
   }
}

void updateCube(int x, int y){
  obsx=8.0 - (x/40.0);// 64=1024/16
  obsy=5.0 - (y/51.0);// 48=768/16;

}







void detect_and_draw( IplImage* image );

const char* cascade_name =
    "haarcascades/haarcascade_frontalface_alt.xml";
/*    "haarcascade_profileface.xml";*/
const char* nested_cascade_name =
    "haarcascades/haarcascade_eye_tree_eyeglasses.xml";
//    "../../data/haarcascades/haarcascade_eye.xml";
double scale = 2.0;


CvCapture* capture = 0;
IplImage *frame, *frame_copy = 0;
IplImage *image = 0;

void proces_frame(){
  frame = cvQueryFrame( capture );
  if( frame ){
    if( !frame_copy )
	frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
				    IPL_DEPTH_8U, frame->nChannels );
    if( frame->origin == IPL_ORIGIN_TL )
	cvCopy( frame, frame_copy, 0 );
    else
	cvFlip( frame, frame_copy, 0 );

    detect_and_draw( frame_copy );
    glutPostRedisplay();
  }
}

int main( int argc, char** argv )
{
    
    
    const char* scale_opt = "--scale=";
    int scale_opt_len = (int)strlen(scale_opt);
    const char* cascade_opt = "--cascade=";
    int cascade_opt_len = (int)strlen(cascade_opt);
    const char* nested_cascade_opt = "--nested-cascade";
    int nested_cascade_opt_len = (int)strlen(nested_cascade_opt);
    int i;
    const char* input_name = 0;

    for( i = 1; i < argc; i++ )
    {
        if( strncmp( argv[i], cascade_opt, cascade_opt_len) == 0 )
            cascade_name = argv[i] + cascade_opt_len;
        else if( strncmp( argv[i], nested_cascade_opt, nested_cascade_opt_len ) == 0 )
        {
            if( argv[i][nested_cascade_opt_len] == '=' )
                nested_cascade_name = argv[i] + nested_cascade_opt_len + 1;
            nested_cascade = (CvHaarClassifierCascade*)cvLoad( nested_cascade_name, 0, 0, 0 );
            if( !nested_cascade )
                fprintf( stderr, "WARNING: Could not load classifier cascade for nested objects\n" );
        }
        else if( strncmp( argv[i], scale_opt, scale_opt_len ) == 0 )
        {
            if( !sscanf( argv[i] + scale_opt_len, "%lf", &scale ) || scale < 1 )
                scale = 1;
        }
        else if( argv[i][0] == '-' )
        {
            fprintf( stderr, "WARNING: Unknown option %s\n", argv[i] );
        }
        else
            input_name = argv[i];
    }

    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );

    if( !cascade )
    {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        fprintf( stderr,
        "Usage: facedetect [--cascade=\"<cascade_path>\"]\n"
        "   [--nested-cascade[=\"nested_cascade_path\"]]\n"
        "   [--scale[=<image scale>\n"
        "   [filename|camera_index]\n" );
        return -1;
    }
    storage = cvCreateMemStorage(0);

    if( !input_name || (isdigit(input_name[0]) && input_name[1] == '\0') )
        capture = cvCaptureFromCAM( !input_name ? 0 : input_name[0] - '0' );
    else if( input_name )
    {
        image = cvLoadImage( input_name, 1 );
        if( !image )
            capture = cvCaptureFromAVI( input_name );
    }
    else
        image = cvLoadImage( "lena.jpg", 1 );

    cvNamedWindow( "result", 1 );

    
    framen=0;
    sprintf(mensaje,"Keys: w a s d x space c esc\n");
    //detectwiimote();

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (960, 600); 
    glutInitWindowPosition (0,0);
    glutCreateWindow (argv[0]);
    //glutGameModeString( "1024x768:32@60" ); //the settings for fullscreen mode
    //glutEnterGameMode(); //set glut to fullscreen using the settings in the line above
    init ();
    glutDisplayFunc(display); 
    glutKeyboardFunc(keyboard);
    glutIdleFunc(proces_frame);
    glutMainLoop();






    if( capture )
    {
        /*for(;;)
        {
            frame = cvQueryFrame( capture );
            if( !frame )
                break;
            if( !frame_copy )
                frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
                                            IPL_DEPTH_8U, frame->nChannels );
            if( frame->origin == IPL_ORIGIN_TL )
                cvCopy( frame, frame_copy, 0 );
            else
                cvFlip( frame, frame_copy, 0 );

            detect_and_draw( frame_copy );

            if( cvWaitKey( 10 ) >= 0 )
                goto _cleanup_;
        }*/

        //cvWaitKey(0);
_cleanup_:
        cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
    }
    /*else
    {
        if( image )
        {
            detect_and_draw( image );
            cvWaitKey(0);
            cvReleaseImage( &image );
        }
        else if( input_name )
        {
            // assume it is a text file containing the
            //   list of the image filenames to be processed - one per line 
            FILE* f = fopen( input_name, "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    printf( "file %s\n", buf );
                    image = cvLoadImage( buf, 1 );
                    if( image )
                    {
                        detect_and_draw( image );
                        c = cvWaitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                        cvReleaseImage( &image );
                    }
                }
                fclose(f);
            }
        }
    }*/

    cvDestroyWindow("result");

    if (storage)
    {
        cvReleaseMemStorage(&storage);
    }
    if (cascade)
    {
        cvReleaseHaarClassifierCascade(&cascade);
    }

    return 0;
}

void detect_and_draw( IplImage* img )
{
    static CvScalar colors[] =
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };

    IplImage *gray, *small_img;
    int i, j;

    gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)), 8, 1 );

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

    if( cascade )
    {
        double t = (double)cvGetTickCount();
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0
                                            //|CV_HAAR_FIND_BIGGEST_OBJECT
                                            //|CV_HAAR_DO_ROUGH_SEARCH
                                            |CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,
                                            cvSize(30, 30) );
        t = (double)cvGetTickCount() - t;
        printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
            CvMat small_img_roi;
            CvSeq* nested_objects;
            CvPoint center;
            CvScalar color = colors[i%8];
            int radius;
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
	    printf("X= %i, Y= %i\n",center.x,center.y);
	    updateCube(center.x, center.y);
            radius = cvRound((r->width + r->height)*0.25*scale);
            cvCircle( img, center, radius, color, 3, 8, 0 );
            /*if( !nested_cascade )
                continue;
            cvGetSubRect( small_img, &small_img_roi, *r );
            nested_objects = cvHaarDetectObjects( &small_img_roi, nested_cascade, storage,
                                        1.1, 2, 0
                                        //|CV_HAAR_FIND_BIGGEST_OBJECT
                                        //|CV_HAAR_DO_ROUGH_SEARCH
                                        //|CV_HAAR_DO_CANNY_PRUNING
                                        //|CV_HAAR_SCALE_IMAGE
                                        ,
                                        cvSize(0, 0) );
            for( j = 0; j < (nested_objects ? nested_objects->total : 0); j++ )
            {
                CvRect* nr = (CvRect*)cvGetSeqElem( nested_objects, j );
                center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
                center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
                radius = cvRound((nr->width + nr->height)*0.25*scale);
                cvCircle( img, center, radius, color, 3, 8, 0 );
            }*/
        }
    }

    cvShowImage( "result", img );
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
}

