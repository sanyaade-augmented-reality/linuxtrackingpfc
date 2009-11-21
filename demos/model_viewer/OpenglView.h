/****************************************************************************

 This file is part of GLC-Player.
 Copyright (C) 2007-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.1.0, packaged on September 2009.

 http://www.glc-player.net

 GLC-Player is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-Player is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-Player; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

#ifndef OPENGLVIEW_H_
#define OPENGLVIEW_H_

#include <GLC_Viewport>
#include <GLC_3DViewCollection>
#include <GLC_Light>
#include <GLC_Factory>
#include <GLC_Shader>
#include <GLC_MoverController>
#include <QGLWidget>
#include <QFile>
// PFC MOD starts here
// incluimos las cabeceras necesarias para el cliente de tracking
#include <TrackingPFC_client.h>
// PFC MOD ends here

// The State of OpenGL view
enum ViewState_enum
{
	V_NORMAL= 1, V_ORBITING, V_PANNING, V_ZOOMING
};

enum ViewEnterState_enum
{
	VE_NORMAL= 10, VE_PANNING, VE_ORBITING, VE_POINTING, VE_ZOOMING
};
typedef QList<GLC_Shader*> ShaderList;

//////////////////////////////////////////////////////////////////////
//! \class OpenglView
/*! \brief OpenglView : The OpenGl view of GLC_Player*/
//////////////////////////////////////////////////////////////////////
class OpenglView : public QGLWidget
{
	Q_OBJECT

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	OpenglView(const bool, QWidget *);
	virtual ~OpenglView();
//@}

//////////////////////////////////////////////////////////////////////
// Public Static Interface
//////////////////////////////////////////////////////////////////////
public:
	//! get the shader list
	inline static ShaderList shaderList()
	{return m_ShaderList;}

//////////////////////////////////////////////////////////////////////
// Public Interface
//////////////////////////////////////////////////////////////////////
public:
	//! Initialize OpenGL window
	void initializeGL();
	//! Reframe the Scene
	void reframe(const GLC_BoundingBox&, bool);
	//! Change The enter state of the view
	void changeEnterState(ViewEnterState_enum);
	//! Clear the view
	inline void clear() {m_World= GLC_World();}
	//! Add World in the view
	inline void add(GLC_World& world) {m_World= world;}
	//! Retrieve the view world
	inline GLC_World getWorld() {return m_World;}
	//! Get Mode
	inline GLenum getMode() const {return m_Mode;}
	//! Set Mode
	inline void setMode(GLenum mode)
	{
		m_Mode= mode;
		m_World.collection()->setPolygonModeForAll(GL_FRONT_AND_BACK, mode);
	}
	//! bool isEmpty
	inline bool isEmpty() {return m_World.collection()->isEmpty();}
	//! Get the Viewport
	inline GLC_Viewport* getViewport() {return &m_GlView;}
	//! Get the view Camera
	inline GLC_Camera getCamera() const {return *(m_GlView.cameraHandle());}
	//! Get the view angle
	inline double getViewAngle() const {return m_GlView.viewAngle();}
	//! Set the view Camera
	void setCameraAndAngle(const GLC_Camera&, const double&);
	//! Set AutoBufferSwap
	inline void setAutoBufferSwap(bool on) {QGLWidget::setAutoBufferSwap(on);}
	//! set Info Panel visibility
	inline void setDisplayInfoPanel(bool vis){m_dislayInfoPanel= vis;}
	//! set the snapshoot Mode
	inline void setSnapShootMode(bool mode){m_SnapShootMode= mode;}
	//! set the view min and max distance
	void setDistMinAndMax();
	//! Get Opengl view light
	inline GLC_Light* getLight() {return &m_Light;}
	//! Swap visible space
	void swapVisibleSpace();
	//! get the view visible state
	inline bool getVisibleState() {return m_World.collection()->showState();}
	//! set the view to visible state
	void setToVisibleState();
	//! set the view to visible state
	void setToInVisibleState();
	//! Set Blocking selection
	inline void blockSelection(const bool flag) {m_BlockSelection= flag;}
	//! Get the shader list
	inline ShaderList* getShaderListHandle()
	{return &m_ShaderList;}
	//! Set the global shader id
	inline void setGlobalShaderId(GLuint id, const QString& name)
	{
		m_GlobalShaderId= id;
		if (not name.isEmpty()) m_GlobalShaderName= name;
	}
	//! return the global shader id
	inline GLuint globalShaderId() const {return m_GlobalShaderId;}

