/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Laurent Bauer
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_camera.cpp Implementation of the GLC_Camera class.

#include "glc_camera.h"

#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////
GLC_Camera::GLC_Camera()
: GLC_Object("Camera")
, m_Eye(0,0,1)
, m_Target()
, m_VectUp(Y_AXIS)
, m_MatCompOrbit()
, m_DefaultVectUp(Y_AXIS)
{

}

GLC_Camera::GLC_Camera(const GLC_Point4d &Eye, const GLC_Point4d &Target, const GLC_Vector4d &Up)
: GLC_Object("Camera")
, m_Eye()
, m_Target()
, m_VectUp()
, m_MatCompOrbit()
, m_DefaultVectUp(Y_AXIS)

{
	setCam(Eye, Target, Up);
	createMatComp();
}

// Copy constructor
GLC_Camera::GLC_Camera(const GLC_Camera& cam)
: GLC_Object(cam)
, m_Eye(cam.m_Eye)
, m_Target(cam.m_Target)
, m_VectUp(cam.m_VectUp)
, m_MatCompOrbit(cam.m_MatCompOrbit)
, m_DefaultVectUp(cam.m_DefaultVectUp)
{

}

/////////////////////////////////////////////////////////////////////
// Get Functions
/////////////////////////////////////////////////////////////////////

// equality operator
bool GLC_Camera::operator==(const GLC_Camera& cam) const
{
	return (m_Eye == cam.m_Eye) && (m_Target == cam.m_Target)
			&& (m_VectUp == cam.m_VectUp) && (m_DefaultVectUp == cam.m_DefaultVectUp);
}


/////////////////////////////////////////////////////////////////////
// Set Functions
/////////////////////////////////////////////////////////////////////
GLC_Camera& GLC_Camera::orbit(GLC_Vector4d VectOldPoss, GLC_Vector4d VectCurPoss)
{
	// Map Vectors
	VectOldPoss= m_MatCompOrbit * VectOldPoss;
	VectCurPoss= m_MatCompOrbit * VectCurPoss;

	// Compute rotation matrix
	const GLC_Vector4d VectAxeRot(VectCurPoss ^ VectOldPoss);
	// Check if rotation vector is not null
	if (!VectAxeRot.isNull())
	{  // Ok, is not null
		const double Angle= acos(VectCurPoss * VectOldPoss);
		const GLC_Matrix4x4 MatOrbit(VectAxeRot, Angle);

		// Camera transformation
		m_Eye= (MatOrbit * (m_Eye - m_Target)) + m_Target;
		m_VectUp= MatOrbit * m_VectUp;
		m_MatCompOrbit= MatOrbit * m_MatCompOrbit;
	}

	return *this;
}

GLC_Camera& GLC_Camera::pan(GLC_Vector4d VectDep)
{
	// Vector mapping
	VectDep= m_MatCompOrbit * VectDep;

	// Camera transformation
	m_Eye= m_Eye + VectDep;
	m_Target= m_Target + VectDep;

	return *this;
}

GLC_Camera& GLC_Camera::zoom(double factor)
{
	Q_ASSERT(factor > 0);
	// Eye->target vector
	GLC_Vector4d VectCam(m_Eye - m_Target);

	// Compute new vector length
	const double Norme= VectCam.norm() * 1 / factor;
	VectCam.setNormal(Norme);

	m_Eye= VectCam + m_Target;

	return *this;
}

// Move camera
GLC_Camera& GLC_Camera::move(const GLC_Matrix4x4 &MatMove)
{
	m_Eye= MatMove * m_Eye;
	m_Target= MatMove * m_Target;

	// Up vector computation
	// In case of translation in matrix
	const GLC_Vector4d VectOrigine(0,0,0);
	// Backup m_VectUp
	const GLC_Vector4d VectUpOld(m_VectUp);
	m_VectUp= (MatMove * VectUpOld) - (MatMove * VectOrigine); // Up Vector Origin must be equal to 0,0,0
	m_VectUp.setNormal(1.0);
	createMatComp();

	return *this;
}

// Rotate around an axis
GLC_Camera& GLC_Camera::rotateAround(const GLC_Vector4d& axis, const double& angle, const GLC_Point4d& point)
{
	const GLC_Matrix4x4 rotationMatrix(axis, angle);
	translate(-point);
	move(rotationMatrix);
	translate(point);

	return *this;
}

// Rotate around camera target
GLC_Camera& GLC_Camera::rotateAroundTarget(const GLC_Vector4d& axis, const double& angle)
{
	GLC_Point4d target(m_Target);
	rotateAround(axis, angle, target);

	return *this;
}

GLC_Camera& GLC_Camera::translate(const GLC_Vector4d &VectTrans)
{
	m_Eye= m_Eye + VectTrans;
	m_Target= m_Target + VectTrans;

	return *this;
}

