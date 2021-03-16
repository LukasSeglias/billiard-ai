// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENAPI_H__
#define __PVGENAPI_H__

// Namespaces
#ifdef WIN32
    #define PV_GENAPI_NS GenApi_3_0_PT
    #define PV_GENICAM_NS GenICam_3_0_PT
#else
    #define PV_GENAPI_NS GenApi_3_0
    #define PV_GENICAM_NS GenICam_3_0
#endif

// All GenApi/GenICam forward declarations
namespace PV_GENAPI_NS
{
    class FileProtocolAdapter;
    class CChunkAdapter;
    class CNodeMapRef;
    class CEventAdapter;

    struct INodeMap;
    struct INode;
    struct IEnumEntry;
    struct IBoolean;
    struct ICommand;
    struct IEnumeration;
    struct IFloat;
    struct IInteger;
    struct IString;
    struct IRegister;
    struct ICategory;
    struct IPort;
    struct ICategory;
    struct IValue;

};

#endif
