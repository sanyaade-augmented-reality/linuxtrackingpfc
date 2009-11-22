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
#include "OpenglView.h"
#include <GLC_3DViewInstance>
#include <GLC_Interpolator>
#include <GLC_Exception>
#include <GLC_SelectionMaterial>
#include <GLC_State>

// For VSYNC problem under Mac OS X
#if defined(Q_OS_MAC)
#include <OpenGL.h>
#endif

// List of usable shader
ShaderList OpenglView::m_ShaderList;

OpenglView::OpenglView(const bool useFrameBuffer, QWidget *pParent)
: QGLWidget(pParent)
, m_GlView(this)
, m_MoverController()
, m_World()
, m_Light()
, m_Mode(GL_FILL)
, m_ViewState(V_NORMAL)
, m_ViewEnterState(VE_NORMAL)
, m_MotionStep(20)
, m_CurrentFps(0)
, m_infoFont()
, m_SelectionMode(false)
, m_dislayInfoPanel(true)
, m_SnapShootMode(false)
, m_BlockSelection(false)
, m_GlobalShaderId(0)
, m_GlobalShaderName(tr("No Shader"))
, m_UseFrameBufferObject(useFrameBuffer)
, m_pQGLFramebufferObject(NULL)
, m_CaptureSize()
, m_SmoothCaptures(false)
, m_UiCollection()
, m_CurrentMoverType(GLC_MoverController::TrackBall)
{
	m_Light.setPosition(1.0, 1.0, 1.0);

	// Set backroundImage
	m_GlView.loadBackGroundImage(":images/default_background.png");

	//Use the default mover controller
	QColor repColor;
	repColor.setRgbF(1.0, 0.11372, 0.11372, 0.0);
	m_MoverController= GLC_Factory::instance(this->context())->createDefaultMoverController(repColor, &m_GlView);

	// Create other UI element
	GLC_3DViewInstance line= GLC_Factory::instance()->createLine(GLC_Point4d(), glc::X_AXIS);
	GLC_Material* pMaterial= new GLC_Material(Qt::red);
	line.geomAt(0)->addMaterial(pMaterial);
	m_UiCollection.add(line);
	line= GLC_Factory::instance()->createLine(GLC_Point4d(), glc::Y_AXIS);
	pMaterial= new GLC_Material(Qt::darkGreen);
	line.geomAt(0)->addMaterial(pMaterial);
	m_UiCollection.add(line);
	line= GLC_Factory::instance()->createLine(GLC_Point4d(), glc::Z_AXIS);
	pMaterial= new GLC_Material(Qt::blue);
	line.geomAt(0)->addMaterial(pMaterial);
	m_UiCollection.add(line);
	// PFC MOD Starts here
	// añadimos el cliente de tracker y lo enlazamos con la funcion de callback (ver mas arriba)
	ht_client= new TrackingPFC_client();
	ht_client->coordmode=TPFCCORD_GLC;
	// PFC MOD ends here

}

OpenglView::~OpenglView()
{
	GLC_SelectionMaterial::deleteShader();
	if (not m_ShaderList.isEmpty())
	{
		const int size= m_ShaderList.size();
		for (int i= 0; i < size; ++i)
		{
			delete m_ShaderList[i];
		}
		m_ShaderList.clear();
	}
}

//////////////////////////////////////////////////////////////////////
// Public Interface
//////////////////////////////////////////////////////////////////////

// Initialyze OpenGL window
void OpenglView::initializeGL()
{
	m_GlView.cameraHandle()->setDefaultUpVector(glc::Z_AXIS);
	m_GlView.initGl();

	// For VSYNC problem under Mac OS X
	#if defined(Q_OS_MAC)
	const GLint swapInterval = 1;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swapInterval);
	#endif

	glEnable(GL_NORMALIZE);

	emit glInitialed();

}

// Reframe the Scene
void OpenglView::reframe(const GLC_BoundingBox& boundingBox, bool motion)
{
	const GLC_BoundingBox collectionBox= m_World.collection()->boundingBox();
	if (boundingBox.isEmpty())
	{
		if (!collectionBox.isEmpty())
		{
			GLC_Camera savCam(*(m_GlView.cameraHandle()));
			m_GlView.reframe(collectionBox);
			if (motion)
			{
				GLC_Camera newCam(*(m_GlView.cameraHandle()));
				m_GlView.cameraHandle()->setCam(savCam);
				motionReframe(newCam, m_MotionStep);
			}
			else
			{
				m_GlView.setDistMinAndMax(collectionBox);
				updateGL();
				emit viewChanged();
			}
		}
	}
	else
	{
		GLC_Camera savCam(*(m_GlView.cameraHandle()));
		m_GlView.reframe(boundingBox);
		if (savCam == *(m_GlView.cameraHandle()))
		{
			m_GlView.reframe(collectionBox);
		}
		if (motion)
		{
			GLC_Camera newCam(*(m_GlView.cameraHandle()));
			m_GlView.cameraHandle()->setCam(savCam);
			motionReframe(newCam, m_MotionStep);
		}
		else
		{
			m_GlView.setDistMinAndMax(boundingBox);
			updateGL();
		}
	}
}


