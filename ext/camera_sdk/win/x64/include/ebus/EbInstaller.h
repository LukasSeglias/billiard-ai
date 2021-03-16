// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __EBINSTALLER_H__
#define __EBINSTALLER_H__

#include "EbInstallerLib.h"
#include "EbNetworkAdapter.h"
#include "PtResult.h"
#include "PtTypes.h"


namespace EbInstallerLib
{
    class Installer;
};

class EB_INSTALLER_LIB_API EbInstaller
{
public:
    
    EbInstaller();
    virtual ~EbInstaller();

    PtResult Initialize();

    uint32_t GetDriverCount() const;
    const EbDriver* GetDriver( uint32_t aIndex ) const;
    PtResult IsInstalled( const EbDriver* aDriver, bool& aInstalled ) const;
    PtResult Install( const EbDriver* aDriver );
    PtResult Update( const EbDriver* aDriver );
    PtResult Uninstall( const EbDriver* aDriver );

    bool IsRebootNeeded() const;

    uint32_t GetNetworkAdapterCount() const;
    EbNetworkAdapter* GetNetworkAdapter( uint32_t aIndex );

private:

#ifndef DOXYGEN
    EbInstallerLib::Installer* mThis;
#endif // DOXYGEN

};

#endif
