#pragma once

#include "type.hpp"
#include "macro_definition.hpp"

extern "C" {

    EXPORT_UNITY_ADAPTER_LIB void onStart();
    EXPORT_UNITY_ADAPTER_LIB void onTearDown();
    EXPORT_UNITY_ADAPTER_LIB void processEvents();

    ///////////////////////////////////////////////////////
    //// Application Interface
    ///////////////////////////////////////////////////////
    EXPORT_UNITY_ADAPTER_LIB void onAnimationChangedEvent(AnimationChangedEventCallback callback);
    EXPORT_UNITY_ADAPTER_LIB void onStateChangedEvent(StateChangedEventCallback callback);

    EXPORT_UNITY_ADAPTER_LIB void configuration(Configuration config);
    EXPORT_UNITY_ADAPTER_LIB void capture();
    EXPORT_UNITY_ADAPTER_LIB void image();
    EXPORT_UNITY_ADAPTER_LIB void video();
    EXPORT_UNITY_ADAPTER_LIB void search(Search search);

    ///////////////////////////////////////////////////////
    //// For development purposes only
    ///////////////////////////////////////////////////////

    EXPORT_UNITY_ADAPTER_LIB void state(State state);
    EXPORT_UNITY_ADAPTER_LIB void debugger(Debugger debugger);
}
