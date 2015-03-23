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

#include "RPhiDeviceManager.h"

namespace RPhi
{

/*
	RemoteDeviceManager
*/
class RemoteDeviceManager : public DeviceManager
{
public:
	RemoteDeviceManager( const std::string& serverID, const std::string& password );
	RemoteDeviceManager( const std::string& serverAddress, int port, const std::string& password );
	
	bool					openedUsingServerID() const		{ return mOpenedUsingServerID; }
	const std::string&		getPassword() const				{ return mPassword; }

	const std::string&		getServerID() const				{ return mServerID; }
	
	const std::string&		getServerAddress() const		{ return mServerAddress; }
	int						getPort() const					{ return mPort; }
	
	bool					isConnected() const;

protected:
	virtual void			openDevice( CPhidgetHandle phidgetHandle, int serialNumber );

private:
	bool					mOpenedUsingServerID;
	std::string				mPassword;

	std::string				mServerID;
	
	std::string				mServerAddress;
	int						mPort;
};

}