/*
  Name:         CMachineTarget_Base.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Abstract base class for a machine target backend. Receives
                i-code from the parser with the task of translating for given
                target machine assembly output...
*/

// Multiple include protection...
#ifndef _CMACHINETARGET_BASE_H_
#define _CMACHINETARGET_BASE_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"
        #include "../include/AgniCommonDefinitions.h"

        // C++ string...
        #include <string>

        // Parser class...
        #include "CParser.h"

// Within the Agni namespace...
namespace Agni
{
    // CMachineTarget_Base class definition...
    class CMachineTarget_Base
    {
        // Public stuff...
        public:

            // Methods...

                // Default constructor...
                CMachineTarget_Base(CParser const &InputParser, 
                                    std::string const &_sOutputAssemblyListing);

                // Emit assembly listing for target or throw ...
                virtual void EmitAssemblyListing() = 0;
                
                // Deconstructor...
                virtual ~CMachineTarget_Base();

        // Protected stuff...
        protected:

            // Methods...

                // Get the output listing file extension for this target...
                virtual std::string const GetListingFileExtension() const = 0;

                // Get the output file name appropriate for this backend...
                std::string const GetOutputFile() const;

            // Attributes...
            
                // Parser reference...
                CParser const &Parser;
                
                // Path to output assembly listing...
                std::string const sOutputAssemblyListing;
    };
}

#endif

