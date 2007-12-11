/*
  Name:         CAgniMachineTarget.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  The AVM target machine backend receives i-code from the parser
                and translates to assembly listing for the Agni assembler...
*/

// Includes...
#include "CAgniMachineTarget.h"
#include <cassert>
#include <cstring>

// Using the Agni namepace...
using namespace Agni;

// Default constructor...
CAgniMachineTarget::CAgniMachineTarget(
    Agni::CParser const &InputParser, std::string const &_sOutputAssemblyListing)
    : CMachineTarget_Base(InputParser, _sOutputAssemblyListing)
{

}

// Get the output listing file extension for this target...
std::string const CAgniMachineTarget::GetListingFileExtension() const
{
    // Return it...
    return std::string("agl");
}

// Emit assembly listing for target or throw ...
void CAgniMachineTarget::EmitAssemblyListing()
{

}

