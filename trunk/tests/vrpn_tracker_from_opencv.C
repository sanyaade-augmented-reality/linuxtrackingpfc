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
#include <vrpn_Connection.h>
#include "vrpn_Tracker.h"

#ifdef _EiC
#define WIN32
#endif

static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
const char* cascade_name =
    "haarcascades/haarcascade_frontalface_alt.xml";
/*    "haarcascade_profileface.xml";*/
double scale = 2.0;


vrpn_Tracker_Server * nt;
struct timeval current_time;
vrpn_float64 position[3];
vrpn_float64 quaternion[4];
vrpn_Connection * connection;

void detect_and_draw( IplImage* img ){
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
    int i;

    gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)), 8, 1 );

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );
    if( cascade ){
        double t = (double)cvGetTickCount();
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0
                                            |CV_HAAR_FIND_BIGGEST_OBJECT
                                            //|CV_HAAR_DO_ROUGH_SEARCH
                                            //|CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,cvSize(30, 30) );
        t = (double)cvGetTickCount() - t;
        //printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
	CvPoint center;
	int radius;
        for( i = 0; i < (faces ? faces->total : 0); i++ ){
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
            CvMat small_img_roi;
            CvScalar color = colors[i%8];
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            cvCircle( img, center, radius, color, 3, 8, 0 );
            cvGetSubRect( small_img, &small_img_roi, *r );
        }
	if (i>0){
	  //printf("X: %i, Y: %i, r: %i (%f)\n", center.x, center.y, radius, sqrt(radius));
	  position[0]=-(center.x-320)/640.0;
	  position[1]=-(center.y-240)/480.0;
	  //position[2]=1.0 + ( (6.0-sqrt(radius))/4 );
	  position[2]=0.5;
	  vrpn_gettimeofday(&current_time, NULL);
	  nt->report_pose(0,current_time, position, quaternion);
	  nt->mainloop();
	  connection->mainloop();
	}
    }
    

    cvShowImage( "result", img );
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
}


int main( int argc, char** argv ){
    int	port = vrpn_DEFAULT_LISTEN_PORT_NO;
    char  con_name[1024];
    sprintf(con_name, ":%d", port);
    connection = vrpn_create_server_connection(con_name, NULL, NULL);
    if ((nt = new vrpn_Tracker_Server("Tracker0", connection)) == NULL){
      fprintf(stderr,"Can't create new vrpn_Tracker_NULL\n");
      return -1;
    }
    
    CvCapture* capture = 0;
    IplImage *frame, *frame_copy = 0;
    const char* input_name = 0;

    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );

    if( !cascade ){
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        return -1;
    }
    storage = cvCreateMemStorage(0);

    
    capture = cvCaptureFromCAM( !input_name ? 0 : input_name[0] - '0' );

    cvNamedWindow( "result", 1 );

    if( capture ){
        for(;;){
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
        }

        cvWaitKey(0);
_cleanup_:
        cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
    }

    cvDestroyWindow("result");

    if (storage){
        cvReleaseMemStorage(&storage);
    }if (cascade){
        cvReleaseHaarClassifierCascade(&cascade);
    }

    return 0;
}

