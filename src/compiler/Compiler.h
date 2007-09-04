/*
  Name:         Compiler.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Routines to produce an Agni assembly listing from a script...
*/

// Multiple include protection...
#ifndef _COMPILER_H_
#define _COMPILER_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"

        // Common structures...
        #include "../include/AgniCommonDefinitions.h"

        // Standard argument routines...
        #include <stdarg.h>

        // STL stuff...
        #include <string>
        #include <vector>
        #include <list>
        
        // I/O streams...
        #include <iostream>
        
        // Loader class...
        #include "CLoader.h"

        // Pre-processor class...
        #include "CPreProcessor.h"

        // Lexer class...
        #include "CLexer.h"
        
        // Parser class...
        #include "CParser.h"

// Within the Agni namespace...
namespace Agni
{
    // Compiler class definition...
    class Compiler
    {
        // Public stuff...
        public:

            // Compiler parameters...
            class Parameters
            {
                // Public stuff...
                public:
                
                    // Default constructor...
                    Parameters();
                    
                    // Accessors...
                        
                        // Get the process name...
                        std::string const  &GetProcessName() const;
                        
                        // Get the input file name...
                        std::string const  &GetInputFile() const;
                        
                        // Get the optimization level...
                        uint8 const         GetOptimizationLevel() const;
                        
                        // Get the output file name...
                        std::string const  &GetOutputFile() const;
                        
                        // Print out the help...
                        void                PrintHelp() const;
                        
                        // Print out the version...
                        void                PrintVersion() const;

                        // Should we be verbose?
                        bool                ShouldBeVerbose() const;
                        
                        // Should we dump token stream to stdout?
                        bool                ShouldDumpTokenStream() const;
                        
                        // Should we pre-process only?
                        bool                ShouldPreProcessOnly() const;
                        
                        // Should we stop after compilation and not assemble?
                        bool                ShouldStopAfterCompile() const;

                    // Mutators...
                    
                        // Initialize from the command line automatically...
                        bool ParseCommandLine(int const nArguments,
                                              char * const ppszArguments[]);

                // Protected stuff...
                protected:
                
                    // Attributes...

                        // Process name...
                        std::string sProcessName;

                        // Input file name...
                        std::string sInputFile;

                        // Optimization level...
                        uint8       OptimizationLevel;

                        // Dump token stream to stdout...
                        bool        bDumpTokenStream;

                        // Output file name...
                        std::string sOutputFile;

                        // Preprocess only...
                        bool        bPreProcessOnly;

                        // Stop after compilation and do not assemble...
                        bool        bStop;

                        // Verbosity...
                        bool        bVerbose;
            };

            // Methods...

                // Constructor...
                Compiler(Parameters &_UserParameters);

                // Compile script...
                bool Compile();

                // Deconstructor...
               ~Compiler();

        // Private stuff...
        private:

            // Methods...

                // Be verbose, if appropriate...
                void Verbose(std::string const sMessage) const;

            // Variables...

                // Compiler interface parameters...
                Parameters                  UserParameters;
                
                // Source code listing...
                std::vector<std::string>    SourceCode;
    };
}

#endif

