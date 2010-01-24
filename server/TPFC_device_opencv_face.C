#include "TPFC_device_opencv_face.h" 

// incializacion de la variable estatica
int TPFC_device_opencv_face::firstinstance=-1;

// Constructora
TPFC_device_opencv_face::TPFC_device_opencv_face(int ident, int c, bool single, float fl):TPFC_device(ident){
  // si somos el primer device de este tipo, marcamos firstinstance
  if (firstinstance==-1)
    firstinstance=ident;
  // copiamos opcion de singleusar
  singleuser=single;
  // guardamos la camara
  cam = c;
  // Creamos el buffer de datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA2DSIZE);
  
  // focal length
  focal_length[0]=fl;
  focal_length[1]=fl;

  // inicializamos el contador de frames incorrectos a uno (como si en el frame anterior
  // no hubiesemos detectado caras, ya que no tenemos datos)
  wrongframes=1;

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

  // archivo de cascada
  string cascade_name;
  if (!getconfig("facedetectcascade", &cascade_name) ){
    printf("No se han podido leer el path del archivo de cascada en la configuracion, abortando device.\n");
    d->stop();
  }

  // inicialización de lo necesario para el facedetect
  CvMemStorage* storage = 0;
  CvHaarClassifierCascade* cascade = 0;
  CvCapture* capture = 0;
  IplImage *frame, *frame_copy = 0;
  const char* input_name = 0;

  
  //cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
  cascade =cvLoadHaarClassifierCascade(cascade_name.c_str(),cvSize(60,60) );

  if( !cascade ){
      fprintf( stderr, "ERROR: Could not load classifier cascade\n");
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
    
    // obtenemos los Focal Lengths, y los guardamos
    d->focal_length[0] =CV_MAT_ELEM(*intrinsic,float,0,0);
    d->focal_length[1] =CV_MAT_ELEM(*intrinsic,float,0,0);
    

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
      detect_and_draw( frame_copy , scale, storage, cascade, winname, d);
	

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

void TPFC_device_opencv_face::detect_and_draw( IplImage* img, double scale,  CvMemStorage* storage, CvHaarClassifierCascade* cascade, const char* winname, TPFC_device_opencv_face* d){
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

  // si estamos buscando mas de un usuario, o no teniamos imagen en el frame anterior
  // usamos toda la imagen, escalada
  if (!d->singleuser || d->wrongframes>0){
    gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
			  cvRound (img->height/scale)), 8, 1 );

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
  }else{
    // si estamos buscando un solo usuario y teniamos su posicion en el frame anterior
    // buscamos solo en un area mas pequeña de la imagen
    gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    small_img = cvCreateImage( cvSize( cvRound (d->lastradius*4),
			  cvRound (d->lastradius*3)), 8, 1 );
    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvGetRectSubPix(gray, small_img,d->lastframepos);
    cvEqualizeHist( small_img, small_img );
  }
  cvClearMemStorage( storage );
  

  if( cascade ){
      double t = (double)cvGetTickCount();
      CvSeq* faces;
      if (!d->singleuser){
	// Buscando a mas de un usuario
	faces = cvHaarDetectObjects( small_img, cascade, storage,
					  1.1, 2, 0
					  //|CV_HAAR_FIND_BIGGEST_OBJECT
					  //|CV_HAAR_DO_ROUGH_SEARCH
					  |CV_HAAR_DO_CANNY_PRUNING
					  //|CV_HAAR_SCALE_IMAGE
					  ,cvSize(30, 30) );
      }else if (d->wrongframes==0){
	// Buscando un solo usuario, frame anterior correcto
	// aqui podemos poner minsize == 0, ya que la imagen es pequeña
	// y el tiempo de busqueda (aunque no haya cara) es pequeño
	faces = cvHaarDetectObjects( small_img, cascade, storage,
					  1.1, 3, 0
					  |CV_HAAR_FIND_BIGGEST_OBJECT
					  |CV_HAAR_DO_ROUGH_SEARCH
					  //|CV_HAAR_DO_CANNY_PRUNING
					  //|CV_HAAR_SCALE_IMAGE
					  ,cvSize(0, 0) );
      }else{
	// buscando solo un usuario, frame anterior incorrecto
	// calculamos el tamaño que debemos buscar
	// cuanto mas pequeño es el numero, mas tardara la deteccion
	// si no hay caras, pero podremos detectar objectos mas lejanos
	// para evitar lag en caso de un frame borroso, empezamos con un valor
	// alto (poco tiempo de calculo), y a partir de ahi buscaremos cada vez objetos
	// mas pequeños (mas tiempo)
	int minsize= 60 - d->wrongframes*5;
	// nos aseguramos de que no es un numero negativo
	if (minsize<0) minsize=0;

	faces = cvHaarDetectObjects( small_img, cascade, storage,
					  1.1, 3, 0
					  |CV_HAAR_FIND_BIGGEST_OBJECT
					  |CV_HAAR_DO_ROUGH_SEARCH
					  //|CV_HAAR_DO_CANNY_PRUNING
					  //|CV_HAAR_SCALE_IMAGE
					  ,cvSize(minsize, minsize) );
      }
      t = (double)cvGetTickCount() - t;
      //printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );

      // incluimos mensaje de tiempo de deteccion
      char text[64];
      sprintf( text, "Detection time = %gms", t/((double)cvGetTickFrequency()*1000.) );
      CvFont font = cvFont( 1, 1 );
      CvPoint text_origin;
      int base_line;
      CvSize text_size = {0,0};
      cvGetTextSize( text, &font, &text_size, &base_line );
      text_origin.x = 10; //img->width - text_size.width - 10;
      text_origin.y = img->height - base_line - 10;
      cvPutText( img, text, text_origin, &font, CV_RGB(255,255,255));

      CvPoint center;
      int radius;
      // bucle de procesado de caras
      
      for( i = 0; i < (faces ? faces->total : 0); i++ ){
	  CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
	  CvScalar color = colors[i%8];

	  // calculo de la posicion en la imagen original
	  if (d->singleuser && d->wrongframes==0){
	    color = colors[(i+1)%8];
	    // se esta usando recorte
	    // calculo de centro y radio
	    center.x = cvRound(  r->x + r->width*0.5 + d->lastframepos.x - d->lastradius*2);
	    center.y = cvRound(  r->y + r->height*0.5 + d->lastframepos.y - d->lastradius*1.5 );
	    radius = cvRound((r->width + r->height)*0.25);
	    
	    // calculo del rectangulo (si se esta usando)
	    CvPoint p1;
	    CvPoint p2;
	    p1.x=cvRound( d->lastframepos.x + d->lastradius*2);
	    p1.y=cvRound( d->lastframepos.y + d->lastradius*1.5);
	    p2.x=cvRound( d->lastframepos.x - d->lastradius*2);
	    p2.y=cvRound( d->lastframepos.y - d->lastradius*1.5);
	    cvRectangle(img, p1, p2 ,color,3,8,0);
	  }else{
	    // se está usando escalado
	    center.x = cvRound((r->x + r->width*0.5)*scale);
	    center.y = cvRound((r->y + r->height*0.5)*scale);
	    radius = cvRound((r->width + r->height)*0.25*scale);
	    
	  }
	  //printf("      x:%i y:%i r:%i \n", center.x, center.y, radius);

	  // Dibujamos el circulo
	  cvCircle( img, center, radius, color, 3, 8, 0 );

	  // guardamos la posicion para usarla con el siguiente frame
	  d->lastframepos.x = center.x;
	  d->lastframepos.y = center.y;
	  d->lastradius = radius;

	  // guardamos los datos
	  float* aux= new float[3];
	  aux[0]=atan(-(center.x-img->width/2.0)/d->focal_length[0]);
	  aux[1]=atan(-(center.y-img->height/2.0)/d->focal_length[1]);
	  // como tamaño guardamos el angulo de abertura del radio (para ser compatibles
	  // con 3dfrom2d
	  aux[2]=atan(radius/d->focal_length[0]);
	  if (i==0) // primera del report
	    (d->getdata())->setnewdata(aux);
	  else // siguientes
	    (d->getdata())->setmoredata(aux);
	  // liberamos la memoria del vector
	  free(aux);
      }
      // si no habia caras, no guardamos datos
      if (i==0){
	  (d->getdata())->setnodata();
      }
  }
  

  if (d->idnum()==firstinstance) cvShowImage( winname, img );
  cvReleaseImage( &gray );
  cvReleaseImage( &small_img );
  // si hemos detectado caras, reportamos y marcamos el flag d->lastframeok
  if (i>0){
    d->report();
    d->wrongframes=0;
  }else{
    d->nullreport();
    d->wrongframes++;
  }
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

