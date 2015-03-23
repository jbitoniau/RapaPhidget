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

#include <vector>
#include "RPhiDevice.h"
typedef struct _CPhidgetTemperatureSensor *CPhidgetTemperatureSensorHandle;

namespace RPhi
{
/* 
	TemperatureSensor 

	A TemperatureSensor is a device which contains one or more Thermocouples. 
	Each Thermocouple provides a temperature reading/measure in Celsius degrees.
	This is the temperature at the end of the wire. There's also the raw measure 
	which is in millivolts. The millivolts gets converted into °C by Phidget
	based on the type of the Thermocouple (K, J, E and T).

	The TemperatureSensor also has a built-in sensor which gives the "ambient"
	temperature. This is the temperature at the board.

	Thermocouple primer
	http://www.phidgets.com/docs/Thermocouple_Primer

	User guide
	http://www.phidgets.com/docs/1051_User_Guide
	
	C-API reference
	http://www.phidgets.com/documentation/web/cdoc/index.html
	http://www.phidgets.com/documentation/web/cdoc/group__phidtemp.html
*/
class TemperatureSensor : public Device
{
public: 
	virtual void			update();

	// Return the list of Thermocouples present on the TemperatureSensor device
	class Thermocouple;
	typedef std::vector<Thermocouple*> Thermocouples;
	const Thermocouples& getThermocouples() const	{ return mThermocouples; }

	// Return the temperature on the board 
	double getAmbientTemperatureInC() const			{ return mAmbientTemperatureInC; }
	double getMinAmbientTemperatureInC() const		{ return mMinAmbientTemperatureInC; }
	double getMaxAmbientTemperatureInC() const		{ return mMaxAmbientTemperatureInC; }

	class Thermocouple 
	{
	public:	
		// Update the measure of the Thermocouple. Returns true if it changes, so that the 
		// parent TemperatureSensor can notify of the change
		bool	update(); 

		// The index of the Thermocouple in the parent device
		int		getIndex() const { return mIndex; }

		// The type of Thermocouple. This must be kept in-sync with CPhidgetTemperatureSensor_ThermocoupleType
		enum Type
		{
			K_Type = 1,
			J_Type,
			E_Type,
			T_Type,
		};
		Type	getType() const										{ return mType; }
		void	setType( Type type );	

		class Measure;
		const Measure& getMeasure() const							{ return mMeasure; }
		const Measure& getMinMeasure() const						{ return mMinMeasure; }
		const Measure& getMaxMeasure() const						{ return mMaxMeasure; }

		class Measure
		{
		public:
			Measure();
			Measure( double temperatureInC, double potentialInMV );
			bool operator==( const Measure& other ) const;
			bool operator!=( const Measure& other ) const;

			inline double		getTemperatureInC() const			{ return mTemperatureInC; }
			inline double		getPotentialInMV() const			{ return mPotentialInMV; }
			
			std::string			toString() const;

		private:
			double				mTemperatureInC;		
			double				mPotentialInMV;	
		};

		std::string	toString() const;

	protected:
		friend class TemperatureSensor;
		Thermocouple( CPhidgetTemperatureSensorHandle parentTemperatureSensorHandle, int index );
	
		void					getThermocoupleInformation();
		void					updateMeasure( Measure& measure );
	
	private:
		CPhidgetTemperatureSensorHandle mParentTemperatureSensorHandle;
		int						mIndex;
		Type					mType;
		Measure					mMeasure;
		Measure					mMinMeasure;
		Measure					mMaxMeasure;
	};
	
	virtual std::string		toString() const;

protected:
	friend class DeviceManager;
	TemperatureSensor( CPhidgetHandle phidgetSpecificHandle );
	virtual ~TemperatureSensor();

	CPhidgetTemperatureSensorHandle	getTemperatureSensorHandle() const  { return reinterpret_cast<CPhidgetTemperatureSensorHandle>(getPhidgetHandle()); }

	void							getTemperatureSensorInformation();

private:
	Thermocouples					mThermocouples;
	double							mAmbientTemperatureInC;
	double							mMinAmbientTemperatureInC;
	double							mMaxAmbientTemperatureInC;
};

}