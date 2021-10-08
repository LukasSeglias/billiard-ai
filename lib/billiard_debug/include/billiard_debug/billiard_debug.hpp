#pragma once

#include "macro_definition.hpp"
#include <mutex>
#include <sstream>

//#define BILLIARD_DEBUG // TODO: Remove

#ifndef BILLIARD_DEBUG
    #ifdef NDEBUG
        #undef BILLIARD_DEBUG
    #endif
    #ifndef NDEBUG
        #define BILLIARD_DEBUG 1
    #endif
#endif

#ifdef BILLIARD_DEBUG
    #define DEBUG(x) billiard::debug::Debug{} << x
#else
    #define DEBUG(x) do {} while (0)
#endif

namespace billiard::debug {

    /**
    * Source: https://stackoverflow.com/a/41602842
    * Thread safe cout class
    * Example of use:
    *    Debug{} << "Hello world!" << std::endl;
    */
    class EXPORT_BILLIARD_DEBUG_LIB Debug: public std::ostringstream {
    public:
        Debug() = default;
        ~Debug() override;

        static bool lock();
        static void unlock();

    private:
        static std::mutex _mutexPrint;
    };

}