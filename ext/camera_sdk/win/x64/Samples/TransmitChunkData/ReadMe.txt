Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.

===================
TransmitChunkData
===================

1. Description

This samples shows how to use the transmitter in order to send a mix of image and chunk data blocks. 
The image block contains a dynamically generated test pattern and the chunk block contains a LFSR pattern.

2. Prerequisites

This sample assumes that:
 * You have a network adapter installed on your PC with a valid IP address.
 * You have a GigE Vision receiver that can receive and display the test pattern (such as eBUSPlayer or any other GigE Vision receiver that supports the GVSP protocol). The receiver should be reachable and on the same subnet as the interface from which it will be receiving.
 * You have an eBUS SDK video server license installed on your system

3. Description

3.1 TransmitChunkData.cpp

Sample code as described above. Please refer to the comments in the source code for information on the methods used in this sample.

3.2 DataGenerator.cpp

Class used to manage buffers and generate the image test pattern and chunk data LFSR.


IMPORTANT: If you do not have an eBUS SDK video server license installed on your system,
the sample will not work as expected.




