/*
  Name:         CAgniMachineTarget.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  The AVM target machine backend receives i-code from the parser
                and translates to assembly listing for the Agni assembler...
*/

// Multiple include protection...
#ifndef _CAGNIMACHINETARGET_H_
#define _CAGNIMACHINETARGET_H_

// Includes...
#include "CMachineTarget_Base.h"

// Within the Agni namespace...
namespace Agni
{
    // CAgniMachineTarget class definition...
    class CAgniMachineTarget : public CMachineTarget_Base
    {
        // Public stuff...
        public:

            // Methods...

                // Default constructor...
                CAgniMachineTarget(const Agni::CParser &InputParser, 
                                   std::string const &_sOutputAssemblyListing);

                // Emit assembly listing for target or throw ...
                void EmitAssemblyListing() throw(std::string const);

        // Protected stuff...
        protected:
        
            // Constants...
            unsigned char const TabStopWidth;        
        
            // Methods...

                // Get the output listing file extension for this target...
                std::string const GetListingFileExtension() const;
    };
}

#endif
