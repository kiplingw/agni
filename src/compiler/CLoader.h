/*
  Name:         CLoader.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to load user script from disk or stream into memory...
*/

// Multiple include protection...
#ifndef _CLOADER_H_
#define _CLOADER_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"
        
        // String class...
        #include <string>
        
        // Linked list...
        #include <list>

// Within the Agni namespace...
namespace Agni
{
    // CLoader class definition...
    class CLoader
    {
        // Public stuff...
        public:

            // Methods...
            
                // Default constructor...
                CLoader(const string sInputFileName);
                
                // Load source code or throw a const string...
                list<string> Load() const;
        
        // Protected stuff...
        protected:
            
            // Variables...
            const string    sFileName;
    };
}

#endif

