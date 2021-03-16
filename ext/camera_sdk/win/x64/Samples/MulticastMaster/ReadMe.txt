Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.

===============
MulticastMaster
===============

This sample shows how to use the PvDevice to control a multicast master. Once connected as the master, you can run the MulticastSlave sample to connect as a slave. You must also connect to a switch that supports IGMP (Internet Group Management Protocol).

1. Introduction

We use the PvDevice to
 * Connect to a device
 * Configure the device for multicasting
 * We do not receive data on this master (however, the master is capable of receiving data)
 * Start streaming
 * Stop streaming

2. Pre-conditions

This sample assumes that:
 * You have a GigE Vision Device connected to a network adapter.

3. Description

3.1 MulticastMaster.cpp

Shows how to use the PvDevice class to control a multicast master. Please refer to the comments in the source code for information on the methods used in this sample.