GLC_Camera& GLC_Camera::setEyeCam(const GLC_Point4d &Eye)
{
	// Old camera's vector
	GLC_Vector4d VectOldCam(m_Eye - m_Target);
	// New camera's vector
	GLC_Vector4d VectCam(Eye - m_Target);
	if ( !(VectOldCam - VectCam).isNull() )
	{
		VectOldCam.setNormal(1);
		VectCam.setNormal(1);
		const double Angle= acos(VectOldCam * VectCam);
		if ( not qFuzzyCompare(Angle, 0.0) and not qFuzzyCompare(PI - Angle, 0.0))
		{
			const GLC_Vector4d VectAxeRot(VectOldCam ^ VectCam);
			const GLC_Matrix4x4 MatRot(VectAxeRot, Angle);
			m_VectUp= MatRot * m_VectUp;
		}
		else
		{
			if ( qFuzzyCompare(PI - Angle, 0.0))
			{	// Angle de 180�
				m_VectUp.setInv();
			}
		}

		setCam(Eye, m_Target, m_VectUp);
	}

	return *this;
}

GLC_Camera& GLC_Camera::setTargetCam(const GLC_Point4d &Target)
{
	// Old camera's vector
	GLC_Vector4d VectOldCam(m_Eye - m_Target);
	// New camera's vector
	GLC_Vector4d VectCam(m_Eye - Target);
	if ( !(VectOldCam - VectCam).isNull() )
	{
		VectOldCam.setNormal(1);
		VectCam.setNormal(1);
		const double Angle= acos(VectOldCam * VectCam);
		if ( not qFuzzyCompare(Angle, 0.0) and not qFuzzyCompare(PI - Angle, 0.0))
		{
			const GLC_Vector4d VectAxeRot(VectOldCam ^ VectCam);
			const GLC_Matrix4x4 MatRot(VectAxeRot, Angle);
			m_VectUp= MatRot * m_VectUp;
		}
		else
		{
			if ( qFuzzyCompare(PI - Angle, 0.0))
			{	// Angle of 180�
				m_VectUp.setInv();
			}
		}

		setCam(m_Eye, Target, m_VectUp);
	}

	return *this;
}

GLC_Camera& GLC_Camera::setUpCam(const GLC_Vector4d &Up)
{
	if ( !(m_VectUp - Up).isNull() )
	{
		if (not qFuzzyCompare(camVector().getAngleWithVect(Up), 0.0))
		{
			setCam(m_Eye, m_Target, Up);
		}
	}

	return *this;
}

GLC_Camera& GLC_Camera::setCam(GLC_Point4d Eye, GLC_Point4d Target, GLC_Vector4d Up)
{
	Up.setNormal(1);

	const GLC_Vector4d VectCam((Eye - Target).setNormal(1));
	const double Angle= acos(VectCam * Up);

	/* m_VectUp and VectCam could not be parallel
	 * m_VectUp could not be NULL
	 * VectCam could not be NULL */
	//Q_ASSERT((Angle > EPSILON) && ((PI - Angle) > EPSILON));

	if ( not qFuzzyCompare(Angle - (PI / 2), 0.0))
	{	// Angle not equal to 90�
		const GLC_Vector4d AxeRot(VectCam ^ Up);
		GLC_Matrix4x4 MatRot(AxeRot, PI / 2);
		Up= MatRot * VectCam;
	}

	m_Eye= Eye;
	m_Target= Target;
	m_VectUp= Up;
	createMatComp();

	return *this;
}

//! Set the camera by copying another camera
GLC_Camera& GLC_Camera::setCam(const GLC_Camera& cam)
{
	m_Eye= cam.m_Eye;
	m_Target= cam.m_Target;
	m_VectUp= cam.m_VectUp;
	m_MatCompOrbit= cam.m_MatCompOrbit;

	return *this;
}


GLC_Camera& GLC_Camera::setDistEyeTarget(double Longueur)
{
    GLC_Vector4d VectCam(m_Eye - m_Target);
    VectCam.setNormal(Longueur);
    m_Eye= VectCam + m_Target;

    return *this;
}
// Assignment operator
GLC_Camera &GLC_Camera::operator=(const GLC_Camera& cam)
{
	GLC_Object::operator=(cam);
	m_Eye= cam.m_Eye;
	m_Target= cam.m_Target;
	m_VectUp= cam.m_VectUp;
	m_MatCompOrbit= cam.m_MatCompOrbit;
	m_DefaultVectUp= cam.m_DefaultVectUp;

	return *this;
}
// almost equality (Bauer Laurent)
bool GLC_Camera::isAlmostEqualTo(const GLC_Camera& cam, const double distanceAccuracy) const
{
      GLC_Vector4d incident1 = m_Target - m_Eye;
      GLC_Vector4d incident2 = cam.m_Target - cam.m_Eye;

      double allowedGap =  incident1.norm() * distanceAccuracy;
      GLC_Point4d left1 = incident1 ^ m_VectUp;
      GLC_Point4d left2 = incident2 ^ cam.m_VectUp;

      return ((m_Eye - cam.m_Eye).norm() < allowedGap ) && ( (m_Target - cam.m_Target).norm() < allowedGap)
                  && ((left1 - left2).norm() < allowedGap) ;
}

