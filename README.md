RapaPhidget
=================
A C++ object-oriented and cross-platform library for accessing Phidgets devices
![alt text](docs/RapaPhidget.jpg?raw=true "A couple of Phidget devices")

# Overview
Phidgets are little USB devices manufactured by [Phidgets Inc](http://www.phidgets.com/).

Although an official API/lib is provided by the manufacturer for many languages, there's no proper object-oriented one for C++. There's only a C-function style API.

RapaPhidget is an attempt at providing a collection of C++ classes for manipulating Phidgets devices as simply and elegantly as possible.

RapaPhidget is cross-platform and is available on Windows (x86 or x64), MacOS and Linux. At the moment it only supports the following devices:
* Spatial
* Temperature sensor

New types of Phidget devices should be easy to add.
