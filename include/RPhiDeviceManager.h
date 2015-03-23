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

#include <string>
#include <vector>
typedef struct _CPhidgetManager *CPhidgetManagerHandle;		
typedef struct _CPhidget *CPhidgetHandle;

namespace RPhi
{

class Device;

/*	
	DeviceManager

	The DeviceManager is responsible for enumerating all the Phidget devices
	connected to the machine. The manager keeps track of the devices and 
	make them avalaible via a list of Device objects that it maintains.

	This DeviceManager class is abstract. Only LocalDeviceManager and 
	RemoteDeviceManager classes can be instantiated

	User guide
	http://www.phidgets.com/docs/Phidget_Manager

	C-API reference
	http://www.phidgets.com/documentation/web/cdoc/index.html
	http://www.phidgets.com/documentation/web/cdoc/group__phidmanager.html
*/
class DeviceManager
{
public:
	virtual ~DeviceManager();

	bool					isLocal() const				{ return mIsLocal; }
	typedef std::vector<Device*> Devices;
	const Devices&			getDevices() const			{ return mDevices; }

	void					update();

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void onDeviceConnected( DeviceManager* /*deviceManager*/, Device* /*device*/ ) {}
		virtual void onDeviceDisconnecting( DeviceManager* /*deviceManager*/, Device* /*device*/ ) {}
	};

	void					addListener( Listener* listener );
	bool					removeListener( Listener* listener );

	static const char*		getLibraryVersion();
	static const char*		getErrorDescription( int errorCode );

protected:
	DeviceManager();


	CPhidgetManagerHandle	getPhidgetManagerHandle() const { return mManagerHandle; }
	
	void					openLocally();
	void					openRemotelyWithServerID( const std::string& serverID, const std::string& password );
	void					openRemotelyWithServerAddress( const std::string& address, int port, const std::string& password );

	virtual void			openDevice( CPhidgetHandle phidgetHandle, int serialNumber ) = 0;

private:
	void					updateDeviceList();

	static CPhidgetHandle	createDeviceSpecificHandle( int deviceID );
	static Device*			createDevice( int deviceID, CPhidgetHandle deviceSpecificHandle );
	void					addDevice( CPhidgetHandle phidgetHandle );
	void					deleteDevice( CPhidgetHandle phidgetHandle );

	CPhidgetManagerHandle	mManagerHandle;
	bool					mIsLocal;
	Devices					mDevices;
	typedef					std::vector<Listener*> Listeners; 
	Listeners				mListeners;
};

}