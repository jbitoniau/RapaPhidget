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
#include "RPhiDevice.h"

#include <assert.h>
#include <algorithm>
#include <sstream>
#include <phidget21.h>

/*
	Notes:
	* The Device is created by the DeviceManager which gives it its own generic Phidget handle
	* This handle cannot be used to access specific devices, therefore each subclass needs to 
	  create its own type-specific handle (like Spatial handle for example). 
    * The generic handle is still available in this base class but only for the DeviceManager
	  which uses it as a key (to which it associates the Device object)
    * All the methods here act on the specific handle created in the subclass
	* All the non dynamic information of the Device here (like the serial number, the name etc...)
	  are cached into members, so they are still available when the DeviceManager discovers that
	  the Phidget is detached and notify the client code (otherwise, direct calls to the Phidget
	  would fail)

	* Instead of calling waitForAttachment with infinite time, put a timeout in place (say 1 second). 
	  If the device isn't there after that time, exit the constructor with a PhidgetHandle null to 
	  indicate an error. Have the DeviceManager check for that and delete the Device.  
	  This situation happens when trying to access a Phidget which is already opened by another 
	  process

    * Check	who should close/delete the specific phidget
*/
namespace RPhi
{

Device::Device( Type type, CPhidgetHandle phidgetSpecificHandle )
	: mPhidgetHandleFromManager(NULL),
	  mPhidgetHandle(phidgetSpecificHandle),
	  mType(type),
	  mName(),
	  mSerialNumber(0),
	  mVersion(0),
	  mTypeName(),
	  mListeners()
{
}

Device::~Device()
{
}

void Device::getInformation()
{
	int ret = EPHIDGET_OK;

	const char* name = NULL;
	ret = CPhidget_getDeviceName( getPhidgetHandle(), &name );
	assert( ret==EPHIDGET_OK );
	
	int serialNumber = 0;
	ret = CPhidget_getSerialNumber( getPhidgetHandle(), &serialNumber );
	assert( ret==EPHIDGET_OK );
	
	int version = 0;
	ret = CPhidget_getDeviceVersion( getPhidgetHandle(), &version );
	assert( ret==EPHIDGET_OK );
	
	const char* typeName = NULL;
	ret = CPhidget_getDeviceType( getPhidgetHandle(), &typeName );
	assert( ret==EPHIDGET_OK );
		
	mName = name;
	mSerialNumber = serialNumber;
	mVersion = version;
	mTypeName = typeName;
}

bool Device::isAttached() const
{
	int result = 0;
	int ret = CPhidget_getDeviceStatus( getPhidgetHandle(), &result );
	assert( ret==EPHIDGET_OK );
	bool attached = ( result==PHIDGET_ATTACHED );
	return attached;
}	

const char* Device::getLabel() const
{
	const char* label = NULL;
	int ret = CPhidget_getDeviceLabel( getPhidgetHandle(), &label );
	assert( ret==EPHIDGET_OK );
	return label;
}

void Device::setLabel( const char* label )
{
	assert( label );
	int ret = CPhidget_setDeviceLabel( getPhidgetHandle(), label );
	assert( ret==EPHIDGET_OK );
}

void Device::addListener( Listener* listener )
{
	assert(listener);
	mListeners.push_back(listener);
}

bool Device::removeListener( Listener* listener )
{
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

void Device::removeListeners()
{
	Listeners listeners = mListeners; // The copy is on purpose here
	for ( Listeners::iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
		removeListener( *itr );
}

std::string	Device::toString() const
{
	std::stringstream stream;
	stream << "type:" << getType() << " ";
	stream << "name:'" << getName() << "' ";
	stream << "serialNumber:" << getSerialNumber() << " ";
	stream << "version:" << getVersion() << " ";
	stream << "isAttached:" << isAttached() << " ";
	stream << "typeName:'" << getTypeName() << "' ";
	stream << "label:'" << getLabel() << "'";
	return stream.str();
}

}