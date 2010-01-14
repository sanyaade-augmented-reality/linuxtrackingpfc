#!/bin/bash
# para incluir el tracker en la declaracion de G
cp sources/blenkernel/BKE_global.h ./
# para inicializar el tracker a la vez que G
cp sources/blenkernel/intern/blender.c ./
# para destruir el tracker antes de salir, sin provocar un segfault
cp sources/src/usiblender.c ./

cp sources/include/BIF_mywindow.h ./
cp sources/src/mywindow.c ./

cp sources/include/BSE_view.h ./
cp sources/src/view.c ./

cp sources/src/drawview.c ./
