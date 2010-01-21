#include "TPFC_device_opencv_face.h" 

// incializacion de la variable estatica
int TPFC_device_opencv_face::firstinstance=-1;

// Constructora
TPFC_device_opencv_face::TPFC_device_opencv_face(int ident, int c, bool single):TPFC_device(ident){
  // si somos el primer device de este tipo, marcamos firstinstance
  if (firstinstance==-1)
    firstinstance=ident;
  // copiamos opcion de singleusar
  singleuser=single;
  // guardamos la camara
  cam = c;
  // Creamos el buffer de datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA2DSIZE);
  // lanzamos el thread
  pthread_create( &facedetect_thread, NULL, facedetect,this);
  
}

// Destructora
TPFC_device_opencv_face::~TPFC_device_opencv_face(){
  free(data);
}


// Thread de facedetect
void* TPFC_device_opencv_face::facedetect(void * t){
  // para no tener que estar haciendo casts, creamos un apuntador al device
  TPFC_device_opencv_face* d = (TPFC_device_opencv_face*)t;

  // escala a usara
    double scale = 2.0;

  // inicialización de lo necesario para el facedetect
  CvMemStorage* storage = 0;
  CvHaarClassifierCascade* cascade = 0;
  const char* cascade_name =
    "haarcascades/haarcascade_frontalface_alt.xml";
  CvCapture* capture = 0;
  IplImage *frame, *frame_copy = 0;
  const char* input_name = 0;
  cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
  if( !cascade ){
      fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
      d->stop();
  }
  storage = cvCreateMemStorage(0);

  // Captura de video desde la camara
  capture = cvCaptureFromCAM(d->camera());
  
  // Gestion de la ventana
  char* winname = new char[48];
  sprintf(winname, "FaceDetect (cam: %i)", d->idnum());
  // solo se crea si somos la primera instance
  if (d->idnum()==firstinstance) cvNamedWindow( winname );
  // si no es posible, avisamos al usuario y paramos el device
  if( !capture ){
    d->stop();
    printf("Fallo al iniciar la captura en la webcam, abortando thread");
  }

  // Carga de los parametros de la camara
  char filename1[200];
  char filename2[200];
  // formateamos el nombre del archivo
  sprintf(filename1, "%s/.trackingpfc/Distortion.xml",getenv ("HOME"));
  sprintf(filename2, "%s/.trackingpfc/Intrinsics.xml",getenv ("HOME"));
  // flag de uso de undistort
  bool undistort=false;

  IplImage* mapx;
  IplImage* mapy;
  if (FileExists(filename1) && FileExists(filename2)){

    // Cargamos los datos desde el disco
    CvMat *intrinsic = (CvMat*)cvLoad(filename2);
    CvMat *distortion = (CvMat*)cvLoad(filename1);

    // obtenemos un frame desde la camara para poder saber el tamaño
    frame = cvQueryFrame( capture );
    while (!frame){
      frame = cvQueryFrame( capture );
    }
    // creamos los mapas
    mapx = cvCreateImage( cvSize(frame->width,frame->height), IPL_DEPTH_32F, 1 );
    mapy = cvCreateImage( cvSize(frame->width,frame->height), IPL_DEPTH_32F, 1 );
    cvInitUndistortMap(intrinsic, distortion, mapx, mapy );

    // marcamos el flag de undistort a cierto
    undistort=true;

    printf("%s y Intrinsics.xml existen, datos cargados.\n", filename1);
  }
  // fin de las inicializaciones
    
  while (d->alive()==1){ // mientras no recibamos la señal de parar
    if (d->working()){ // si no estamos en pausa

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

      // si tenemos los datos, aplicamos undistort
      if (undistort){
	IplImage *t = cvCloneImage(frame_copy);
	cvRemap( t, frame_copy, mapx, mapy );     // Undistort image
	cvReleaseImage(&t);
      }

      // llamamos a detect&draw
      if (detect_and_draw( frame_copy , scale, storage, cascade, winname, d)==1)
	d->report();
      // si somos la primera instancia, ejecutamos el bucle de la ventana
      if (d->idnum()==firstinstance) cvWaitKey( 10 ); // si quito esto la ventana no aparece :\

      // fin del bucle central de facedetect

      vrpn_SleepMsecs(1); // liberamos la cpu
    }else{// estamos en pausa
      vrpn_SleepMsecs(100); // el sleep es mas largo para consumir menos cpu
    }
  }

  

  // limpieza para finalizar el thread
  cvReleaseImage( &frame_copy );
  cvReleaseCapture( &capture );
  if (d->idnum()==firstinstance) cvDestroyWindow(winname);
  if (storage){
      cvReleaseMemStorage(&storage);
  }if (cascade){
      cvReleaseHaarClassifierCascade(&cascade);
  }
}

int TPFC_device_opencv_face::detect_and_draw( IplImage* img, double scale,  CvMemStorage* storage, CvHaarClassifierCascade* cascade, const char* winname, TPFC_device_opencv_face* d){
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

    /*CvPoint2D32f cent;
    cent.x = cvRound(img->width*0.5);
    cent.y = cvRound(img->height*0.5);
    cvGetRectSubPix(gray, small_img,cent);*/

    if( cascade ){
        double t = (double)cvGetTickCount();
	CvSeq* faces;
	if (!d->singleuser){
	  // Buscando a mas de un usuario
	  faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0
                                            //|CV_HAAR_FIND_BIGGEST_OBJECT
                                            |CV_HAAR_DO_ROUGH_SEARCH
                                            //|CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,cvSize(30, 30) );
	}else{
	  // buscando solo un usuario
	  faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0
                                            |CV_HAAR_FIND_BIGGEST_OBJECT
                                            |CV_HAAR_DO_ROUGH_SEARCH
                                            //|CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,cvSize(30, 30) );
	}
        t = (double)cvGetTickCount() - t;
        printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
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
	  aux[0]=atan(-(center.x-320)/640.0);
	  aux[1]=atan(-(center.y-240)/480.0);
	  aux[2]=radius;
	  (d->getdata())->setnewdata(aux);
	}else{
	   (d->getdata())->setnodata();
	}
    }
    

    if (d->idnum()==firstinstance) cvShowImage( winname, img );
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
    return i;
}

// Sobrecarga de stop, para esperar al thread
void TPFC_device_opencv_face::stop(){
   running= STOP;
   pthread_join( facedetect_thread, NULL);
}


// Consultora de numero de camara
int TPFC_device_opencv_face::camera(){
   return cam;
}

// Informacion sobre el dispositivo
string TPFC_device_opencv_face::info(){
  return "OpenCV Facedetect";
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_opencv_face::checksource(TPFC_device*){
  string ret = "Este dispositivo no acepta fuentes.";
  return ret;
}



// funcion auxiliar para comprobar si un archivo existe
// (encontrada en http://www.techbytes.ca/techbyte103.html)
bool TPFC_device_opencv_face::FileExists(string strFilename) {
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }
  
  return(blnReturn);
}