// Return the standard front view form this camera
GLC_Camera GLC_Camera::frontView() const
{
	GLC_Vector4d eye;

	if (m_DefaultVectUp == glc::Z_AXIS)
	{
		eye.setVect(0.0, -1.0, 0.0);
	}
	else // Y_AXIS or X_AXIS
	{
		eye.setVect(0.0, 0.0, 1.0);
	}
	eye= eye + m_Target;

	GLC_Camera newCam(eye, m_Target, m_DefaultVectUp);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);
	return newCam;
}

// Return the standard rear view form this camera
GLC_Camera GLC_Camera::rearView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, glc::PI);
}

// Return the standard right view form this camera
GLC_Camera GLC_Camera::rightView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, glc::PI / 2.0);}

// Return the standard left view form this camera
GLC_Camera GLC_Camera::leftView() const
{
	return frontView().rotateAroundTarget(m_DefaultVectUp, - glc::PI / 2.0);
}

// Return the standard top view form this camera
GLC_Camera GLC_Camera::topView() const
{
	GLC_Vector4d eye= m_DefaultVectUp;
	eye= eye + m_Target;
	GLC_Vector4d up;

	if (m_DefaultVectUp == glc::Y_AXIS)
	{
		up.setVect(0.0, 0.0, -1.0);
	}
	else // Z_AXIS or X_AXIS
	{
		up.setVect(0.0, 1.0, 0.0);
	}

	GLC_Camera newCam(eye, m_Target, up);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);

	return newCam;
}

// Return the standard bottom view form this camera
GLC_Camera GLC_Camera::bottomView() const
{
	GLC_Camera newCam(topView());
	newCam.rotateAroundTarget(newCam.upVector(), glc::PI);

	return newCam;
}

// Return the standard isoview from his camera
GLC_Camera GLC_Camera::isoView() const
{
	GLC_Vector4d eye;
	if (m_DefaultVectUp == glc::Z_AXIS)
	{
		eye.setVect(-1.0, -1.0, 1.0);
	}
	else if (m_DefaultVectUp == glc::Y_AXIS)
	{
		eye.setVect(-1.0, 1.0, 1.0);
	}
	else
	{
		eye.setVect(1.0, 1.0, 1.0);
	}

	eye= eye + m_Target;

	GLC_Camera newCam(eye, m_Target, m_DefaultVectUp);
	newCam.setDistEyeTarget(distEyeTarget());
	newCam.setDefaultUpVector(m_DefaultVectUp);
	return newCam;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////
void GLC_Camera::glExecute()
{	
	gluLookAt(m_Eye.X(), m_Eye.Y(), m_Eye.Z(),
		m_Target.X(), m_Target.Y(), m_Target.Z(),
		m_VectUp.X(), m_VectUp.Y(), m_VectUp.Z());

}

// PFC MOD starts here
void GLC_Camera::glExecute(TrackingPFC_client*& c)
{	
	c->htgluLookAt(m_Eye.X(), m_Eye.Y(), m_Eye.Z(),
		m_Target.X(), m_Target.Y(), m_Target.Z(),
		m_VectUp.X(), m_VectUp.Y(), m_VectUp.Z());

}
// PFC MOD ends here

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

void GLC_Camera::createMatComp(void)
{
	const GLC_Vector4d VectCam((m_Eye - m_Target).setNormal(1));
	const GLC_Vector4d orthoVect(m_VectUp ^ VectCam);

	// Create camera matrix
	double newMat[16];
	newMat[0]= orthoVect.X();
	newMat[1]= orthoVect.Y();
	newMat[2]= orthoVect.Z();
	newMat[3]= 0.0;

	// Vector Up is Y Axis
	newMat[4]= m_VectUp.X();
	newMat[5]= m_VectUp.Y();
	newMat[6]= m_VectUp.Z();
	newMat[7]= 0.0;

	// Vector Cam is Z axis
	newMat[8]= VectCam.X();
	newMat[9]= VectCam.Y();
	newMat[10]= VectCam.Z();
	newMat[11]= 0.0;

	newMat[12]= 0.0;
	newMat[13]= 0.0;
	newMat[14]= 0.0;
	newMat[15]= 1.0;

	// Load the result matrix into camera matrix
	m_MatCompOrbit= GLC_Matrix4x4(newMat);
}
