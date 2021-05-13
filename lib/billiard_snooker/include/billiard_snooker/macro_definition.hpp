#pragma once
#if defined(_MSC_VER)
// Windows
    #if defined(BILLIARD_SNOOKER_IMPORT)
        #define EXPORT_BILLIARD_SNOOKER_LIB __declspec(dllimport)
    #else
        #define EXPORT_BILLIARD_SNOOKER_LIB __declspec(dllexport)
    #endif
#elif defined(__GNUC__)
//  GCC
    #define EXPORT_BILLIARD_SNOOKER_LIB __attribute__((visibility("default")))
#else
    //  do nothing and hope for the best?
    #define EXPORT_BILLIARD_SNOOKER_LIB
    #pragma warning Unknown dynamic link import/export semantics.
#endif