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
        #include <sstream>
        #include <list>
        #include <vector>

// Using the standard namespace...
using namespace std;

// CPreProcessor class definition...
class CPreProcessor
{
    // Public stuff...
    public:

        // Methods...
        
            // Default constructor...
            CPreProcessor(const string sInputRootPath, 
                          const list<string> InputSourceCodeLinkedList);
            
            // Process source code linked list and produce final vector form...
            vector<string> Process();

    // Protected stuff...
    protected:

        // Variables...
        
            // Root path of input...
            const string        sRootPath;
            
            // Source code linked list...
            list<string>        SourceCodeLinkedList;
};

#endif
