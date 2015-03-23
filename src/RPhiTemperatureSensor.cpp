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
#include "RPhiTemperatureSensor.h"

#include <assert.h>
#include <sstream>
#include <phidget21.h>

/*
	Notes:
	- The CPhidgetTemperatureSensor_setTemperatureChangeTrigger function is not exposed here. This is because we're 
	  polling the device, we're not using the change trigger event. So in our case, the function doesn't do anything.
	  We could however implement our own change trigger mecanism if required.
	  After investigation, it turns out the function is important in remote. It dictates when measure packets are 
	  sent to the network.
*/
namespace RPhi
{

// Change trigger only usefull in Event-based mode
TemperatureSensor::TemperatureSensor( CPhidgetHandle phidgetSpecificHandle )
	: Device( kTemperatureSensor, phidgetSpecificHandle ),
	  mThermocouples(),
	  mAmbientTemperatureInC(0.0),
	  mMinAmbientTemperatureInC(0.0),
	  mMaxAmbientTemperatureInC(0.0)
{
	// Get common Phidget information
	getInformation();

	// Get non dynamic Sinformation about the Spatial
	getTemperatureSensorInformation();
}

TemperatureSensor::~TemperatureSensor()
{
	// Delete the Thermocouples
	for ( Thermocouples::iterator itr=mThermocouples.begin(); itr!=mThermocouples.end(); ++itr )
		delete (*itr);
	mThermocouples.clear();

	// Close the device
	int ret = EPHIDGET_OK;
	ret = CPhidget_close( getPhidgetHandle() );
	assert( ret==EPHIDGET_OK );

	// Delete the device
	ret = CPhidget_delete( getPhidgetHandle() );
	assert( ret==EPHIDGET_OK );
}

void TemperatureSensor::getTemperatureSensorInformation()
{
	// Create the Thermocouples 
	int ret = EPHIDGET_OK;
	int count = 0;
	ret = CPhidgetTemperatureSensor_getTemperatureInputCount( getTemperatureSensorHandle(), &count );
	assert( ret==EPHIDGET_OK );
	for ( int i=0; i<count; ++i )
	{
		Thermocouple* thermocouple = new Thermocouple( getTemperatureSensorHandle(), i );
		mThermocouples.push_back( thermocouple );
	}

	// Ambient temperature
	ret = CPhidgetTemperatureSensor_getAmbientTemperatureMin( getTemperatureSensorHandle(), &mMinAmbientTemperatureInC );	
	assert( ret==EPHIDGET_OK );
	ret = CPhidgetTemperatureSensor_getAmbientTemperatureMax( getTemperatureSensorHandle(), &mMaxAmbientTemperatureInC );	
	assert( ret==EPHIDGET_OK );
}

void TemperatureSensor::update()
{
	// Thermocouples
	bool thermocouplesMeasureChanged = false;
	for ( Thermocouples::iterator itr=mThermocouples.begin(); itr!=mThermocouples.end(); ++itr  )
	{
		if ( (*itr)->update() )
			thermocouplesMeasureChanged = true;
	}
	
	// Ambient temperature
	bool ambientTemperatureChanged = false;
	double ambientTemperature = 0.0;
	int ret = CPhidgetTemperatureSensor_getAmbientTemperature( getTemperatureSensorHandle(), &ambientTemperature );	
	assert( ret==EPHIDGET_OK );
	if ( ambientTemperature!=mAmbientTemperatureInC )
	{
		mAmbientTemperatureInC = ambientTemperature;
		ambientTemperatureChanged = true;
	}

	if ( thermocouplesMeasureChanged || ambientTemperatureChanged )
	{
		// Notify
		const Listeners& listeners = getListeners();
		for ( Listeners::const_iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
			(*itr)->onDeviceChanged( this );
	}
}

std::string TemperatureSensor::toString() const
{
	std::stringstream stream;
	stream << "thermocouples:(";
	for ( Thermocouples::const_iterator itr=mThermocouples.begin(); itr!=mThermocouples.end(); ++itr  )
		stream << (*itr)->toString();
	stream << ") ";
	stream << "ambientTemperatureInC:" << getAmbientTemperatureInC() << " ";
	stream << "minAmbientTemperatureInC:" << getMinAmbientTemperatureInC() << " ";
	stream << "maxAmbientTemperatureInC:" << getMaxAmbientTemperatureInC() << " ";
	return stream.str();
}

//
//	TemperatureSensor::Thermocouple
//
TemperatureSensor::Thermocouple::Thermocouple( CPhidgetTemperatureSensorHandle parentTemperatureSensorHandle, int index )
	: mParentTemperatureSensorHandle(parentTemperatureSensorHandle),
	  mIndex(index),
	  mType(K_Type),
	  mMeasure(),
	  mMinMeasure(),
	  mMaxMeasure()
{
	getThermocoupleInformation();	
}

void TemperatureSensor::Thermocouple::getThermocoupleInformation()
{
	int ret = EPHIDGET_OK;
	
	CPhidgetTemperatureSensor_ThermocoupleType type = PHIDGET_TEMPERATURE_SENSOR_K_TYPE;
	ret = CPhidgetTemperatureSensor_getThermocoupleType( mParentTemperatureSensorHandle, mIndex, &type );
	assert( ret==EPHIDGET_OK );
	mType = static_cast<Type>(type);	// CPhidgetTemperatureSensor_ThermocoupleType and TemperatureSensor::Thermocouple::Type must be the same

	double minTemperature;
	double minPotential;
	ret = CPhidgetTemperatureSensor_getTemperatureMin( mParentTemperatureSensorHandle, mIndex, &minTemperature );
	assert( ret==EPHIDGET_OK );
	ret = CPhidgetTemperatureSensor_getPotentialMin( mParentTemperatureSensorHandle, mIndex, &minPotential );
	assert( ret==EPHIDGET_OK );
	mMinMeasure = Measure( minTemperature, minPotential );

	double maxTemperature;
	double maxPotential;
	ret = CPhidgetTemperatureSensor_getTemperatureMax( mParentTemperatureSensorHandle, mIndex, &maxTemperature );
	assert( ret==EPHIDGET_OK );
	ret = CPhidgetTemperatureSensor_getPotentialMax( mParentTemperatureSensorHandle, mIndex, &maxPotential );
	assert( ret==EPHIDGET_OK );
	mMaxMeasure = Measure( maxTemperature, maxPotential );
}	

void TemperatureSensor::Thermocouple::setType( Type type )
{
	CPhidgetTemperatureSensor_ThermocoupleType theType = static_cast<CPhidgetTemperatureSensor_ThermocoupleType>(type) ;
	int ret = CPhidgetTemperatureSensor_setThermocoupleType( mParentTemperatureSensorHandle, mIndex, theType );
	assert( ret==EPHIDGET_OK );
	
	// After the type has changed, we need to update the thermocouple information. 
	// Min/max measures depends on the type (well the temperature range, not the potential range)
	getThermocoupleInformation();
}

bool TemperatureSensor::Thermocouple::update()
{
	bool changed = false;

	// Get a new measure
	Measure measure;
	updateMeasure( measure );

	// Update the current measure with the new one
	if ( measure!=mMeasure )
	{
		mMeasure = measure;
		changed = true;
	}

	return changed;
}

void TemperatureSensor::Thermocouple::updateMeasure( Measure& measure )
{
	double temperature;
	double potential;
	int ret = EPHIDGET_OK;
	ret = CPhidgetTemperatureSensor_getTemperature( mParentTemperatureSensorHandle, mIndex, &temperature );
	assert( ret==EPHIDGET_OK );
	ret = CPhidgetTemperatureSensor_getPotential( mParentTemperatureSensorHandle, mIndex, &potential );
	assert( ret==EPHIDGET_OK );
	measure = Measure( temperature, potential );
}

std::string TemperatureSensor::Thermocouple::toString() const
{
	std::stringstream stream;
	stream << "index:" << mIndex << " ";
	stream << "type:" << mType << " ";
	stream << "measure:(" << mMeasure.toString() << ") ";
	stream << "minMeasure:(" << mMinMeasure.toString() << ") ";
	stream << "maxMeasure:(" << mMaxMeasure.toString() << ")";
	return stream.str();
}

//
//	TemperatureSensor::Thermocouple::Measure
//
TemperatureSensor::Thermocouple::Measure::Measure()
	: mTemperatureInC( 0.0 ),
	  mPotentialInMV( 0.0 )
{
}

TemperatureSensor::Thermocouple::Measure::Measure( double temperatureInC, double potentialInMV )
	: mTemperatureInC( temperatureInC ),
	  mPotentialInMV( potentialInMV )
{
}

bool TemperatureSensor::Thermocouple::Measure::operator==( const Measure& other ) const
{
	return	mTemperatureInC==other.mTemperatureInC &&
			mPotentialInMV==other.mPotentialInMV;
}

bool TemperatureSensor::Thermocouple::Measure::operator!=( const Measure& other ) const
{
	return !(*this==other);
}

std::string TemperatureSensor::Thermocouple::Measure::toString() const
{
	std::stringstream stream;
	stream.setf( std::ios::fixed, std:: ios::floatfield );
	stream.precision(3);
	stream << "temperatureInC:" << getTemperatureInC() << " ";
	stream << "potentialInMV:" << getPotentialInMV();
	return stream.str();	
}

}