/*
  Name:         Main.cpp
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Entry point for command line interface to AgniCompiler...
*/

// Includes...

    // Command line option parsing...
    #include <getopt.h>

    // CAgniCompiler definitiion...
    #include "Compiler.h"

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Compiler parameters...
    Agni::Compiler::Parameters CompilerParameters;
    
    // Parse command line and exit if recommended...
    if(!CompilerParameters.ParseCommandLine(nArguments, ppszArguments))
        return 0;
    
    // Initialize compiler...
    Agni::Compiler Compiler(CompilerParameters);

    // Assemble...
    Compiler.Compile();

    // Done...
    return 0;
}
