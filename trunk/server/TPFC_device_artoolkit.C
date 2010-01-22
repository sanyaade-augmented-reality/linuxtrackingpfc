#include "TPFC_device_artoolkit.h" 

// incializacion de la variable estatica
int TPFC_device_artoolkit::firstinstance=-1;

// Creadora
TPFC_device_artoolkit::TPFC_device_artoolkit(int ident,int argc, char **argv):TPFC_device(ident){
  // creamos los datos
  data = new TrackingPFC_data(TrackingPFC_data::TPFCDATA3DORI,1);

  // si somos el primer device de este tipo, marcamos firstinstance
  if (firstinstance==-1){
    firstinstance=ident;
    // inicializamos el glutInit para no tener que pasar argc y argv al thread
    glutInit(&argc, argv);
    // lanzamos el thread
    pthread_create( &art_thread, NULL, art_main,this);
  }else{
    // como hay problemas al lanzar 2 instancias de artoolkit, avisamos al usuario
    printf("Solo se permite un dispositivo del tipo artoolkit.\n");
    // y detenemos el dispositivo
    running= STOP;
  }
  
}

// Destructora
TPFC_device_artoolkit::~TPFC_device_artoolkit(){
  free(data);
}

// Sobrecarga de stop, para esperar al thread
void TPFC_device_artoolkit::stop(){
   running= STOP;
   pthread_join( art_thread, NULL);
}


// Informacion sobre el dispositivo
string TPFC_device_artoolkit::info(){
  char aux[64]; // si se crean MUCHOS dispositivos nos saldremos del buffer... dudo que sea un problema
  sprintf(aux, "Artoolkit");
  return aux;
}

// funcion que comprueba si el dispositivo s es una fuente valida para este dispositivo
// devuelve "ok" si es correcta, o una string con la informacion relevante si no lo es
string TPFC_device_artoolkit::checksource(TPFC_device* s){
  string ret = "Este dispositivo no acepta fuentes.";
  return ret;
}



/* cleanup function called when program exits */
void TPFC_device_artoolkit::cleanup(void){
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}


void TPFC_device_artoolkit::draw( double trans[3][4] , int patt_id){
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]       = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
    
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    /* load the camera transformation matrix */
    argConvGlpara(trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 0.0, 0.0, 25.0 );
    glutSolidCube(50.0);
    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}


/* main loop */
int TPFC_device_artoolkit::mainLoop(int patt_id, int count){
    static int      contF = 0;
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             j, k;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return count;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    argDrawMode2D();
    argDispImage( dataPtr, 0,0 );

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, 100, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }

    arVideoCapNext();

    /* check for object visibility */
    k = -1;
    for( j = 0; j < marker_num; j++ ) {
        if( patt_id == marker_info[j].id ) {
            if( k == -1 ) k = j;
            else if( marker_info[k].cf < marker_info[j].cf ) k = j;
        }
    }
    if( k == -1 ) {
        contF = 0;
        argSwapBuffers();
        return count;
    }

    /* get the transformation between the marker and the real camera */
    int             patt_width     = 80.0;
    double          patt_center[2] = {0.0, 0.0};
    double          patt_trans[3][4];
    arGetTransMatCont(&marker_info[k], patt_trans, patt_center, patt_width, patt_trans);
    
    contF = 1;

    draw( patt_trans , patt_id);

    argSwapBuffers();
    return count;
}


// cuerpo principal del thread
void* TPFC_device_artoolkit::art_main(void * t){

    int xsize, ysize;
    ARParam cparam;
    int patt_id;

    // init
    ARParam  wparam;

    /* open the video path */
    if( arVideoOpen( "" ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    // obtenemos la ruta del archivo
    string cam_dat;
    if (!getconfig("artoolkitcam", &cam_dat) ){
      printf("No se han podido leer el path del archivo de calibracion en la configuracion, abortando device.\n");
      ((TPFC_device*)t)->stop();
    }
    if( arParamLoad(cam_dat.c_str(), 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }

    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );
    
    // obtenemos la ruta del archivo
    string pattern_path;
    if (!getconfig("artoolkitpat", &pattern_path) ){
      printf("No se han podido leer el path del archivo de patron en la configuracion, abortando device.\n");
      ((TPFC_device*)t)->stop();
    }
    if( (patt_id=arLoadPatt(pattern_path.c_str())) < 0 ) {
        printf("pattern load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 0, 0, 0 );

    // fin de init

    arVideoCapStart();


    int count=0;
    while (1)
      count=mainLoop(patt_id, count);

}
