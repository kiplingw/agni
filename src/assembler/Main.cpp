/*
  Name:         Main.cpp
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Entry point for command line interface to AgniAssembler...
*/

// Includes...

    // Assembler definitiion...
    #include "Assembler.h"
    
    // POSIX compliant exit status codes...
    #include <cstdlib>

// Entry point...
int main(int const nArguments, char *ppszArguments[])
{
    // Assembler parameters...
    Agni::Assembler::Parameters AssemblerParameters;
    
    // Parse command line and exit if recommended...
    if(!AssemblerParameters.ParseCommandLine(nArguments, ppszArguments))
        return 0;
    
    // Initialize assembler...
    Agni::Assembler Assembler(AssemblerParameters);

    // Assemble ok...
    if(Assembler.Assemble())
        return EXIT_SUCCESS;

    // Assemble failed...
    else
        return EXIT_FAILURE;
}
