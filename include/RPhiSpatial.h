/*
   The MIT License (MIT) (http://opensource.org/licenses/MIT)
   
   Copyright (c) 2015 Jacques Menuet
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#pragma once

#include "RPhiVector3.h"
#include "RPhiDevice.h"
typedef struct _CPhidgetSpatial *CPhidgetSpatialHandle;

namespace RPhi
{

/*
	Spatial

	The Spatial device provides acceleration, angular rate and magnetic field
	information. 
	
	Some devices may only provide a subset of these. 
	
	Some might also be limited in the number of axis for each type of measure.
	For example an accelerometer can be 1D, meaning the acceleration it provides
	is for a signle direction (as opposed to a fully fledged 3D accelerometer)

	Setting the data-rate at which the Spatial device "averages" the measures 
	and makes them available to the client code is something to be aware of.
	See http://www.phidgets.com/docs/General_Phidget_Programming#Data_Rate

	Accelerometer primer
	http://www.phidgets.com/docs/Accelerometer_Primer

	Gyroscope primer
	http://www.phidgets.com/docs/Gyroscope_Primer

	Compass primer
	http://www.phidgets.com/docs/Compass_Primer
	
	User guide
	http://www.phidgets.com/docs/1041_User_Guide
	http://www.phidgets.com/docs/1056_User_Guide

	C-API reference
	http://www.phidgets.com/documentation/web/cdoc/index.html
	http://www.phidgets.com/documentation/web/cdoc/group__phidspatial.html
*/
class Spatial : public Device
{
public: 
	virtual void			update();

	bool					setDataRateInMs( int dataRateInMs );
	int						getDataRateInMs() const					{ return mDataRateInMs; }

	void					zeroGyro();

	int						getNumAccelerationAxes() const			{ return mNumAccelerationAxes; }
	int						getNumAngularRateAxes() const			{ return mNumAngularRateAxes; }
	int						getNumMagneticFieldAxes() const			{ return mNumMagneticFieldAxes; }
	
	class Measure;
	const Measure&			getMeasure() const						{ return mMeasure; }
	const Measure&			getMinMeasure() const					{ return mMinMeasure; }
	const Measure&			getMaxMeasure() const					{ return mMaxMeasure; }
	
	class Measure
	{
	public:
		Measure();
		Measure( const Vector3d& accelerationInGs, const Vector3d& angularRateInDegPerSec, const Vector3d& magneticFieldInGauss );
		bool operator==( const Measure& other ) const;
		bool operator!=( const Measure& other ) const;

		inline Vector3d		getAccelerationInGs() const				{ return mAccelerationInGs; }
		inline Vector3d		getAngularRateInDegPerSec() const		{ return mAngularRateInDegPerSec; }
		inline Vector3d		getMagneticFieldInGauss() const			{ return mMagneticFieldInGauss; }
		
		std::string			toString() const;

	private:
		Vector3d			mAccelerationInGs;		
		Vector3d			mAngularRateInDegPerSec;
		Vector3d			mMagneticFieldInGauss;	
	};

	virtual std::string		toString() const;

protected:
	friend class DeviceManager;
	Spatial( CPhidgetHandle phidgetSpecificHandle );
	virtual ~Spatial();

	CPhidgetSpatialHandle	getSpatialHandle() const { return reinterpret_cast<CPhidgetSpatialHandle>(getPhidgetHandle()); }

	void					updateMeasure( Measure& measure, const Vector3d& fallbackMagneticFieldInGauss );
	
	void					getSpatialInformation();
	void					getAccelerationInformation( int& numAxes, Vector3d& min, Vector3d& max ) const;
	void					getAngularRateInformation( int& numAxes, Vector3d& min, Vector3d& max ) const;
	void					getMagneticFieldInformation( int& numAxes, Vector3d& min, Vector3d& max ) const;
	void					internalGetDataRateInMs();

private:
	double					mAngularRateZSignFix;	// The gyrometer of one of our Spatials seems to be mounted incorrectly on the board!
	int						mNumAccelerationAxes;
	int						mNumAngularRateAxes;
	int						mNumMagneticFieldAxes;
	int						mDataRateInMs;
	Measure					mMeasure;
	Measure					mMinMeasure;
	Measure					mMaxMeasure;
};

}