/*
  Name:         AgniPlatformSpecific.h
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Platform specific data types, byte ordering, packing, and
                platform specific function selector choke point. Make sure that
                both your platform and compiler are handled below. It shouldn't
                be too difficult to port to a new one...
*/

// Multiple include protection...
#ifndef _AGNIPLATFORMSPECIFIC_H_
#define _AGNIPLATFORMSPECIFIC_H_

// Byte ordering... (borrowed from SDL)
#if (defined(__i386__) || defined(__i386)) || \
     defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define AGNI_HOST_BIG_ENDIAN    false
#else
#define AGNI_HOST_BIG_ENDIAN    true
#endif

// Define cross-platform structure packing attribute...

    // Using a real compiler...
    #if defined(__GNUC__)
        #define AGNI_ATTRIBUTE_PACKED __attribute__ ((__packed__))

    // Using Visual C++...
    #elif defined(_MSC_VER)
        #define AGNI_ATTRIBUTE_PACKED #pragma pack(1)

    // Unknown compiler...
    #else
        #error I don't know how to set the structure packing attributes for your compiler...
    #endif

// Primitives whose size we depend on...

    // 32-bit little-endian x86 machine running some flavour of Linux...
    #if (defined(__i686__) || defined(__i586__) || defined(__i486__) || \
         defined(__i386__)) && \
        (defined(Linux) || defined(linux) || defined(__linux))

        // Includes...
        #include <sys/times.h>
        #include <unistd.h>

        // Target platform identification...
        #define AGNI_HOST_TARGET "Linux"

        // Primitives...

            // Signed...
            typedef signed char     int8;
            typedef short           int16;
            typedef int             int32;
            typedef float           float32;
            typedef double          double32;

            // Unsigned...
            typedef unsigned char   uint8;
            typedef unsigned short  uint16;
            typedef unsigned int    uint32;

            // Boolean...
            typedef unsigned char   boolean;

        // Functions...

            // Get the current time in milliseconds...
            #define __Agni_GetMilliSeconds()    \
                times(NULL) * 1000 / sysconf(_SC_CLK_TCK)

    // 32-bit little-endian x86 machine running Winblows...
    #elif defined(__MINGW32__) || defined(_WIN32) || defined(_MSC_VER)

        // Includes...
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>

        // Target platform identification...
        #define AGNI_HOST_TARGET "Win32"

        // Primitives...

            // Signed...
            typedef signed char     int8;
            typedef short           int16;
            typedef int             int32;
            typedef float           float32;
            typedef double          double32;

            // Unsigned...
            typedef unsigned char   uint8;
            typedef unsigned short  uint16;
            typedef unsigned int    uint32;

            // Boolean...
            typedef unsigned char   boolean;

        // Functions...

            // Get the current time in clock ticks...
            #define __Agni_GetMilliSeconds() \
                GetTickCount()

    // 32-bit big-endian SPARC machine running Sun OS...
    #elif defined(sparc) && defined(sun) && defined(__svr4__)

        // Target platform identification...
        #define AGNI_HOST_TARGET "Sun OS";

        // Primitives...

            // Signed...
            typedef signed char     int8;
            typedef short           int16;
            typedef int             int32;
            typedef float           float32;
            typedef double          double32;

            // Unsigned...
            typedef unsigned char   uint8;
            typedef unsigned short  uint16;
            typedef unsigned int    uint32;

            // Boolean...
            typedef unsigned char   boolean;

        // Functions...

            // Get the current time in clock ticks...
            #define __Agni_GetMilliSeconds()    \
                times(NULL) * 1000 / sysconf(_SC_CLK_TCK)

    // 32-bit Mac...
    #elif defined(__APPLE__)

        /* Notes: http://developer.apple.com/documentation/
                         mac/runtimehtml/RTArch-2.html */

        // Target platform identification...
        #define AGNI_HOST_TARGET "PowerPC";

        // Primitives...

            // Signed...
            typedef signed char     int8;
            typedef short           int16;
            typedef int             int32;
            typedef float           float32;
            typedef double          double32;

            // Unsigned...
            typedef unsigned char   uint8;
            typedef unsigned short  uint16;
            typedef unsigned int    uint32;

            // Boolean...
            typedef unsigned char   boolean;

        // Functions...

            // Get the current time in clock ticks...
            #define __Agni_GetMilliSeconds()    \
                times(NULL) * 1000 / sysconf(_SC_CLK_TCK)

    // Unknown target platform, bail out...
    #else
        #error Unknown machine type. Check AgniPlatformSpecific.h and gcc -dumpmachine...
    #endif

#endif
