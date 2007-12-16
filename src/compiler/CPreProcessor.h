/*
  Name:         CPreProcessor.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to take a source code linked list and convert it into 
                compiler ready source code...
*/

// Multiple include protection...
#ifndef _CPREPROCESSOR_H_
#define _CPREPROCESSOR_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"
        
        // CLoader class...
        #include "CLoader.h"
        
        // CLexer class...
        #include "CLexer.h"
        
        // Standard libraries and templates...
        #include <string>
        #include <list>
        #include <vector>
        #include <utility>

// Within the Agni namespace...
namespace Agni
{
    // CPreProcessor class definition...
    class CPreProcessor
    {
        // Public stuff...
        public:

            // Methods...
            
                // Default constructor...
                CPreProcessor(std::string const sInputRootPath, 
                              std::list<std::string> const InputSourceCodeLinkedList);

                // Get host name...
                std::string const &GetHostName() const
                    { return sHostName; }
                
                // Get host major / minor version...
                std::pair<int, int> const &GetHostVersion() const
                    { return HostVersion; }
               
                // Get stack size...
                int const &GetStackSize() const
                    { return nStackSize; }
               
                // Get thread priority...
                std::string const &GetThreadPriority() const
                    { return sThreadPriority; }

                // Process source code linked list and produce final vector form...
                std::vector<std::string> Process();

        // Protected stuff...
        protected:

            // Variables...
            
                // Root path of input...
                std::string const       sRootPath;
                
                // Source code linked list...
                std::list<std::string>  SourceCodeLinkedList;
                
                // Host name and version...
                std::string             sHostName;
                std::pair<int, int>     HostVersion;
                
                // Stack size...
                int                     nStackSize;
                
                // Thread priority...
                std::string             sThreadPriority;
    };
}

#endif

