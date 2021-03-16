// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Persistent.h"


///
/// \brief Constructor
///

Persistent::Persistent()
{
}


///
/// \brief Returns true if the image filtering configuration has changed
///

bool Persistent::HasChanged()
{
    // Save filter configuration to writer
    PvConfigurationWriter lWriter;
    Save( &lWriter );

    // Save configuration to a string
    PvString lNow;
    lWriter.SaveToString( lNow );

    // Compare what we now have with the baseline
    return mBaseline != lNow;
}


///
/// \brief Resets the configuration baseline for HasChanged test
///

void Persistent::ResetChanged()
{
    // Save filter configuration to writer
    PvConfigurationWriter lWriter;
    Save( &lWriter );

    // Save configuration baseline to a string
    lWriter.SaveToString( mBaseline );
}

