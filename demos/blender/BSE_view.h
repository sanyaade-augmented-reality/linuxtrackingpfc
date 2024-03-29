/**
 * $Id: BSE_view.h 18840 2009-02-06 19:21:24Z blendix $
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 * protos for view.c  -- not complete
 */

#ifndef BSE_VIEW_H
#define BSE_VIEW_H

struct Object;
struct BoundBox;
struct View3D;
struct ScrArea;

typedef struct ViewDepths {
	unsigned short w, h;
	float *depths;
	double depth_range[2];

	char damaged;
} ViewDepths;

#define PERSP_WIN	0
#define PERSP_VIEW	1
#define PERSP_STORE	2

void persp_general(int a);
void persp(int a);

/* note, the define below is still used for shorts, to calc distances... */
#define IS_CLIPPED	12000
void view3d_get_object_project_mat(struct ScrArea *area, struct Object *ob, float pmat[4][4], float vmat[4][4]);
void view3d_project_float(struct ScrArea *area, float *vec, float *adr, float mat[4][4]);
void view3d_project_short_clip(struct ScrArea *area, float *vec, short *adr, float projmat[4][4], float viewmat[4][4]);
void view3d_project_short_noclip(struct ScrArea *area, float *vec, short *adr, float mat[4][4]);

void initgrabz(float x, float y, float z);
void window_to_3d(float *vec, short mx, short my);
void project_short(float *vec, short *adr);
void project_short_noclip(float *vec, short *adr);
void project_int(float *vec, int *adr);
void project_int_noclip(float *vec, int *adr);
void project_float(float *vec, float *adr);
void project_float_noclip(float *vec, float *adr);
void viewray(short mval[2], float ray_start[3], float ray_normal[3]);
void viewline(short mval[2], float ray_start[3], float ray_end[3]);

int boundbox_clip(float obmat[][4], struct BoundBox *bb);
void fdrawline(float x1, float y1, float x2, float y2);
void fdrawbox(float x1, float y1, float x2, float y2);
void sdrawline(short x1, short y1, short x2, short y2);
void sdrawbox(short x1, short y1, short x2, short y2);
void calctrackballvecfirst(struct rcti *area, short *mval, float *vec);
void calctrackballvec(struct rcti *area, short *mval, float *vec);
void viewmove(int mode);
void viewmoveNDOFfly(int mode);
void viewmoveNDOF(int mode);
void view_zoom_mouseloc(float dfac, short *mouseloc);
int view_mouse_depth( float mouse_worldloc[3], short mval[2], int dist);

int get_view3d_ortho(struct View3D *v3d);
int get_view3d_viewplane(int winxi, int winyi, rctf *viewplane, float *clipsta, float *clipend, float *pixsize);
// PFC MOD Starts here
// estructura de comunicacion entre las 2 funciones
typedef struct tpfcinfo{
  float originalz; // ajuste que hay que aplicar a obsz
  int htactive; // flag de ht activo
}tpfcinfo;
// setwinmatrix aparte de modificar el frustum ahora devuelve un tpfcres
tpfcinfo setwinmatrixview3d(int winx, int winy, struct rctf *rect);
//void setwinmatrixview3d(int winx, int winy, struct rctf *rect);
// para mantener compatibilidad, setviewmatrixview3d(void); se mantiene inalterada
// pero se crea una version especial, que acepta como parametro un tpfcinfo
void tpfcsetviewmatrixview3d(tpfcinfo);
//PFC MOD ends here

void obmat_to_viewmat(struct Object *ob, short smooth);
void setviewmatrixview3d(void);
float *give_cursor(void);
unsigned int free_localbit(void);
void initlocalview(void);
void centerview(void);
void restore_localviewdata(struct View3D *vd);
void endlocalview(struct ScrArea *sa);
void view3d_home(int center);
short view3d_opengl_select(unsigned int *buffer, unsigned int buffsize, short x1, short y1, short x2, short y2);
void view3d_align_axis_to_vector(struct View3D *v3d, int axisidx, float vec[3]);

#endif