// Change The enter state of the view
void OpenglView::changeEnterState(ViewEnterState_enum state)
{
	if (m_ViewState == V_NORMAL)
	{
		m_ViewEnterState= state;
	}
}

// Set the view Camera
void OpenglView::setCameraAndAngle(const GLC_Camera& cam, const double& angle)
{
	*(m_GlView.cameraHandle())= cam;
	m_GlView.setViewAngle(angle);
	m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
	emit viewChanged();
}

// Init Iso view
void OpenglView::initIsoView()
{
	m_GlView.cameraHandle()->setIsoView();
	m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
	updateGL();
	emit viewChanged();
}

// Change the default camera Up axis
void OpenglView::changeDefaultUp()
{
	GLC_Vector4d upVector(m_GlView.cameraHandle()->defaultUpVector());
	// circular permutation
	if (upVector == glc::X_AXIS)
		upVector= glc::Y_AXIS;
	else if (upVector == glc::Y_AXIS)
		upVector= glc::Z_AXIS;
	else if (upVector == glc::Z_AXIS)
		upVector= glc::X_AXIS;

	m_GlView.cameraHandle()->setDefaultUpVector(upVector);
	m_GlView.cameraHandle()->setUpCam(upVector);
	updateGL();
}

// Iso view
void OpenglView::isoView1(bool motion)
{
	// Update position of the camera
	changeView(m_GlView.cameraHandle()->isoView(), motion);
}

// Iso view
void OpenglView::isoView2()
{
	GLC_Camera newCam= m_GlView.cameraHandle()->isoView();
	newCam.rotateAroundTarget(newCam.defaultUpVector(), glc::PI / 2.0);
	// Update position of the camera
	changeView(newCam);
}
// Iso view
void OpenglView::isoView3()
{
	GLC_Camera newCam= m_GlView.cameraHandle()->isoView();
	newCam.rotateAroundTarget(newCam.defaultUpVector(), glc::PI);
	// Update position of the camera
	changeView(newCam);
}
// Iso view
void OpenglView::isoView4()
{
	GLC_Camera newCam= m_GlView.cameraHandle()->isoView();
	newCam.rotateAroundTarget(newCam.defaultUpVector(), 3.0 * glc::PI / 2.0);
	// Update position of the camera
	changeView(newCam);
}

// Front view
void OpenglView::frontView()
{
	// Update position of the camera
	changeView(m_GlView.cameraHandle()->frontView());
}

// Left view
void OpenglView::rightView()
{
	changeView(m_GlView.cameraHandle()->rightView());
}
// Top view
void OpenglView::topView()
{
	changeView(m_GlView.cameraHandle()->topView());
}
// Zoom in
void OpenglView::zoomIn()
{
	int max= static_cast<int>(static_cast<double>(m_MotionStep) / 4);
	if (max == 0) max= 1;
	const double targetFactor= 1.5;
	const double factor= exp(log(targetFactor)/ max);
	m_World.collection()->setLodUsage(true, &m_GlView);
	for (int i= 0; i < max - 1; ++i)
	{
		m_GlView.cameraHandle()->zoom(factor);
		const GLC_BoundingBox collectionBox= m_World.collection()->boundingBox();
		m_GlView.setDistMinAndMax(collectionBox);
		updateGL();
	}
	m_World.collection()->setLodUsage(false, &m_GlView);
	m_GlView.cameraHandle()->zoom(factor);
	const GLC_BoundingBox collectionBox= m_World.collection()->boundingBox();
	m_GlView.setDistMinAndMax(collectionBox);
	updateGL();
	emit viewChanged();

}
// Zoom out
void OpenglView::zoomOut()
{
	int max= static_cast<int>(static_cast<double>(m_MotionStep) / 4);
	if (max == 0) max= 1;
	const double targetFactor= 1.0 / 1.5;
	const double factor= exp(log(targetFactor)/ max);

	for (int i= 0; i < max; ++i)
	{
		m_GlView.cameraHandle()->zoom(factor);
		const GLC_BoundingBox collectionBox= m_World.collection()->boundingBox();
		m_GlView.setDistMinAndMax(collectionBox);
		updateGL();
		emit viewChanged();
	}
}
// Select All instances
void OpenglView::selectAll()
{
	m_World.collection()->selectAll();
	updateGL();
	emit updateSelection(m_World.collection()->selection());
}

