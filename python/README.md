Kinect Common Bridge 2.0 - Python Bindings
========================

## Introduction

Holds the results of the research and development effort to wrap KCB library with python bindings.

Approaches under research:
- pybindgen **(POC working)**
- SWIG
- boost:python
- Cython 
- Custom parser (Python C API)

## Requirements

The Hardware and Software below are required to build the library:

- A **Kinect for Windows V2 sensor**:
	A Kinect for Windows V2 Sensor is required to capture sound and images that will be used by the Kinect Common Bridge 2.0 on top of the Kinect for Windows SDK. You can either [order it online](http://www.microsoft.com/en-us/kinectforwindows/) or purchase it at a Microsoft Store.

- **Visual Studio 2010**:
	The library builds with [Visual Studio](http://www.visualstudio.com/) 2010. This is a requirement since Python 3.4 is built with this version of VS and so should be the extension modules. 

- **Python 3.4**:
	Bindings require you have [Python 3.4+](https://www.python.org/downloads/) installed on your machine with all required environment variables set up and available via system ```PATH```.	
	
- **Kinect for Windows SDK**:
	In order to build this library, you first need to download the SDK from the [Kinect for Windows Developer site](http://www.microsoft.com/en-us/kinectforwindows/develop/). 

## Troubleshooting

Various binding generation engines will require it's own prerequisites. Most of the python modules can be obtained via:
```cmd
easy_install <module-name>
```
or 
```cmd
pip install <module-name>
```

## Samples

Versions of bindings can be found in respective folders under ```python``` directory. The only working version now is the one using ```pybindgen```.
