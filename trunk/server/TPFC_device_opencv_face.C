#include "TPFC_device_opencv_face.h" 



int detect_and_draw( IplImage* img, double scale,  CvMemStorage* storage, CvHaarClassifierCascade* cascade, const char* winname, TPFC_device_opencv_face* d){
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
    int res;

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
	  float* aux= new float[3];
	  /*aux[0]=-(center.x-320)/640.0;;
	  aux[1]=-(center.y-240)/480.0;*/
	  aux[0]=atan(-(center.x-320)/640.0);
	  aux[1]=atan(-(center.y-240)/480.0);
	  aux[2]=radius;
	  (d->getdata())->setnewdata(aux);
	}else{
	   (d->getdata())->setnodata();
	}
    }
    

    cvShowImage( winname, img );
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
    return i;
}

void* TPFC_device_opencv_face::facedetect(void * t){
  // para no tener que estar haciendo casts, creamos un apuntador al device
  TPFC_device_opencv_face* d = (TPFC_device_opencv_face*)t;

  // inicialización de lo necesario para el facedetect
  CvMemStorage* storage = 0;
  CvHaarClassifierCascade* cascade = 0;
  const char* cascade_name =
    "haarcascades/haarcascade_frontalface_alt.xml";
  double scale = 2.0;
  CvCapture* capture = 0;
  IplImage *frame, *frame_copy = 0;
  const char* input_name = 0;
  cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
  if( !cascade ){
      fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
      d->stop();
  }
  storage = cvCreateMemStorage(0);
  //capture = cvCaptureFromCAM( !input_name ? 0 : input_name[0] - '0' );
  capture = cvCaptureFromCAM(d->camera());
  char* winname = new char[48];
  sprintf(winname, "%s%i", "Face Detect on cam: ", d->idnum());
  cvNamedWindow( winname, 1 );
  if( !capture ){
    d->stop();
    printf("Fallo al iniciar la captura en la webcam, abortando thread");
  }
  // fin de las inicializaciones
    
  while (d->alive()==1){ // mientras no recibamos la señal de parar
    if (d->working()==1){ // si no estamos en pausa

      // bucle central del facedetect
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

      if (detect_and_draw( frame_copy , scale, storage, cascade, winname, d)==1)
	d->report();
      if (d->camera()==0) cvWaitKey( 10 ); // si quito esto la ventana no aparece :\
      // fin del bucle central de facedetect

      vrpn_SleepMsecs(1); // liberamos la cpu
    }else{// estamos en pausa
      vrpn_SleepMsecs(100); // el sleep es mas largo para consumir menos cpu
    }
  }

  

  // limpieza para finalizar el thread
  cvReleaseImage( &frame_copy );
  cvReleaseCapture( &capture );
  cvDestroyWindow("result");
  if (storage){
      cvReleaseMemStorage(&storage);
  }if (cascade){
      cvReleaseHaarClassifierCascade(&cascade);
  }


}

TPFC_device_opencv_face::TPFC_device_opencv_face(int ident, int c):TPFC_device(ident){
  cam = c;
  data = new TrackingPFC_data(TPFCDATA2DSIZE);
  // lanzamos el thread
  pthread_create( &facedetect_thread, NULL, facedetect,this);
  
}

TPFC_device_opencv_face::~TPFC_device_opencv_face(){
  free(data);
}

void TPFC_device_opencv_face::stop(){
   running= STOP;
   pthread_join( facedetect_thread, NULL);
}
int TPFC_device_opencv_face::camera(){
   return cam;
}