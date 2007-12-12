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
#include <fstream>
#include <ctime>

// Using the Agni namepace...
using namespace Agni;

// Default constructor...
CAgniMachineTarget::CAgniMachineTarget(
    Agni::CParser const &InputParser, std::string const &_sOutputAssemblyListing)
    : CMachineTarget_Base(InputParser, _sOutputAssemblyListing),
      TabStopWidth(4)
{

}

// Get the output listing file extension for this target...
std::string const CAgniMachineTarget::GetListingFileExtension() const
{
    // Return it...
    return std::string("agl");
}

// Emit assembly listing for target or throw string...
void CAgniMachineTarget::EmitAssemblyListing() throw(std::string const)
{
    // Objects, variables, and structures...
    std::ofstream           Output;
    time_t                  RawTime     = 0;
    struct tm              *pTimeInfo   = NULL;
    Agni_MainHeader const  &MainHeader  = Parser.GetMainHeader();
    
    // Try to write out listing...
    try
    {
        // Open the file...
        Output.open(GetOutputFile().c_str(), std::ios::out);

            // Failed...
            if(!Output.is_open())
                throw std::string("access denied to write assembly listing");

        // Write header...
        
            // File name...
            Output << "; Name: " << GetOutputFile() << std::endl;
            
            // The Agni version...
            Output << "; Agni Version: " << AGNI_VERSION_MAJOR << "." 
                   << AGNI_VERSION_MINOR << std::endl;

            // Compile time...
                
                // Get raw time and convert to local time...
                time(&RawTime);
                pTimeInfo = localtime(&RawTime);

                // Output...
                Output << "; Compiled: " << asctime(pTimeInfo) << std::endl;

        // Write directives...
        Output << "; Preprocessor directives..." << std::endl << std::endl;
                
                // Stack size...
                Output << "\t; Stack size..." << std::endl
                       << "\tSetStackSize " << MainHeader.unStackSize
                       << std::endl << std::endl;

                // Thread priority...
                Output << "\t; Thread priority..." << std::endl
                       << "\tSetThreadPriority Low" << std::endl << std::endl;

        // Write global variables...
        Output << "; Global variables..." << std::endl;
    }

        // Failed to write listing...
        catch(std::string const sReason)
        {
            // Format...
            std::string const sBetterReason = 
                std::string(": error: ") + sReason;

            // Pass up the error chain...
            throw sBetterReason;
        }
}

