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
typedef struct _CPhidget *CPhidgetHandle;

namespace RPhi
{

/*
	Device

	The Device is the base class for all Phidget devices

	User guide
	http://www.phidgets.com/docs/General_Phidget_Programming

	C-API reference
	http://www.phidgets.com/documentation/web/cdoc/index.html
	http://www.phidgets.com/documentation/web/cdoc/group__phidcommon.html
*/
class Device
{
public:
	virtual void		update() {}
	
	enum Type
	{
		kSpatial,
		kTemperatureSensor
	};

	Type				getType() const				{ return mType; }	

	CPhidgetHandle		getPhidgetHandle() const	{ return mPhidgetHandle; }
	const std::string&	getName() const				{ return mName; }
	int					getSerialNumber() const		{ return mSerialNumber; }
	int					getVersion() const			{ return mVersion; }
	bool				isAttached() const;		
	const std::string&	getTypeName() const			{ return mTypeName; }
	const char*			getLabel() const;
	void				setLabel( const char* label );
	
	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void onDeviceChanged( Device* /*device*/ ) {}
	};

	void				addListener( Listener* listener );
	bool				removeListener( Listener* listener );
	void				removeListeners();
	
	virtual std::string toString() const;

protected:
	Device( Type type, CPhidgetHandle phidgetSpecificHandle );
	
	friend class DeviceManager;
	virtual ~Device();

	CPhidgetHandle		getPhidgetHandleFromManager() const { return mPhidgetHandleFromManager; }
	void				setPhidgetHandleFromManager( CPhidgetHandle phidgetHandle ) { mPhidgetHandleFromManager = phidgetHandle; }
	
	void				getInformation();
	
	typedef				std::vector<Listener*> Listeners; 
	const Listeners&	getListeners() const { return mListeners; }	

private:
	CPhidgetHandle		mPhidgetHandleFromManager;
	CPhidgetHandle		mPhidgetHandle;
	Type				mType;
	std::string			mName;
	int					mSerialNumber;
	int					mVersion;
	std::string			mTypeName;
	Listeners			mListeners;
};

}