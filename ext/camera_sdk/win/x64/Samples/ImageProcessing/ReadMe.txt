Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.

==============
ImageProcessing
==============

This sample shows how to use the PvStream class for image acquisition and processing using memory allocated
from a buffer belonging to a third-party processing library. The ImageProcessing library 
is only provided for this sample and is not supported.

1. Introduction

Using the PvStream class, this sample shows how to:
 * Connect to a device
 * Setup the stream
 * Retrieve images from the device
 * Attach a third party buffer to PvBuffer::PvImage class
 * Monitor statistics
 * Stop streaming


2. Pre-conditions

This sample assumes that:
 * you have a GigE Vision Device connected to a network adapter or a U3V device connected to a USB 3.0 interface.

3. Description

3.1 ImageProcessing.cpp

Shows how to use the PvStream class for image acquisition and processing using a third party buffer. Please refer to the comments in the source code for information on the methods used in this sample.

