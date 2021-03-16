Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.

==================
TransmitTiledImages
==================

This sample illustrates how to use the eBUS SDK to receive, transform, and transmit images. More specifically, it is an MFC GUI application that receives video from up to four different GigE Vision or USB3 Vision transmitters, tiles the images together, transmits the output to a given destination, and finally displays a the tiled output to the monitor. This sample is designed more for use as an application than as illustrative source code. As a result, it is recommended that application developers begin by reading and understanding a simpler example such as TransmitTestPattern before attempting to dissect the source code of TransmitTiledImages.

1. Introduction

Basic instructions for using the sample:
	1. Start TransmitTiledImages.
	2. Click one of the four button "Connect/Open" to select a GigE Vision/USB3 Vision transmitter.
	3. Select a transmitter from the list of available devices (if GigE Vision, the transmitter should be on the same subnet as the network interface you wish to receive on).
	4. You can use the buttons "..." to customized the configuration of transmitter.
	5. Repeat the steps 2 and 3 to connect up to 4 GigE Vision/USB3 Vision devices.
	6. In the "Video Output" frame, configure the tiled image configuration.
	7. In the "Transmission" frame, configure the transmitter stream information.
	8. Start your GigE Vision receiver and listen for the configuration of the transmitter done in the step 7.
	9. Click "Play" in the application. You should see a tiled input appearing in the application and in the GigE Vision receiver.

2. Prerequisites

This sample assumes that:
 * You have at least one network adapter installed on your PC with a valid IP address.
 * You have a GigE Vision receiver that can receive and display the transmitted data (such as a vDisplay, eBUSPlayer, NetCommand or any other GigE Vision receiver that supports the GVSP protocol). The receiver should be reachable and on the same subnet as the interface from which it will be receiving.
 * You have at least one GigE Vision or USB3 Vision transmitter connected to your PC. The transmitter should be reachable. If the transmitter is using GigE Vision to transport data, the device should be on the same subnet as the interface to which it will be transmitting.
 * Each interface (network adapter) you use should be on a different subnet.
 * You are aware of the amount of bandwidth that is available for transmitting and receiving video data. For best results, use a different network adapter for transmitting than the one that is receiving and ensure that your desired throughput is less than your theoretical bandwidth (typically less than the max interface speed upstream or downstream).

3. Description

This sample show a how to tile multiple sources into a resulting image and transfer it to the network. 
Each of the GigE Vision/USB3 Vision transmitter source are managed by independent threads (StreamThread). The latest image acquire by the thread is stored in the SmartBuffer who is stored in the CurrentBuffersTable. 
The transmitter thread is implemented in the TransmitterThread class. This class will get a snapshot of the image from the currentBuffersTable to transform it into the tiled image after multiple transformations. These transformations are executed in ImageBuffer and the tiling process is executed with the CImage MFC/ATL library. On the 4 source images are tiled in a resulting image, this image is sent out by this thread. 
To avoid as most as possible the memory copy, which is a long operation, the SmartBuffer are used. These buffers have a use count and the system is recycling then when the use count is 0.





