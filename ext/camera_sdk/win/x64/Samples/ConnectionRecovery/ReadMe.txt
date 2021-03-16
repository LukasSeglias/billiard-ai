Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.

==================
ConnectionRecovery
==================

This sample code illustrates how to create a simple command line
application performing typical image acquisition and fully handles
the interaction required with the PvDevice in order to support
automatic recovery.

1. Introduction

Mostly the PvDevice and PvStream classes, this sample shows how to:
 * Select a device
 * Connect to a device
 * Open a stream
 * Start acquisition on a device
 * Receive images using a PvStream
 * Receive disconnect events/call-back from a PvDevice
 * Handle disconnect events

2. Prerequisites

This sample assumes that:
 * you have a GigE Vision Device connected to a network adapter or a USB3 Vision device connected to a USB 3.0 interface.

3. Description

3.1 ConnectionRecovery.cpp

Shows how to create a simple command line application for image
acquisition with recovery support by using the eBUS SDK.


