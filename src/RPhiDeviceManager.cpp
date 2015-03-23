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
#include "RPhiDeviceManager.h"

#include <phidget21.h>
#include <assert.h>
#include <algorithm>

#include "RPhiSpatial.h"
#include "RPhiTemperatureSensor.h"

/*
	Notes
	- Check for changes in connected devices only once in a while instead of at each update 
*/
namespace RPhi
{

/*
	DeviceManager
*/
DeviceManager::DeviceManager()
	: mManagerHandle(NULL),
	  mIsLocal(true),
	  mDevices(),
	  mListeners()
{
	// To activate logging
	//ret = CPhidget_enableLogging( PHIDGET_LOG_VERBOSE, "c:\\phidget.log");
	//assert( ret==EPHIDGET_OK );

	int ret = EPHIDGET_OK;
	ret = CPhidgetManager_create( &mManagerHandle );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK  )
		return;
}

void DeviceManager::openLocally()
{
	mIsLocal = true;
	int ret = CPhidgetManager_open( mManagerHandle );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK  )
	{
		ret = CPhidgetManager_delete( mManagerHandle );
		assert( ret==EPHIDGET_OK );
		mManagerHandle = NULL;
	}
}

void DeviceManager::openRemotelyWithServerID( const std::string& serverID, const std::string& password )
{
	mIsLocal = false;
	int ret = CPhidgetManager_openRemote( mManagerHandle, serverID.c_str(), password.c_str() );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK  )
	{
		ret = CPhidgetManager_delete( mManagerHandle );
		assert( ret==EPHIDGET_OK );
		mManagerHandle = NULL;
	}
}

void DeviceManager::openRemotelyWithServerAddress( const std::string& address, int port, const std::string& password )
{
	mIsLocal = false;
	int ret = CPhidgetManager_openRemoteIP( mManagerHandle, address.c_str(), port, password.c_str() );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK  )
	{
		ret = CPhidgetManager_delete( mManagerHandle );
		assert( ret==EPHIDGET_OK );
		mManagerHandle = NULL;
	}
}

DeviceManager::~DeviceManager()
{
	if ( mManagerHandle )
	{
		Devices	devices = mDevices;		// The copy is on purpose
		for ( Devices::iterator itr=devices.begin(); itr!=devices.end(); ++itr )
			deleteDevice( (*itr)->getPhidgetHandleFromManager() );	

		int ret = EPHIDGET_OK;
		ret = CPhidgetManager_close( mManagerHandle );
		assert( ret==EPHIDGET_OK );
	
		ret = CPhidgetManager_delete( mManagerHandle );
		assert( ret==EPHIDGET_OK );
		mManagerHandle = NULL;
	}
}

const char* DeviceManager::getLibraryVersion()
{
	const char* version = NULL;
	int ret = CPhidget_getLibraryVersion( &version );
	assert( ret==EPHIDGET_OK );
	return version;
}

const char*	DeviceManager::getErrorDescription( int errorCode )
{
	const char* description = NULL;
	int ret = CPhidget_getErrorDescription( errorCode, &description );
	assert( ret==EPHIDGET_OK );
	return description;
}

void DeviceManager::update()
{
	if ( !mManagerHandle )
		return;
	
	updateDeviceList();
	
	for ( std::size_t i=0; i<mDevices.size(); ++i )
		mDevices[i]->update();
}

void DeviceManager::updateDeviceList()
{
	assert( mManagerHandle );
	
	// Get the list of connected Phidget devices
	int ret = EPHIDGET_OK;
	CPhidgetHandle* currentDeviceHandles = NULL;
	int currentDeviceCount = 0;
	ret = CPhidgetManager_getAttachedDevices( mManagerHandle, &currentDeviceHandles, &currentDeviceCount );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK )
		return;
	
	// Identify the devices to delete
	std::vector<CPhidgetHandle> devicesToDelete;
	for ( std::size_t i=0; i<mDevices.size(); ++i )
	{
		CPhidgetHandle existingDeviceHandle = mDevices[i]->getPhidgetHandleFromManager();
		bool foundInCurrentDevices = false;
		for ( int j=0; j<currentDeviceCount; ++j )
		{
			if ( currentDeviceHandles[j]==existingDeviceHandle )
			{
				foundInCurrentDevices = true;
				break;
			}
		}
		if ( !foundInCurrentDevices )
			devicesToDelete.push_back( existingDeviceHandle );
	}

	// Identify the devices to add
	std::vector<CPhidgetHandle> devicesToAdd;
	for ( int i=0; i<currentDeviceCount; ++i )
	{
		CPhidgetHandle currentDeviceHandle = currentDeviceHandles[i];
		bool foundInExistingDevices = false;
		for ( std::size_t j=0; j<mDevices.size(); ++j )
		{
			if ( mDevices[j]->getPhidgetHandleFromManager()==currentDeviceHandle )
			{
				foundInExistingDevices = true;
				break;
			}
		}
		if ( !foundInExistingDevices )
			devicesToAdd.push_back( currentDeviceHandle );
	}
	
	// Delete detached devices
	for ( std::size_t i=0; i<devicesToDelete.size(); ++i )
		deleteDevice( devicesToDelete[i] );
	
	// Add new devices
	for ( std::size_t i=0; i<devicesToAdd.size(); ++i )
		addDevice( devicesToAdd[i] );	

	// Free device list
	ret = CPhidgetManager_freeAttachedDevicesArray( currentDeviceHandles );
	assert( ret==EPHIDGET_OK );		
}

