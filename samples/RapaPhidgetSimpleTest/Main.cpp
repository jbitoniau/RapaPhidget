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
#include "RPhiLocalDeviceManager.h"
#include "RPhiRemoteDeviceManager.h"
#include "RPhiSpatial.h"
#include "RPhiTemperatureSensor.h"

#include <map> 
#include <assert.h> 
#include <stdio.h>

#if _WIN32
	#include <Windows.h>
#else
	#include <unistd.h>
	#include <sys/timeb.h>
	#include <time.h>
	#include <sys/time.h>
#endif

// A utility class to provide cross-platform sleep and simple time methods
class Utils
{
public:
	static void sleep( unsigned int _Milliseconds );
	static unsigned long long int getTickFrequency();
	static unsigned long long int getTimeAsTicks();
	static unsigned int getTimeAsMilliseconds();

private:
	static unsigned long long int mInitialTickCount;
};

int i = 0;
class DebugDeviceListener : public RPhi::Device::Listener
{
public:
	void onDeviceChanged( RPhi::Device* device ) 
	{
		switch ( device->getType() )
		{
			case RPhi::Device::kSpatial:
			{
				RPhi::Spatial* spatial = static_cast<RPhi::Spatial*>(device);
				printf("%s\n", spatial->toString().c_str() );

				i++;
				if ( i==20 )
				{
					if ( !spatial->setDataRateInMs( 400 ) )
						printf("\n\nFAILED TO SET RATE\n\n");
				}	
			}
			break;

			case RPhi::Device::kTemperatureSensor:
			{
				RPhi::TemperatureSensor* temperatureSensor = static_cast<RPhi::TemperatureSensor*>(device);
				printf("%s\n", temperatureSensor->toString().c_str() );

				i++;
				if ( i==20 )
				{
					temperatureSensor->getThermocouples()[0]->setType( RPhi::TemperatureSensor::Thermocouple::E_Type );
				}	
			}
			break;
		}
	}
};

class DebugDeviceManagerListener : public RPhi::DeviceManager::Listener
{
public:
	virtual void onDeviceConnected( RPhi::DeviceManager* /*deviceManager*/, RPhi::Device* device )
	{
		printf("Device %d %d - Connected\n", device->getSerialNumber(), device->isAttached() );
		
		// Create a listener for the Device, register it and remember it
		RPhi::Device::Listener* listener = new DebugDeviceListener();
		mListeners.insert( std::make_pair( device, listener ) );
		device->addListener( listener );
	}

	virtual void onDeviceDisconnecting( RPhi::DeviceManager* /*deviceManager*/, RPhi::Device* device )
	{
		printf("Device %d %d - Disconnecting\n", device->getSerialNumber(), device->isAttached() );
		
		// Retrieve the listener we created for the Device, unregister it, delete it and forget it
		std::map<RPhi::Device*, RPhi::Device::Listener*>::iterator itr = mListeners.find( device );
		assert( itr!=mListeners.end() );
		RPhi::Device::Listener* listener = (*itr).second;
		device->removeListener( listener );
		mListeners.erase( itr );
		delete listener;
	}

private:
	std::map<RPhi::Device*, RPhi::Device::Listener*> mListeners;	
};

int main( int /*argc*/, char** /*argv*/ )
{
	RPhi::LocalDeviceManager deviceManager;
	//RPhi::RemoteDeviceManager deviceManager( "BOB-PC", "" );
	//RPhi::RemoteDeviceManager deviceManager( "192.168.1.56", 5001, "" );
	
//	printf("version: %s\n", deviceManager.getLibraryVersion() );(

	DebugDeviceManagerListener listener;
	deviceManager.addListener( &listener );
	
	bool up = true;
	for (int i=0; i<60; ++i )
	{
		if ( up )
			deviceManager.update();
		Utils::sleep( 1000 );
		printf(".");
		fflush( stdout );
	}

	return 0;
}

// Utils class implementation
void Utils::sleep( unsigned int _Milliseconds )
{
#if _WIN32
	::Sleep( _Milliseconds );
#else
	struct timespec l_TimeSpec;
	l_TimeSpec.tv_sec = _Milliseconds / 1000;
	l_TimeSpec.tv_nsec = (_Milliseconds % 1000) * 1000000;
	struct timespec l_Ret;
	nanosleep(&l_TimeSpec,&l_Ret);
#endif
}

unsigned long long int Utils::getTickFrequency()
{
#if _WIN32
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
#else
	// The gettimeofday function returns the time in microseconds. So it's frequency is 1,000,000.
	return 1000000;
#endif
}

unsigned long long int Utils::getTimeAsTicks()
{
	unsigned long long int tickCount;
#if _WIN32
	LARGE_INTEGER l;
	QueryPerformanceCounter(&l);
	tickCount = l.QuadPart;
#else
	struct timeval p;
	gettimeofday(&p, NULL);	// Gets the time since the Epoch (00:00:00 UTC, January 1, 1970) in sec, and microsec
	tickCount = (p.tv_sec * 1000LL * 1000LL) + p.tv_usec;
#endif
	if ( mInitialTickCount==0xffffffffffffffffUL )
		mInitialTickCount = tickCount;
	tickCount -= mInitialTickCount;
	return tickCount;
}

unsigned int Utils::getTimeAsMilliseconds()
{
	unsigned int millecondsTime = static_cast<unsigned int>( (getTimeAsTicks() * 1000) / getTickFrequency() );
	return millecondsTime;
}

unsigned long long int Utils::mInitialTickCount = 0xffffffffffffffffUL;
