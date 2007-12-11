/*
  Name:         CMachineTarget_Base.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Abstract base class for a machine target backend. Receives
                i-code from the parser with the task of translating for given
                target machine assembly output...
*/

// Includes...
#include "CMachineTarget_Base.h"
#include <cassert>
#include <cstring>

// Using the Agni namepace...
using namespace Agni;

// Default constructor...
CMachineTarget_Base::CMachineTarget_Base(
    Agni::CParser const &InputParser, std::string const &_sOutputAssemblyListing)
    : Parser(InputParser),
      sOutputAssemblyListing(_sOutputAssemblyListing)
{    

}

// Get the output file name appropriate for this backend...
std::string const CMachineTarget_Base::GetOutputFile() const
{
    // Generate full extension..
    std::string const sFullExtension = 
        std::string(".") + GetListingFileExtension();

    // Check to make sure contains proper file extension...
    if(sOutputAssemblyListing.rfind(sFullExtension.c_str(), 
        sOutputAssemblyListing.length() - 1, sFullExtension.length()) 
        == std::string::npos)
        return sOutputAssemblyListing + sFullExtension;

    // Already ok...
    else
        return sOutputAssemblyListing;
}

// Deconstructor...
CMachineTarget_Base::~CMachineTarget_Base()
{

}