CPhidgetHandle DeviceManager::createDeviceSpecificHandle( int deviceID )
{
	int ret = EPHIDGET_OK;
	CPhidgetHandle result = NULL;
	switch ( deviceID )
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS : 
			ret = CPhidgetSpatial_create( reinterpret_cast<CPhidgetSpatialHandle*>(&result) );
			assert( ret==EPHIDGET_OK );
			break;
		case PHIDID_TEMPERATURESENSOR : 
			ret = CPhidgetTemperatureSensor_create( reinterpret_cast<CPhidgetTemperatureSensorHandle*>(&result) );
			assert( ret==EPHIDGET_OK );
			break;
		default:
			break;
	}
	return result;
}

Device* DeviceManager::createDevice( int deviceID, CPhidgetHandle deviceSpecificHandle )
{
	Device* result = NULL;
	switch ( deviceID )
	{
		case PHIDID_SPATIAL_ACCEL_GYRO_COMPASS : 
			result = new Spatial( deviceSpecificHandle );
			break;
		case PHIDID_TEMPERATURESENSOR : 
			result = new TemperatureSensor( deviceSpecificHandle );
			break;
		default:
			break;
	}
	return result;
}

void DeviceManager::addDevice( CPhidgetHandle phidgetHandle )
{
	// Get the device ID
	int ret = EPHIDGET_OK;
	CPhidget_DeviceID deviceID = static_cast<CPhidget_DeviceID>(0);
	ret = CPhidget_getDeviceID( phidgetHandle, &deviceID );
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK )
		return;
 	
	// Get the serial number
	int serialNumber = 0;
	ret = CPhidget_getSerialNumber( phidgetHandle, &serialNumber);
	assert( ret==EPHIDGET_OK );
	if ( ret!=EPHIDGET_OK )
		return;
 	
	// Create a handle specific to the type of Phidget and return it as a generic one
	CPhidgetHandle deviceSpecificHandle = createDeviceSpecificHandle( deviceID );
	if ( !deviceSpecificHandle )
		return;

	// Open the device. Call the DeviceManager sub-class implementation for that
	openDevice( deviceSpecificHandle, serialNumber );
	
	// Wait for attachment
	ret = CPhidget_waitForAttachment( deviceSpecificHandle, 0 );
	assert( ret==EPHIDGET_OK );

	// Now everything is in place for the Device object to be created
	Device* device = createDevice( deviceID, deviceSpecificHandle );
	if ( !device )
		return;

	// Associate the generic/common handle from the DeviceManager to the Device
	device->setPhidgetHandleFromManager( phidgetHandle );
		
	// Add the newly created device
	mDevices.push_back( device );
		
	// Notify
	Listeners listeners = mListeners;		// The copy is on purpose here. It allows client code to add/remove listeners
	for ( Listeners::iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
		(*itr)->onDeviceConnected( this, device );
}

void DeviceManager::deleteDevice( CPhidgetHandle phidgetHandle )
{
	// Find the Device corresponding to the manager Phidget handle
	Devices::iterator itr;
	for ( itr=mDevices.begin(); itr!=mDevices.end(); ++itr )
	{
		if ( (*itr)->getPhidgetHandleFromManager()==phidgetHandle )
			break;
	}

	assert( itr!=mDevices.end() );
	if ( itr==mDevices.end() )
		return;
	Device* device = *itr;
	
	// Notify 
	Listeners listeners = mListeners;		// The copy is on purpose here. It allows client code to add/remove listeners
	for ( Listeners::iterator itrListener=listeners.begin(); itrListener!=listeners.end(); ++itrListener )
		(*itrListener)->onDeviceDisconnecting( this, device );
		
	// Delete the device
	mDevices.erase( itr );
	delete device;
	device = NULL;
}

void DeviceManager::addListener( Listener* listener )
{
	assert(listener);
	mListeners.push_back(listener);
}

bool DeviceManager::removeListener( Listener* listener )
{
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

}