// unselect Instance
void OpenglView::unselectAllSlot()
{
	// if a geometry is selected, unselect it
	m_World.collection()->unselectAll();
	updateGL();
	emit unselectAll();
}

// Reframe on selection
void OpenglView::reframeOnSelection()
{
	GLC_BoundingBox SelectionBox;
	PointerNodeHash* pSelections= m_World.collection()->selection();
	PointerNodeHash::iterator iEntry= pSelections->begin();
    while (iEntry != pSelections->constEnd())
    {
    	SelectionBox.combine(iEntry.value()->boundingBox());
    	iEntry++;
    }
	reframe(SelectionBox, true);
}

// Select specified ID
void OpenglView::selectInstance(const GLC_uint SelectionID)
{
	m_World.collection()->select(SelectionID);
	qDebug() << "Selection size " << m_World.collection()->selection()->size();
	updateGL();
	emit updateSelection(m_World.collection()->selection());
}

// Change Current mover type
void OpenglView::changeCurrentMoverToTrackBall()
{
	m_CurrentMoverType= GLC_MoverController::TrackBall;
}

// Change Current mover type
void OpenglView::changeCurrentMoverToTurnTable()
{
	m_CurrentMoverType= GLC_MoverController::TurnTable;
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
void OpenglView::paintGL()
{
	//qDebug() << "PaintGL";
	QTime time;
	time.start();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	try
	{
		// Enable and execute lighting
		m_Light.enable();

		m_Light.glExecute();

		// PFC ALERT original code starts here
		// m_GlView.glExecuteCam();
		// PFC ALERT original code ends here

		// PFC MOD Starts here
		// para mantener los cambios en local no tocaremos la libreria glc
		// los archivos mencionados estan en glc_lib/viewport
		// la llamada original era a: (glvierport.h)
		/* void glExecuteCam(void){
			m_pViewCam->glExecute();
			glExecuteImagePlane();
		   }*/
		// que llama a: glc_camera.cpp
		/* void GLC_Camera::glExecute(){	
			gluLookAt(m_Eye.X(), m_Eye.Y(), m_Eye.Z(),
			m_Target.X(), m_Target.Y(), m_Target.Z(),
			m_VectUp.X(), m_VectUp.Y(), m_VectUp.Z());
		}*/

		// Primero necesitaremos hacer Cambios en el frustum ( ver GLC_Viewport::updateProjectionMat
		// en glc_lib/viewport/glviewport.cpp, aunque ahi se usa gluPerspective)
		// activamos la matriz de proyección
		//glMatrixMode(GL_PROJECTION);
		// Aqui ejecutamos el frustum
		m_GlView.updateProjectionMat(ht_client);
		
		// devolvemos la matriz activa a la de modelo, que es la que se esperaba en el codigo original
		//glMatrixMode(GL_MODELVIEW);
		
		// ejecutamos la camara
		ht_client->setvirtualdisplaysize(1000);
		m_GlView.glExecuteCam(ht_client);
		// PFC MOD Ends here
		

		// Test if there is a global shader
		if ((0 != m_GlobalShaderId) and not GLC_State::isInSelectionMode()) GLC_Shader::use(m_GlobalShaderId);

		// Display non transparent normal object
		m_World.glExecute(0, false);
		// Display non transparent instance of the shaders group
		if (GLC_State::glslUsed())
		{
			m_World.glExecuteShaderGroup(false);
		}

		// Display transparent normal object
		m_World.glExecute(0, true);
		// Display transparent instance of the shaders group
		if (GLC_State::glslUsed())
		{
			m_World.glExecuteShaderGroup(true);
		}

		// Display Selected Objects
		const int numberOfSelectedNode= m_World.collection()->selectionSize();
		if ((numberOfSelectedNode > 0) and GLC_State::selectionShaderUsed() and not GLC_State::isInSelectionMode())
		{
			if (numberOfSelectedNode != m_World.collection()->drawableObjectsSize())
			{
				//Draw the selection with Zbuffer
				m_World.glExecute(1);
			}
			// Draw the selection transparent
			glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	        glEnable(GL_CULL_FACE);
	        glEnable(GL_BLEND);
	        glDepthFunc(GL_ALWAYS);
	        glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	        m_World.glExecute(1);

    	    // Restore attributtes
	        glPopAttrib();
		}
		else if (numberOfSelectedNode > 0)
		{
			m_World.glExecute(1);
		}
		// Test if there is a global shader
		if (0 != m_GlobalShaderId) GLC_Shader::unuse();

		if (!m_SnapShootMode) // Don't display orbit circle in snapshootmode
		{
			m_GlView.glExecute();
			m_MoverController.drawActiveMoverRep();
		}

	}
	catch (GLC_Exception &e)
	{
		qDebug() << e.what();
	}

	updateFps(time.elapsed());
	if (!m_SelectionMode && m_dislayInfoPanel && !m_SnapShootMode)
	{
		// Display info area
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1,1,-1,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		displayInfo();

		// Restore 3DState
		glPopAttrib();
		glPopMatrix(); // restore modelview

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
}


void OpenglView::resizeGL(int width, int height)
{
	m_GlView.setWinGLSize(width, height);
}

void OpenglView::mousePressEvent(QMouseEvent * e)
{
	if ((m_ViewState == V_NORMAL) and ((m_ViewEnterState == VE_NORMAL)))
	{
		switch (e->button())
		{
			// Left Button Pressed
			case Qt::LeftButton:
				if (not m_BlockSelection)
				{
					bool multiSelection= ((e->modifiers() == Qt::ControlModifier) or (e->modifiers() == Qt::ShiftModifier));
					select(e->x(),e->y(), multiSelection);
				}
			break;

			// Right Button Pressed
			case Qt::RightButton:
			if (e->modifiers() == Qt::NoModifier)
			{
				m_MoverController.setActiveMover(m_CurrentMoverType, e->x(), e->y());
				setCursor(Qt::ClosedHandCursor);
				m_ViewState= V_ORBITING;
				//updateGL();
			}
			else if (e->modifiers() == Qt::ShiftModifier)
			{
				m_MoverController.setActiveMover(GLC_MoverController::Zoom, e->x(), e->y());
				setCursor(Qt::SizeVerCursor);
				m_ViewState= V_ZOOMING;
				//updateGL();
			}
			break;

			// Mid Button Pressed
			case Qt::MidButton:
			if (e->modifiers() == Qt::NoModifier)
			{
				m_MoverController.setActiveMover(GLC_MoverController::Pan, e->x(), e->y());
				setCursor(Qt::SizeAllCursor);
				m_ViewState= V_PANNING;
				//updateGL();
			}
			else if (e->modifiers() == Qt::ShiftModifier)
			{
				m_MoverController.setActiveMover(GLC_MoverController::Target, e->x(), e->y());
				m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
				updateGL();
				emit viewChanged();
			}

			break;
			// Default case
			default:
			break;
		}
	}
	else if ((m_ViewState == V_NORMAL) && (e->button() == Qt::LeftButton))
	{
		switch (m_ViewEnterState)
		{
			case VE_PANNING:
			m_MoverController.setActiveMover(GLC_MoverController::Pan, e->x(), e->y());
			setCursor(Qt::SizeAllCursor);
			m_ViewState= V_PANNING;
			//updateGL();
			break;

			case VE_ORBITING:
			m_MoverController.setActiveMover(m_CurrentMoverType, e->x(), e->y());
			setCursor(Qt::ClosedHandCursor);
			m_ViewState= V_ORBITING;
			//updateGL();
			break;

			case VE_POINTING:
			m_MoverController.setActiveMover(GLC_MoverController::Target, e->x(), e->y());
			m_MoverController.setNoMover();
			m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
			updateGL();
			break;

			case VE_ZOOMING:
			m_MoverController.setActiveMover(GLC_MoverController::Zoom, e->x(), e->y());
			setCursor(Qt::SizeVerCursor);
			m_ViewState= V_ZOOMING;
			//updateGL();
			break;

			default:
			break;

		}
	}

	QGLWidget::mousePressEvent(e);
}

void OpenglView::mouseReleaseEvent(QMouseEvent * e)
{
	if (m_ViewEnterState == VE_NORMAL)
	{
		switch (e->button())
		{
			// Left Button Released
			case Qt::LeftButton:
			// Perhaps selection...
			break;

			// Right Button Released
			case Qt::RightButton:
			if ((m_ViewState == V_ORBITING) or (m_ViewState == V_ZOOMING))
			{
				m_MoverController.setNoMover();
				unsetCursor();
				m_ViewState= V_NORMAL;
				updateGL();
			}
			break;

			// Mid Button Released
			case Qt::MidButton:
			if (m_ViewState == V_PANNING)
			{
				m_MoverController.setNoMover();
				unsetCursor();
				m_ViewState= V_NORMAL;
				updateGL();
			}
			break;

			// Default case
			default:
			break;
		}
	}
	else if((e->button() == Qt::LeftButton)
			and ((m_ViewEnterState == VE_PANNING)or(m_ViewEnterState == VE_ORBITING) or (m_ViewEnterState == VE_ZOOMING)))
	{
		m_MoverController.setNoMover();
		unsetCursor();
		m_ViewState= V_NORMAL;
		updateGL();
	}
	updateGL();

}
void OpenglView::mouseMoveEvent(QMouseEvent * e)
{
	if ((m_ViewState == V_ORBITING) or (m_ViewState == V_ZOOMING) or (m_ViewState == V_PANNING))
	{
		m_MoverController.move(e->x(), e->y());
		if (m_ViewState != V_PANNING)
		{
			// Calculate camera depth of view
			m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
		}
		m_World.collection()->setLodUsage(true, &m_GlView);
		updateGL();
		m_World.collection()->setLodUsage(false, &m_GlView);
		emit viewChanged();
	}
}
void OpenglView::wheelEvent(QWheelEvent *e)
{
	if (e->delta() > 0)
	{
		zoomIn();
	}
	else
	{
		zoomOut();
	}
}
void OpenglView::mouseDoubleClickEvent(QMouseEvent *e)
{
	if ((m_World.collection()->selectionSize() > 0) and (e->button() == Qt::LeftButton) and (e->modifiers() == Qt::NoModifier))
	{
		reframeOnSelection();
	}
}

// Rotate the camera softely with specified step
void OpenglView::rotateCamera(GLC_Camera newCam, const int step)
{
	// Save vectors of view's camera
	GLC_Point4d curEye(m_GlView.cameraHandle()->eye());
	GLC_Vector4d vectCurUp(m_GlView.cameraHandle()->upVector());
	GLC_Point4d curTarget(m_GlView.cameraHandle()->target());

	// Declare Rotations matrixs and rotaion interpolator
	GLC_Matrix4x4 matRot;
	GLC_Matrix4x4 matRotUp;
	GLC_Interpolator angleInterpolator;

	if ((m_GlView.cameraHandle()->camVector() == newCam.camVector()) and
		(m_GlView.cameraHandle()->upVector() == newCam.upVector()))
	{
		// View Camera is equal to destination camera, inverse destination view direction
		GLC_Vector4d newEye=(- (newCam.eye() - newCam.target()) + newCam.target());
		newCam.setCam(newEye, newCam.target(), newCam.upVector());
		const double angle= glc::PI / static_cast<double>(step);
		matRot.setMatRot(newCam.upVector(), angle);
	}
	else if ((m_GlView.cameraHandle()->camVector() == (-newCam.camVector())) and
		(m_GlView.cameraHandle()->upVector() == newCam.upVector()))
	{
		// View Camera is the inverse off destination camera
		const double angle= - glc::PI / static_cast<double>(step);
		matRot.setMatRot(newCam.upVector(), angle);
	}
	else
	{

		angleInterpolator.SetInterpolMat(step, (curEye - curTarget) , (newCam.eye() - newCam.target()), INTERPOL_ANGULAIRE);
		matRot= angleInterpolator.GetInterpolMat();
		// VecteurUp
		angleInterpolator.SetVecteurs(vectCurUp, newCam.upVector());
		matRotUp= angleInterpolator.GetInterpolMat();

	}

	for (int i= 0; i < step - 1; ++i)
	{
		curEye= (matRot * (curEye - curTarget)) + curTarget;
		vectCurUp= matRotUp * vectCurUp;

		m_GlView.cameraHandle()->setCam(curEye, curTarget, vectCurUp);
		m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
		updateGL();
	}

	m_GlView.cameraHandle()->setCam(newCam.eye(), newCam.target(), newCam.upVector());
	m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
	updateGL();
	emit viewChanged();

}
// reframe softely with specified step
void OpenglView::motionReframe(const GLC_Camera newCam, const int step)
{
	if ((m_GlView.cameraHandle()->eye() != newCam.eye()) ||
		(m_GlView.cameraHandle()->target() != newCam.target()))
		{
			GLC_Point4d curEye(m_GlView.cameraHandle()->eye());
			GLC_Point4d curTarget(m_GlView.cameraHandle()->target());
			const GLC_Vector4d vectCurUp(m_GlView.cameraHandle()->upVector());

			GLC_Interpolator transInterpolator;
			transInterpolator.SetInterpolMat(step, curEye, newCam.eye(), INTERPOL_LINEAIRE);
			GLC_Matrix4x4 matEye(transInterpolator.GetInterpolMat());
			transInterpolator.SetVecteurs(curTarget, newCam.target());
			GLC_Matrix4x4 matTarget(transInterpolator.GetInterpolMat());
			m_World.collection()->setLodUsage(true, &m_GlView);
			for (int i= 0; i < step - 1; ++i)
			{
				curEye= matEye * curEye;
				curTarget = matTarget * curTarget;

				m_GlView.cameraHandle()->setCam(curEye, curTarget, vectCurUp);
				m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
				updateGL();
			}
			m_World.collection()->setLodUsage(false, &m_GlView);
			m_GlView.cameraHandle()->setCam(newCam.eye(), newCam.target(), newCam.upVector());
			m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
			updateGL();
			emit viewChanged();

		}
}
// Update the current fps
void OpenglView::updateFps(int elapsed)
{
	static const int max= 20;
	static int fpsVector[max];
	static int j=0;
	float averageFps= 0.0f;


  	if (elapsed > 0)
  	{
    	fpsVector[j]= elapsed;
	  	j= (j + 1) % max;
  	}
	for (int i= 0;i < max;++i)
	{
		averageFps+= fpsVector[i];
	}
	averageFps= averageFps / static_cast<float>(max);
	m_CurrentFps= static_cast<int>(1000.0f / averageFps);
	int relativeFps;

	// The maximum relative fps must be under 60
	// Cause VSSYNC
	if (m_CurrentFps > 60) relativeFps= 60;
	else relativeFps= m_CurrentFps;

	m_MotionStep= static_cast<int>(static_cast<float>(relativeFps) / 3);
	if (0 == m_MotionStep) m_MotionStep= 1;
}

// Display info panel
void OpenglView::displayInfo()
{
	QSize textPosition(70,5);
	QSize screenSize(size());
	int screenHeight= screenSize.height();
	float panelRatio= static_cast<float>(screenHeight - 42) / screenHeight;
	double displayRatio= static_cast<double>(screenSize.height()) / static_cast<double>(screenSize.width());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	QColor areaColor(5, 5, 50, 128);
	qglColor(areaColor);

	glBegin(GL_QUADS);
		glVertex2f(-1.f,-panelRatio); glVertex2f( 1.f,-panelRatio);
		glVertex2f( 1.f,-1.f); glVertex2f(-1.f,-1.f);
	glEnd();

	// Display the frame rate
	qglColor(Qt::white);
	m_infoFont.setPixelSize(12);
	screenSize-= textPosition;
	QString fps(QString::number(m_CurrentFps) + QString(" Fps"));
	renderText(screenSize.width(), screenSize.height(), fps, m_infoFont);
	// If there is one selected object, display its name
	if(m_World.collection()->selection()->size() == 1)
	{
		QString selectionName(m_World.collection()->selection()->begin().value()->name());
		// truncate the string to 50 characters
		selectionName= selectionName.left(50);
		renderText(10, screenSize.height(), selectionName, m_infoFont);
	}
	else if (GLC_State::glslUsed())
	{
		QString currentShaderName(tr("Global Shader : ") + m_GlobalShaderName);
		currentShaderName= currentShaderName.left(50);
		renderText(10, screenSize.height(), currentShaderName, m_infoFont);
	}

	// Display Other UI element
	// Change the current matrix



	GLC_Matrix4x4 uiMatrix(m_GlView.cameraHandle()->viewMatrix());
	uiMatrix.invert();
	// Change matrix to follow camera orientation
	const double scaleFactor= 0.08;
	glTranslated(1.0 - (scaleFactor * displayRatio * 1.4), - panelRatio + (scaleFactor * 1.3), 0.0);
	glScaled(scaleFactor * displayRatio, scaleFactor, scaleFactor);
	glMultMatrixd(uiMatrix.data());

	qglColor(Qt::red);
	renderText(1.0, 0.0, 0.0, "X");
	qglColor(Qt::darkGreen);
	renderText(0.0, 1.0, 0.0, "Y");
	qglColor(Qt::blue);
	renderText(0.0, 0.0, 1.0, "Z");
	m_UiCollection.glExecute(0, false);


	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{// QT render text cause an openGL error
		QString msg(tr("Sorry, Display info panel not supported on your computer, restarting GLC_Player"));
		QMessageBox::warning(this, tr("Warning"), msg, QMessageBox::Ok);
		m_dislayInfoPanel= false;
		emit hideInfoPanel();
	}
}

// Select
void OpenglView::select(int x, int y, bool multiSelection)
{
	m_SelectionMode= true;
	setAutoBufferSwap(false);
	GLC_uint SelectionID= m_GlView.select(this, x, y);
	//qDebug() << "Selection Id: " << SelectionID;
	m_SelectionMode= false;
	//updateGL();
	setAutoBufferSwap(true);

	if (m_World.collection()->contains(SelectionID))
	{

		if ((not m_World.collection()->isSelected(SelectionID)) and (m_World.collection()->selectionSize() > 0) and (not multiSelection))
		{
			m_World.collection()->unselectAll();
			emit unselectAll();
		}
		if (not m_World.collection()->isSelected(SelectionID))
		{
			m_World.collection()->select(SelectionID);
			updateGL();
			emit updateSelection(m_World.collection()->selection());
		}
		else if (m_World.collection()->isSelected(SelectionID) and multiSelection)
		{
			m_World.collection()->unselect(SelectionID);
			updateGL();
			emit updateSelection(m_World.collection()->selection());
		}
		else
		{
			m_World.collection()->unselectAll();
			m_World.collection()->select(SelectionID);
			updateGL();
			emit updateSelection(m_World.collection()->selection());
		}
	}
	else if ((m_World.collection()->selectionSize() > 0) and (not multiSelection))
	{
		// if a geometry is selected, unselect it
		m_World.collection()->unselectAll();
		updateGL();
		emit unselectAll();
	}

}

// Change the current view
void OpenglView::changeView(GLC_Camera newCam, bool motion)
{
	newCam.setDistEyeTarget(m_GlView.cameraHandle()->distEyeTarget());
	if (motion)
	{
		rotateCamera(newCam, m_MotionStep);
	}
	else
	{
		rotateCamera(newCam, 0);
	}
}

void OpenglView::setDistMinAndMax()
{
	m_GlView.setDistMinAndMax(m_World.collection()->boundingBox());
}
// Swap visible space
void OpenglView::swapVisibleSpace()
{
	m_World.collection()->swapShowState();
	if (m_World.collection()->showState())
	{
		m_GlView.loadBackGroundImage(":images/default_background.png");
	}
	else
	{
		m_GlView.loadBackGroundImage(":images/NoShow_background.png");
	}
}
// set the view to visible state
void OpenglView::setToVisibleState()
{
	m_GlView.loadBackGroundImage(":images/default_background.png");
}
// set the view to visible state
void OpenglView::setToInVisibleState()
{
	m_GlView.loadBackGroundImage(":images/NoShow_background.png");
}

// Take a Screenshot of the current view
QImage OpenglView::takeScreenshot(const bool tryToUseFrameBuffer, const QSize& targetSize, const QString& backImageName, const QColor& backColor, double aspectRatio)
{
	// Test if framebuffer must be used
	bool useFrameBuffer;
	if ((tryToUseFrameBuffer) and m_UseFrameBufferObject)
	{
		// Use frame buffer
		useFrameBuffer= true;
	}
	else
	{
		useFrameBuffer= false;
	}
	QImage imageToSave;

	setAutoBufferSwap(false);
	setSnapShootMode(true);

	// Test if the background must be changed
	if (not backImageName.isEmpty())
	{
		// if m_CurrentBackgroundImageName is not empty, the file exist and is readable
		m_GlView.loadBackGroundImage(backImageName);

	}
	else if (backColor.isValid())
	{
		m_GlView.setBackgroundColor(backColor);
		m_GlView.deleteBackGroundImage();

	}

	if (useFrameBuffer)
	{
		// Create the framebuffer
		QGLFramebufferObject framebufferObject(targetSize, QGLFramebufferObject::Depth);
		framebufferObject.bind();
		m_GlView.setWinGLSize(framebufferObject.width(), framebufferObject.height());
		updateGL();
		imageToSave= framebufferObject.toImage();
		framebufferObject.release();
		m_GlView.setWinGLSize(size().width(), size().height());
	}
	else
	{
		// Change view aspect ratio and
		m_GlView.forceAspectRatio(aspectRatio);

		updateGL();
		imageToSave=  grabFrameBuffer();
		QSize shotSize= imageToSave.size();
		imageToSave= imageToSave.scaled(static_cast<int>(shotSize.height() * aspectRatio), shotSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		imageToSave= imageToSave.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		// To match the exact size of image
		imageToSave= imageToSave.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}

	// Retore the view
	if (isInShowState())
	{
		setToVisibleState();
	}
	else
	{
		setToInVisibleState();
	}

	setAutoBufferSwap(true);
	setSnapShootMode(false);
	// PFC MOD Starts here
	//m_GlView.updateProjectionMat();
	m_GlView.updateProjectionMat(ht_client);
	// PFC MOD ends here
	updateGL();

	return imageToSave;
}

// Change the view state to capture mode
void OpenglView::captureMode(const QSize& captureSize, bool smooth, const QColor& color)
{
	m_SmoothCaptures= smooth;
	m_CaptureSize.setWidth(captureSize.width());
	m_CaptureSize.setHeight(captureSize.height());
	if (color.isValid())
	{
		m_GlView.setBackgroundColor(color);
		m_GlView.deleteBackGroundImage();
	}
	else
	{
		setToVisibleState();
	}
	setAutoBufferSwap(false);
	setSnapShootMode(true);

	if (m_UseFrameBufferObject)
	{
		QSize realSize(m_CaptureSize);
		if (m_SmoothCaptures)
		{
			realSize.setWidth(m_CaptureSize.width() * 4);
			realSize.setHeight(m_CaptureSize.height() * 4);
		}
		delete m_pQGLFramebufferObject;
		// Create the framebuffer
		m_pQGLFramebufferObject= new QGLFramebufferObject(realSize, QGLFramebufferObject::Depth);
	}
}

// Return a screenshot of the current view in capture mode
QImage OpenglView::takeScreenshot()
{
	QImage imageToSave;

	if (m_UseFrameBufferObject)
	{
		m_pQGLFramebufferObject->bind();
		m_GlView.setWinGLSize(m_pQGLFramebufferObject->size().width(), m_pQGLFramebufferObject->size().height());
		updateGL();
		imageToSave= m_pQGLFramebufferObject->toImage();
		if (m_SmoothCaptures)
		{
			imageToSave= imageToSave.scaled(m_CaptureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		m_pQGLFramebufferObject->release();
	}
	else
	{
		const double aspectRatio= static_cast<double>(m_CaptureSize.width()) / static_cast<double>(m_CaptureSize.height());
		m_GlView.forceAspectRatio(aspectRatio);
		updateGL();
		imageToSave=  grabFrameBuffer();
		QSize shotSize= imageToSave.size();
		imageToSave= imageToSave.scaled(static_cast<int>(shotSize.height() * aspectRatio), shotSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		imageToSave= imageToSave.scaled(m_CaptureSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		// To match the exact size of image
		imageToSave= imageToSave.scaled(m_CaptureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}

	return imageToSave;
}

// Change the state to normal mode
void OpenglView::normalMode()
{
	if (m_UseFrameBufferObject)
	{
		m_GlView.setWinGLSize(size().width(), size().height());
		delete m_pQGLFramebufferObject;
		m_pQGLFramebufferObject= NULL;
	}

	if (isInShowState())
	{
		setToVisibleState();
	}
	else
	{
		setToInVisibleState();
	}

	setAutoBufferSwap(true);
	setSnapShootMode(false);
	// PFC MOD Starts here
	//m_GlView.updateProjectionMat();
	m_GlView.updateProjectionMat(ht_client);
	// PFC MOD ends here
	updateGL();
}

// Initialize shader list
void OpenglView::initShaderList()
{
	if (GLC_State::glslUsed() and not GLC_State::selectionShaderUsed())
	{
		GLC_State::setSelectionShaderUsage(true);
		// Set selection Shader
		QFile vertexShaderFile(":shaders/select.vert");
		QFile fragmentShaderFile(":shaders/select.frag");
		GLC_SelectionMaterial::setShaders(vertexShaderFile, fragmentShaderFile);
		// Initialize selection shader
		GLC_SelectionMaterial::initShader();

		if (m_ShaderList.isEmpty())
		{
			GLC_Shader* pShader= new GLC_Shader();
			// Toon Shader
			QFile vertexToonShaderFile(":shaders/toon.vert");
			QFile fragmentToonShaderFile(":shaders/toon.frag");
			pShader->setVertexAndFragmentShader(vertexToonShaderFile, fragmentToonShaderFile);
			pShader->createAndCompileProgrammShader();
			m_ShaderList.append(pShader);

			// Minnaert Shader
			pShader= new GLC_Shader();
			QFile vertex2ShaderFile(":shaders/minnaert.vert");
			QFile fragment2ShaderFile(":shaders/minnaert.frag");
			pShader->setVertexAndFragmentShader(vertex2ShaderFile, fragment2ShaderFile);
			pShader->createAndCompileProgrammShader();
			m_ShaderList.append(pShader);

			// Gooch Shader
			pShader= new GLC_Shader();
			QFile vertex3ShaderFile(":shaders/goochShading.vert");
			QFile fragment3ShaderFile(":shaders/goochShading.frag");
			pShader->setVertexAndFragmentShader(vertex3ShaderFile, fragment3ShaderFile);
			pShader->createAndCompileProgrammShader();
			m_ShaderList.append(pShader);
		}
	}

}



