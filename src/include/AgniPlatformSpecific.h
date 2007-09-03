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

    // Agni namespace for byte ordering and structure packing...
    namespace Agni
    {
        // Byte ordering... (borrowed from SDL)
        #if (defined(__i386__) || defined(__i386)) || \
             defined(__ia64__) || defined(WIN32) || \
            (defined(__alpha__) || defined(__alpha)) || \
             defined(__arm__) || \
            (defined(__mips__) && defined(__MIPSEL__)) || \
             defined(__SYMBIAN32__) || \
             defined(__x86_64__) || \
             defined(__LITTLE_ENDIAN__)
        bool const bLittleEndian    = true;
        #else
        bool const bLittleEndian    = false;
        #endif

        // Define cross-platform structure packing attribute...

            // Using a real compiler...
            #if defined(__GNUC__)
                #define ATTRIBUTE_PACKED __attribute__ ((__packed__))

            // Using Visual C++...
            #elif defined(_MSC_VER)
                #define ATTRIBUTE_PACKED #pragma pack(1)

            // Unknown compiler...
            #else
                #error I don't know how to set the structure packing attributes for your compiler...
            #endif
    }

    // 32-bit little-endian x86 machine running some flavour of Linux...
    #if (defined(__i686__) || defined(__i586__) || defined(__i486__) || \
            defined(__i386__)) && \
        (defined(Linux) || defined(linux) || defined(__linux))

        // Includes...
        #include <sys/times.h>
        #include <unistd.h>

        // Agni namespace...
        namespace Agni
        {
            // Target platform identification...
            #define HOST_TARGET "Linux"

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
                #define GetMilliSeconds()    \
                    ::times(NULL) * 1000 / ::sysconf(_SC_CLK_TCK)
        }

    // 32-bit little-endian x86 machine running Winblows...
    #elif defined(__MINGW32__) || defined(_WIN32) || defined(_MSC_VER)

        // Includes...
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>

        // Agni namespace...
        namespace Agni
        {
            // Target platform identification...
            #define HOST_TARGET "Win32"

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
                #define GetMilliSeconds() \
                    ::GetTickCount()
        }

    // Unknown target platform, bail out...
    #else
        #error Unknown machine type. Check AgniPlatformSpecific.h and gcc -dumpmachine...
    #endif

#endif

