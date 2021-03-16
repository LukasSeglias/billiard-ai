// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPVDSSOURCE_H__
#define __IPVDSSOURCE_H__

#ifdef __cplusplus
extern "C" 
{
#endif

    // {7D692BC5-05D4-4032-A581-B7FA1C202825}
    DEFINE_GUID(IID_IPvDSSource, 
    0x7d692bc5, 0x5d4, 0x4032, 0xa5, 0x81, 0xb7, 0xfa, 0x1c, 0x20, 0x28, 0x25);

    DECLARE_INTERFACE_(IPvDSSource, IUnknown)
    {
        // Configuration
        STDMETHOD( get_Role )( THIS_ int *role ) PURE;
        STDMETHOD( put_Role )( THIS_ int role ) PURE;
        STDMETHOD( get_UnicastPort )( THIS_ int *port ) PURE;
        STDMETHOD( put_UnicastPort )( THIS_ int port ) PURE;
        STDMETHOD( get_MulticastIP )( THIS_ BSTR *ip ) PURE;
        STDMETHOD( put_MulticastIP )( THIS_ BSTR ip ) PURE;
        STDMETHOD( get_MulticastPort )( THIS_ int *port ) PURE;
        STDMETHOD( put_MulticastPort )( THIS_ int port ) PURE;

        // Diagnostic
        STDMETHOD( get_DiagnosticEnabled )( THIS_ BOOL *enabled ) PURE;
        STDMETHOD( put_DiagnosticEnabled )( THIS_ BOOL enabled ) PURE;

        // DeviceID
        STDMETHOD( get_DeviceID )( THIS_ BSTR *deviceid ) PURE;
        STDMETHOD( put_DeviceID )( THIS_ BSTR deviceid ) PURE;
        STDMETHOD( DisconnectDevice )( THIS_ ) PURE;
        STDMETHOD( ConnectIfNeeded )( THIS_ ) PURE;

        // Source
        STDMETHOD( get_SourceCount )( THIS_ int *sourcecount ) PURE;
        STDMETHOD( get_SourceName )( THIS_ int aIndex, BSTR *sourcename ) PURE;
        STDMETHOD( get_Source )( THIS_ BSTR *source ) PURE;
        STDMETHOD( put_Source )( THIS_ BSTR source ) PURE;
        STDMETHOD( get_Channel )( THIS_ int *channel ) PURE;
        STDMETHOD( put_Channel )( THIS_ int channel ) PURE;
        
        // Configuration
        STDMETHOD( get_BufferCount )( THIS_ int *count ) PURE;
        STDMETHOD( put_BufferCount )( THIS_ int count ) PURE;
        STDMETHOD( get_DefaultBufferSize )( THIS_ int *size ) PURE;
        STDMETHOD( put_DefaultBufferSize )( THIS_ int size ) PURE;
        STDMETHOD( get_DropThreshold )( THIS_ int *threshold ) PURE;
        STDMETHOD( put_DropThreshold )( THIS_ int threshold ) PURE;
        STDMETHOD( get_Width )( THIS_ int *width ) PURE;
        STDMETHOD( put_Width )( THIS_ int width ) PURE;
        STDMETHOD( get_Height )( THIS_ int *height ) PURE;
        STDMETHOD( put_Height )( THIS_ int height ) PURE;
        STDMETHOD( get_AverageFPS )( THIS_ int *fps ) PURE;
        STDMETHOD( put_AverageFPS )( THIS_ int fps ) PURE;

        // GenICam
        STDMETHOD( get_ParametersSelector )( THIS_ int *selector ) PURE;
        STDMETHOD( put_ParametersSelector )( THIS_ int selector ) PURE;
        STDMETHOD( get_ParametersAvailable )( THIS_ BOOL *available ) PURE;
        STDMETHOD( get_ParameterCount )( THIS_ int *count ) PURE;
        STDMETHOD( get_ParameterName )( THIS_ int index, BSTR *name ) PURE;
        STDMETHOD( get_ParameterCategory )( THIS_ BSTR name, BSTR *category ) PURE;
        STDMETHOD( get_ParameterValue )( THIS_ BSTR name, BSTR *value ) PURE;
        STDMETHOD( put_ParameterValue )( THIS_ BSTR name, BSTR value, BSTR *message ) PURE;
        STDMETHOD( get_ParameterType )( THIS_ BSTR name, BSTR *type ) PURE;
        STDMETHOD( get_ParameterAccess)( THIS_ BSTR name, BOOL *available, BOOL *readable, BOOL *writable ) PURE;
        STDMETHOD( ParameterExecute )( THIS_ BSTR name, BSTR *message ) PURE;
        STDMETHOD( get_ParameterPossibleValueCount )( THIS_ BSTR name, int *count ) PURE;
        STDMETHOD( get_ParameterPossibleValue )( THIS_ BSTR name, int index, BSTR *value ) PURE;
        STDMETHOD( get_ParameterVisibility )( THIS_ BSTR name, int *visibility ) PURE;

        // Default setup
        STDMETHOD( MakeDefault )( THIS_ ) PURE;
        STDMETHOD( ResetDefault )( THIS_ ) PURE;
        STDMETHOD( get_DefaultSummary )( THIS_ BSTR *summary ) PURE;

    };

#ifdef __cplusplus
}
#endif

#endif
