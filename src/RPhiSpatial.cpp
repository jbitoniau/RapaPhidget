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
#include "RPhiSpatial.h"

#include <assert.h>
#include <sstream>
#include <phidget21.h>

/*
	Notes:
	- Compass calibration support
	- Adding a timestamp to the measure (if possible from the Spatial hardware) would be nice. 
	  It is available when using Phidget through the event/notification system which is not
	  our approach. We use the polling approach
*/
namespace RPhi
{

Spatial::Spatial( CPhidgetHandle phidgetSpecificHandle )
	: Device( kSpatial, phidgetSpecificHandle ),
	  mAngularRateZSignFix(1.0),
	  mNumAccelerationAxes(0),
	  mNumAngularRateAxes(0),
	  mNumMagneticFieldAxes(0),
	  mDataRateInMs(0),
	  mMeasure(),
	  mMinMeasure(),
	  mMaxMeasure()
{	
	// Get common Phidget information
	getInformation();

	// Get non dynamic information about the Spatial
	getSpatialInformation();

	// We need to fix the angular rate component on the broken phidget device we've got, this is hard coded.
	if ( getSerialNumber()==165536 )
		mAngularRateZSignFix = -1.0;
}

Spatial::~Spatial()
{
	// Close the device
	int ret = EPHIDGET_OK;
	ret = CPhidget_close( getPhidgetHandle() );
	assert( ret==EPHIDGET_OK );

	// Delete the device
	ret = CPhidget_delete( getPhidgetHandle() );
	assert( ret==EPHIDGET_OK );
}

// Not all data rates are available. 
// See http://www.phidgets.com/docs/General_Phidget_Programming#Data_Rate
bool Spatial::setDataRateInMs( int dataRateInMs )
{
	int ret = CPhidgetSpatial_setDataRate( getSpatialHandle(), dataRateInMs );
	
	// Update cache value from Phidget device
	internalGetDataRateInMs();
	
	if ( ret==EPHIDGET_INVALIDARG )
		return false;
	assert( ret==EPHIDGET_OK );
	return true;
}	

void Spatial::zeroGyro()
{
	int ret = CPhidgetSpatial_zeroGyro( getSpatialHandle() );
	assert( ret==EPHIDGET_OK );
}

void Spatial::update()
{
	// Get a new measure
	Measure measure;
	updateMeasure( measure, mMeasure.getMagneticFieldInGauss() );

	// Update the current measure with the new one
	if ( measure!=mMeasure )
	{
		mMeasure = measure;

		// Notify
		const Listeners& listeners = getListeners();
		for ( Listeners::const_iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
			(*itr)->onDeviceChanged( this );
	}
}

void Spatial::updateMeasure( Measure& measure, const Vector3d& fallbackMagneticFieldInGauss )
{	
	CPhidgetSpatialHandle handle = getSpatialHandle();
	int ret = EPHIDGET_OK;
		
	// Acceleration
	double acc[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<getNumAccelerationAxes(); ++i )
	{
		ret = CPhidgetSpatial_getAcceleration( handle, i, &acc[i] );
		
		// It seems that on linux we can get a EPHIDGET_UNKNOWNVAL just after 
		// plugging a Spatial back into the machine... 
		// Will need to fail more elegantly instead of asserting
		//assert( ret==EPHIDGET_OK );
		//assert( acc[i]!=PUNK_DBL );
		if ( ret!=EPHIDGET_OK )
			return;
	}
	Vector3d accelerationInGs = Vector3d( acc[0], acc[1], acc[2] );
	
	// Angular rate 
	double ang[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<getNumAngularRateAxes(); ++i )
	{
		ret = CPhidgetSpatial_getAngularRate( handle, i, &ang[i] );
		assert( ret==EPHIDGET_OK );
		assert( ang[i]!=PUNK_DBL );
	}
	ang[2] *= mAngularRateZSignFix;	// Correct the value for the broken Spatial we've got
	Vector3d angularRateInDegPerSec = Vector3d( ang[0], ang[1], ang[2] );
	
	// Magnetic field
	double mag[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<getNumMagneticFieldAxes(); ++i )
	{
		ret = CPhidgetSpatial_getMagneticField( handle, i, &mag[i] );
		
		// Every 2 seconds or so, the magnetometer is unavailable PUNK_DBL is returned as value, 
		// while the call to CPhidgetSpatial_getMagneticField() as above returns EPHIDGET_UNKNOWNVAL.
		// When this happens, we return the last valid magnetic field value. This seems like the 
		// most sensible thing to do
		if ( ret==EPHIDGET_UNKNOWNVAL )
		{
			assert( mag[i]==PUNK_DBL );
			if (i==0)
				mag[i] = fallbackMagneticFieldInGauss.x();
			else if (i==1)
				mag[i] = fallbackMagneticFieldInGauss.y();
			else if (i==2)
				mag[i] = fallbackMagneticFieldInGauss.z();
		}
		else
		{
			assert( mag[i]!=PUNK_DBL );
		}
	}
	Vector3d magneticFieldInGauss = Vector3d( mag[0], mag[1], mag[2] );

	// Construct the new measure
	measure = Measure( accelerationInGs, angularRateInDegPerSec, magneticFieldInGauss );
}

void Spatial::getSpatialInformation()
{
	Vector3d minAcc;
	Vector3d maxAcc;
	getAccelerationInformation( mNumAccelerationAxes, minAcc, maxAcc );

	Vector3d minAng;
	Vector3d maxAng;
	getAngularRateInformation( mNumAngularRateAxes, minAng, maxAng );
	
	Vector3d minMag;
	Vector3d maxMag;
	getMagneticFieldInformation( mNumMagneticFieldAxes, minMag, maxMag );

	mMinMeasure = Measure( minAcc, minAng, minMag );
	mMaxMeasure = Measure( maxAcc, maxAng, maxMag );

	internalGetDataRateInMs();
}

void Spatial::getAccelerationInformation( int& numAxes, Vector3d& min, Vector3d& max ) const
{
	int ret = EPHIDGET_OK;
	ret = CPhidgetSpatial_getAccelerationAxisCount( getSpatialHandle(), &numAxes );
	assert( ret==EPHIDGET_OK );
	assert( mNumAccelerationAxes>=0 && mNumAccelerationAxes<=3 );
	double minValues[3] = { 0.0, 0.0, 0.0 };
	double maxValues[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<mNumAccelerationAxes; ++i )
	{
		ret = CPhidgetSpatial_getAccelerationMin( getSpatialHandle(), i, &minValues[i] );
		assert( ret==EPHIDGET_OK );
		ret = CPhidgetSpatial_getAccelerationMax( getSpatialHandle(), i, &maxValues[i] );
		assert( ret==EPHIDGET_OK );
	}
	min = Vector3d( minValues[0], minValues[1], minValues[2] );
	max = Vector3d( maxValues[0], maxValues[1], maxValues[2] );
}

void Spatial::getAngularRateInformation( int& numAxes, Vector3d& min, Vector3d& max ) const
{
	int ret = EPHIDGET_OK;
	ret = CPhidgetSpatial_getGyroAxisCount( getSpatialHandle(), &numAxes );
	assert( ret==EPHIDGET_OK );
	assert( mNumAngularRateAxes>=0 && mNumAngularRateAxes<=3 );
	double minValues[3] = { 0.0, 0.0, 0.0 };
	double maxValues[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<mNumAngularRateAxes; ++i )
	{
		ret = CPhidgetSpatial_getAngularRateMin( getSpatialHandle(), i, &minValues[i] );
		assert( ret==EPHIDGET_OK );
		ret = CPhidgetSpatial_getAngularRateMax( getSpatialHandle(), i, &maxValues[i] );
		assert( ret==EPHIDGET_OK );
	}
	min = Vector3d( minValues[0], minValues[1], minValues[2] );
	max = Vector3d( maxValues[0], maxValues[1], maxValues[2] );
}

void Spatial::getMagneticFieldInformation( int& numAxes, Vector3d& min, Vector3d& max ) const
{
	int ret = EPHIDGET_OK;
	ret = CPhidgetSpatial_getCompassAxisCount( getSpatialHandle(), &numAxes );
	assert( ret==EPHIDGET_OK );
	assert( mNumMagneticFieldAxes>=0 && mNumMagneticFieldAxes<=3 );
	double minValues[3] = { 0.0, 0.0, 0.0 };
	double maxValues[3] = { 0.0, 0.0, 0.0 };
	for ( int i=0; i<mNumMagneticFieldAxes; ++i )
	{
		ret = CPhidgetSpatial_getMagneticFieldMin( getSpatialHandle(), i, &minValues[i] );
		assert( ret==EPHIDGET_OK );
		ret = CPhidgetSpatial_getMagneticFieldMax( getSpatialHandle(), i, &maxValues[i] );
		assert( ret==EPHIDGET_OK );
	}
	min = Vector3d( minValues[0], minValues[1], minValues[2] );
	max = Vector3d( maxValues[0], maxValues[1], maxValues[2] );
}

void Spatial::internalGetDataRateInMs()
{
	int dataRateInMs = 0;
	int ret = CPhidgetSpatial_getDataRate( getSpatialHandle(), &dataRateInMs );
	assert( ret==EPHIDGET_OK );
	mDataRateInMs = dataRateInMs;
}

std::string Spatial::toString() const
{
	std::stringstream stream;
	stream << Device::toString() << " ";
	stream << "measure:(" << getMeasure().toString() << ") ";
	stream << "minMeasure:(" << getMinMeasure().toString() << ") ";
	stream << "maxMeasure:(" << getMaxMeasure().toString() << ")";
	return stream.str();
}

//
//	Spatial::Measure
//
Spatial::Measure::Measure()
	: mAccelerationInGs( 0, 0, 0 ),
	  mAngularRateInDegPerSec( 0, 0, 0 ),
	  mMagneticFieldInGauss( 0, 0, 0 )
{
}

Spatial::Measure::Measure( const Vector3d& accelerationInGs, const Vector3d& angularRateInDegPerSec, const Vector3d& magneticFieldInGauss )
	: mAccelerationInGs( accelerationInGs ),
	  mAngularRateInDegPerSec( angularRateInDegPerSec ),
	  mMagneticFieldInGauss( magneticFieldInGauss )
{
}

bool Spatial::Measure::operator==( const Measure& other ) const
{
	return	mAccelerationInGs==other.mAccelerationInGs &&
			mAngularRateInDegPerSec==other.mAngularRateInDegPerSec &&
			mMagneticFieldInGauss==other.mMagneticFieldInGauss;
}

bool Spatial::Measure::operator!=( const Measure& other ) const
{
	return !(*this==other);
}

std::string Spatial::Measure::toString() const
{
	std::stringstream stream;
	stream.setf( std::ios::fixed, std:: ios::floatfield );
	stream.precision(3);
	const Vector3d& acc = getAccelerationInGs();
	stream << "accelerationInGs:" << acc.x() << " " << acc.y() << " " << acc.z() << " ";
	const Vector3d& ang = getAngularRateInDegPerSec();
	stream << "angularRateInDegPerSec:" << ang.x() << " " << ang.y() << " " << ang.z() << " ";
	const Vector3d& mag = getMagneticFieldInGauss();
	stream << "magneticFieldInGauss:" << mag.x() << " " << mag.y() << " " << mag.z();
	return stream.str();	
}

}