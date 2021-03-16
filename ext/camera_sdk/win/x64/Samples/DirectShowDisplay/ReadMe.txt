Copyright (c) 2014, Pleora Technologies Inc., All rights reserved.

================
DirectShowDsplay
================

Shows how to use the eBUS SDK DirectShow source filter to receive images
from a GigE Vision or USB3 Vision device which is also being controlled.

The images are simply sent to the Microsoft Video Mixing Renderer 9.

It is possible to control the eBUS SDK DirectShow source filter using 
its IPvDSSource COM interface.

This sample focuses mainly on using the eBUS SDK DirectShow Source filter and
assumes good knowledge of DirectShow and COM.

0. DirectShow filter

The filter is not registered during the installation. You will need to do so in order to use the samples.

On Windows 32-bit:
 1. Start a command prompt as an administrator
 2. Change the directory to the eBUS SDK directory (cd C:\Program Files\Common Files\Pleora\eBUS SDK)
 3. Register the filter (regsvr32 PvDSSource.ax)

On Windows 64-bit:
 1. Start a command prompt as an administrator
 
 2. Register the 64-bit filter
 2a. Change the directory to the eBUS SDK 64-bit directory (cd C:\Program Files\Common Files\Pleora\eBUS SDK)
 2b. Register the filter (regsvr32 PvDSSource64.ax)
 
 3. Register the 32-bit filter
 3a. Change the directory to the eBUS SDK 32-bit directory (cd C:\Program Files (x86)\Common Files\Pleora\eBUS SDK)
 3b. Register the filter (regsvr32 PvDSSource.ax)
 

1. Introduction

Using the eBUS SDK DirectShow Source filter, this sample shows how to:
 * Instantiate the filter
 * Configure the filter
 * Add the filter to a graph
 * Connect the filter to a DirectShow display filter
 * Start streaming
 * Stop streaming


2. Pre-conditions

This sample assumes that:
 * you have a GigE Vision Device connected to a network adapter or a USB3 Vision device connected to a USB 3.0 interface
 * you registered the eBUSDirectShow filter (see step 0)


3. Description

3.1 DirectShowDisplay.cpp.

Shows how to use the eBUS SDK DirectShow filter for image acquisition and display. Please refer to the comments in the source 
code for more information regarding the methods used in this sample.