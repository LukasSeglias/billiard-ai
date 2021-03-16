Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.

=================
CameraBridge
=================

This C++ sample shows a simple eBUS SDK application interacting with a frame 
grabber and its attached camera.


1. Description

This sample represents a simpler version of the eBUS Player application with basic support
for a Camera Link bridge to the camera.

It allows the user to connect to a device and control a stream of images coming
from the frame grabber while also controlling its attached camera.

The camera can be controlled using either a properly configured CL Protocol library (for Windows
only, provided by the camera manufacturer) or ideally using GenCP if supported by the camera.

The Pleora Camera Files are not available yet. Contact your Pleora representative if you would
like to learn more about this feature.


2. Prerequisites

This sample assumes that:
 * You have a GigE Vision frame grabber is connected to the network or an USB3 Vision device connected to the USB3 interface. 
 * You have a camera attached to the frame grabber and
     1. You have a Pleora Camera File for the camera
	 2. The camera supports CL Protocol and you have the CL Protocol library installed and properly configured
	 3. The camera supports the GenCP protocol which is natively supported by the camera bridge


3. Description

3.1 CameraBridge.cpp

Contains the entry point to the sample. Displays a devicer finder and prompts the user which
camera bridge scenario should be demonstrated.

3.2 CLProtocol.cpp

Shows how to use the camera bridge with GenICam CLProtocol.

3.3 GenCP.cpp

Shows how to use the camera bridge with GenCP.

3.4 PleoraCameraFile.cpp

Shows how to use the camera bridge with a Pleora Camera File.

The Pleora Camera Files are not available yet. Contact your Pleora representative if you would like to learn more about this feature.