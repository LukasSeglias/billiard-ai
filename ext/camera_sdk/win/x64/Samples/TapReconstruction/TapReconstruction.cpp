// *****************************************************************************
//
//    Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// Shows how to use a PvTapFilter object to perform tap reconstruction on a PvBuffer
// containing an image payload.
//

#include <PvSampleUtils.h>
#include <PvTapFilter.h>
#include <PvBufferWriter.h>

PV_INIT_SIGNAL_HANDLER();

#define IMAGE_WIDTH ( 640 )
#define IMAGE_HEIGHT ( 480 )


//
// Main function
//
int main()
{
    PV_SAMPLE_INIT();

    // Create input buffer, alloc as an image
    PvBuffer lInput;
    PvImage *lInputImage = lInput.GetImage();
    lInputImage->Alloc( IMAGE_WIDTH, IMAGE_HEIGHT, PvPixelMono8 );

    // Fill input buffer with ramp: easy pointer scrolling, no padding and known pixel type
    uint8_t *lPtr = lInputImage->GetDataPointer();
    for ( uint32_t y = 0; y < lInputImage->GetHeight(); y++ )
    {
        uint32_t lCount = y;
        for ( uint32_t x = 0; x < lInputImage->GetWidth(); x++ )
        {
            *( lPtr++ ) = ( lCount++ ) % 256;
        }
    }

    // Create output buffer
    PvBuffer lOutput;

    // Create tap filter, select geometry
    PvTapFilter lFilter;

    // Normally, you would select the correct tap geometry and execute the filter:
    //
    // lFilter.SetGeometry( PvTapGeometryAS_2X_2YE );
    // lFilter.Execute( &lInput, &lOuput );
    //
    // ...but for education purpose wew are going to enumerate all supported tap 
    // geometries and run the tap filter for all those tap geometries, optionally
    // save the output image in a TIFF file
    //
    for ( uint32_t i = 0; i < PvTapFilter::GetSupportedGeometryCount(); i++ )
    {
        // Get information about i-th tap geometry
        std::string lName = PvTapFilter::GetSupportedGeometryName( i ).GetAscii();
        PvTapGeometryEnum lValue = PvTapFilter::GetSupportedGeometryValue( i );

        // Select tap geometry
        std::cout << "==> Selecting " << lName << std::endl;
        lFilter.SetGeometry( lValue );

        // Execute tap filter
        PvResult lResult = lFilter.Execute( &lInput, &lOutput );
        if ( lResult.IsOK() )
        {
            // Success!
            std::cout << "  Tap filter executed successfully" << endl;
        }
        else
        {
            // Error
            std::cout << "  Error executing tap filter: " << 
                lResult.GetCodeString().GetAscii() << " " << 
                lResult.GetDescription().GetAscii() << endl;
        }

        // Save output image as tiff
        PvBufferWriter lWriter;
        std::string lFilename = lName + ".tiff";
        lWriter.Store( &lOutput, lFilename.c_str(), PvBufferFormatTIFF );
    }

    cout << endl;
    cout << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}


