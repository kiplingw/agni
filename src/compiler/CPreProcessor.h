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
                
                // Process source code linked list and produce final vector form...
                std::vector<std::string> Process();

        // Protected stuff...
        protected:

            // Variables...
            
                // Root path of input...
                std::string const       sRootPath;
                
                // Source code linked list...
                std::list<std::string>  SourceCodeLinkedList;
    };
}

#endif

