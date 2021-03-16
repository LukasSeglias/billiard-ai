Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.

============
SerialBridge
============

1. Introduction

This sample shows how to use the SerialBridge class control a serial device
(usually a camera) connected to a GigE Vision or USB3 Vision device from either a
camera configuration application or a CLProtocol GenICam interface.

After connecting to the device, the sample prompts you to select one 
of the three supported serial device interfacing scenario:

A. Serial COM port link: through a NULL modem connection, allows a configuration
   application expecting to interact with the serial device through a serial COM port
   on the host to interact with the serial device attached to one of the serial
   ports of the GigE Vision or USB3 Vsion device.
   
B. Camera Link DLL: with the eBUS SDK Camera Link DLL (clserpte.dll) allows
   a configuration application expecting to interact the the serial device through
   the standardized Camera Link DLL interface to interact with the serial device
   attached to one of the serial ports of the GigE Vision device.


2. Pre-requisites

This sample assumes that:

* You have a GigE Vision or USB3 Vision device with a serial device (usually a camera) attached to it.

* For scenario A, serial COM port link: you have a NULL modem between 2 ports, either
  virtual (software) or with real ports and a NULL modem cable. You also have an application
  from the camera vendor to control the camera through a serial COM port.
  
* For scenario B, Camera Link DLL: you have the eBUS SDK Camera Link DLL (clserpte.dll)
  available properly configured in the Windows Registry:
	The CLSERIALPATH registry string under HKEY_LOCAL_MACHINE\SOFTWARE\CameraLink
    is not defined or not pointing the the clserpte.dll eBUS SDK Camera Link DLL path
  You also have an application from the camera vendor to control the camera using
  a Camera Link DLL.


3. Description

3.1 SerialBridge.cpp

Contains the main entry point to the sample. Displays a device finder allowing the user
to select a device. Then connects the device and prompts the user which serial bridge
scenario should be demonstrated.

3.2 SeriaCOMPortBridge.cpp

Shows how to setup a serial bridge through a NULL modem.

3.3 CameraLinkDLLBridge.cpp

Shows how to setup a serial bridge through the eBUS SDK Camera Link DLL, clserpte.dll.

