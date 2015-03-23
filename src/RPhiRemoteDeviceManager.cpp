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
#include "RPhiRemoteDeviceManager.h"

#include <assert.h>
#include <phidget21.h>

namespace RPhi
{

RemoteDeviceManager::RemoteDeviceManager( const std::string& serverID, const std::string& password )
	: DeviceManager(),
	  mOpenedUsingServerID( true ),
	  mPassword( password ),
	  mServerID( serverID ),
	  mServerAddress(),
	  mPort(0)
{
	openRemotelyWithServerID( serverID, password );
}

RemoteDeviceManager::RemoteDeviceManager( const std::string& serverAddress, int port, const std::string& password )
	: DeviceManager(), 
	  mOpenedUsingServerID( false ),
	  mPassword( password ),
	  mServerID(),
	  mServerAddress( serverAddress ),
	  mPort( port )
{
	openRemotelyWithServerAddress( serverAddress, port, password );
}

bool RemoteDeviceManager::isConnected() const
{
	int result = 0;
	int ret = CPhidgetManager_getServerStatus( getPhidgetManagerHandle(), &result );
	assert( ret==EPHIDGET_OK );
	bool connected = ( result==PHIDGET_ATTACHED );
	return connected;
}

void RemoteDeviceManager::openDevice( CPhidgetHandle phidgetHandle, int serialNumber )
{
	int ret = EPHIDGET_OK;
	if ( mOpenedUsingServerID )
	{ 
		ret = CPhidget_openRemote( phidgetHandle, serialNumber, getServerID().c_str(), getPassword().c_str() );
		assert( ret==EPHIDGET_OK );
	}
	else
	{
		ret = CPhidget_openRemoteIP( phidgetHandle, serialNumber, getServerAddress().c_str(), getPort(), getPassword().c_str() );
		assert( ret==EPHIDGET_OK );
	}
}

}