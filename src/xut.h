#ifndef XUT_H
#define XUT_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define XUT_PLATFORM_WINDOWS
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #define XUT_PLATFORM_APPLE
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    #define XUT_PLATFORM_ANDROID
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__
    #define XUT_PLATFORM_LINUX
    // linux
#elif __unix__ // all unices not caught above
    #define XSLEEP_UNIX
    // Unix
#elif defined(_POSIX_VERSION)
    #define XUT_PLATFORM_POSIX
    // POSIX
#else
#   error "Unknown compiler"
#endif




#ifdef XUT_PLATFORM_LINUX
    #include <unistd.h>
    inline void xutSleep_ms(int ms){ usleep(ms * 1000); }

#elif XUT_PLATFORM_WINDOWS
//#include <windows.h>
//void xutSleep_ms(int ms){ Sleep(ms); }

#endif






#endif // XUT_H