	//! Return true if frame buffer can be used
	inline bool useFrameBuffer() const {return m_UseFrameBufferObject;}

	//! Return true if the view is in show state
	inline bool isInShowState() {return m_World.collection()->showState();}

	//! Take a Screenshot of the current view
	QImage takeScreenshot(const bool, const QSize&, const QString&, const QColor&, double);

	//! Change the view state to capture mode
	void captureMode(const QSize&, bool, const QColor&);

	//! Return a screenshot of the current view in capture mode
	QImage takeScreenshot();

	//! Change the state to normal mode
	void normalMode();

	//! Get the view State
	inline ViewState_enum viewState() const {return  m_ViewState;}

	//! Initialize shader list
	void initShaderList();

	//! Init Iso view
	void initIsoView();


public slots:
	// Change the default camera Up axis
	void changeDefaultUp();
	// Iso view
	void isoView1(bool motion= true);
	// Iso view
	void isoView2();
	// Iso view
	void isoView3();
	// Iso view
	void isoView4();
	// Front View
	void frontView();
	// Left View
	void rightView();
	// Top View
	void topView();
	// Zoom in
	void zoomIn();
	// Zoom out
	void zoomOut();
	//! Select All instances
	void selectAll();
	//! unselect All instance
	void unselectAllSlot();
	//! Reframe on selection
	void reframeOnSelection();
	//! Select specified ID
	void selectInstance(const GLC_uint);
	//! Change Current mover to track ball mover
	void changeCurrentMoverToTrackBall();
	//! Change Current mover to turn table mover
	void changeCurrentMoverToTurnTable();


//////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////
signals:
	//! Progression of loading file quantum
	void currentQuantum(int);
	void updateSelection(PointerNodeHash *);
	void unselectAll();
	void hideInfoPanel();
	void viewChanged();
	void glInitialed();

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
public:
	void paintGL();
private:
	void resizeGL(int width, int height);
	//Mouse events
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	//! Rotate the camera softely with specified step
	void rotateCamera(const GLC_Camera, const int);
	//! reframe softely with specified step
	void motionReframe(const GLC_Camera, const int);
	//! Update the current fps
	void updateFps(int);
	//! Display info panel
	void displayInfo();
	//! Select
	void select(int, int, bool);
	//! Change the current view
	void changeView(GLC_Camera, bool motion= true);

//////////////////////////////////////////////////////////////////////
// Private static members
//////////////////////////////////////////////////////////////////////
private:

	//! List of usable shader
	static ShaderList m_ShaderList;

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	GLC_Viewport m_GlView;
	GLC_MoverController m_MoverController;
	GLC_World m_World;
	GLC_Light m_Light;
	GLenum m_Mode;
	ViewState_enum m_ViewState;
	ViewEnterState_enum m_ViewEnterState;
	int m_MotionStep;
	// fps calcul
	int m_CurrentFps;
	QFont m_infoFont;
	//! bool in selection mode
	bool m_SelectionMode;
	bool m_dislayInfoPanel;
	bool m_SnapShootMode;
	bool m_BlockSelection;
	// PFC MOD starts here
	// añadimos el cliente de headtracking
	TrackingPFC_client* ht_client;
	// PFC MOD ends here
	
	//! Global shader
	GLuint m_GlobalShaderId;
	QString m_GlobalShaderName;

	//! the frame buffer is used and supported
	const bool m_UseFrameBufferObject;

	//! The framebuffer associated to the view
	QGLFramebufferObject* m_pQGLFramebufferObject;

	//! The current capture size
	QSize m_CaptureSize;

	//! Smoth capture
	bool m_SmoothCaptures;

	//! The collection of other User interface object
	GLC_3DViewCollection m_UiCollection;

	//! The current mover type (Trackball or Turn table)
	int m_CurrentMoverType;
};

#endif /*OPENGLVIEW_H_*/
