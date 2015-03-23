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

namespace RPhi
{

/*
	Vector3

	A templated class for representing a Vector3 value. 
	The class provides data storage but no geometrical methods at all.
*/
template<typename T>
struct Vector3
{
public:
	Vector3() 
		: mX(0), mY(0), mZ(0)
	{
	}
	
	Vector3( T x, T y, T z ) 
		: mX(x), mY(y), mZ(z)
	{
	}
	
	Vector3( const Vector3& other )
	{
		mX = other.mX;
		mY = other.mY;
		mZ = other.mZ;
	}

	Vector3& operator=( const Vector3& other )
	{
		if ( this==&other )
			return *this;
		mX = other.mX;
		mY = other.mY;
		mZ = other.mZ;
		return *this;
	}

	bool operator==( const Vector3& other ) const
	{
		return	( mX==other.mX && 
				  mY==other.mY && 
				  mZ==other.mZ );
	}

	bool operator!=( const Vector3& other ) const
	{
		return	!( *this==other );
	}

	T& x()				{ return mX; }
	const T& x() const	{ return mX; }
		
	T& y()				{ return mY; }
	const T& y() const	{ return mY; }
	
	T& z()				{ return mZ; }
	const T& z() const	{ return mZ; }
	
private:
	T mX;
	T mY;
	T mZ;	
};

typedef Vector3<double>	Vector3d;
typedef Vector3<float>	Vector3f;

}