// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvDeviceFinderWnd.h>
#include <PvDeviceInfoGEV.h>

///
/// \class StreamDeviceFinderWnd
///
/// \brief Overload of PvDeviceFinderWnd to filter out what cannot send data
///
class StreamDeviceFinderWnd
    : public PvDeviceFinderWnd
{
public :

    ///
    /// \brief Constructor
    ///

    StreamDeviceFinderWnd( const CString &aIfAddress )
        : PvDeviceFinderWnd()
        , mIfAddress( aIfAddress )
    {
    }

    ///
    /// \brief Destructor
    ///

    virtual ~StreamDeviceFinderWnd()
    {
    }

    ///
    /// \brief Filter the device found to only enumerate the one that can stream and is not our virtual device.
    ///
    /// \param aDI Device discovered.
    /// \param true to keep the device.
    ///

    virtual bool OnFound( const PvDeviceInfo *aDI )
    {
        const PvDeviceInfoGEV *lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV *>( aDI );
		if ( lDeviceInfoGEV != NULL )
		{
			if ( lDeviceInfoGEV->GetIPAddress() == (LPCTSTR) mIfAddress )
			{
				return false;
			}
		}

        return true;
    }

private:

    CString mIfAddress;
};


