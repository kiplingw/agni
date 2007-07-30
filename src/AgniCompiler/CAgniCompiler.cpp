/*
  Name:         CAgniCompiler.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Routines to produce an Agni assembly listing from a script...
*/

// Includes...
#include "CAgniCompiler.h"

// Constructor...
CAgniCompiler::CAgniCompiler(CompilerParameters &RequestedParameters)
{
    // Store parameters...
    Parameters = RequestedParameters;
}

// Compile script...
boolean CAgniCompiler::Compile()
{
    // Variables...
    list<string>    SourceCodeLinkedList;
    vector<string>  SourceCode;
    unsigned int    unIndex                 = 0;
    string          sRootPath;

    // Try to compile...
    try
    {
        // Verify parameters...

            // No input file specified...
            if(Parameters.sInputFile.empty())
                throw Parameters.sCompilerName + ": no input file";

            // No output file specified...
            if(Parameters.sOutputFile.empty())
                throw Parameters.sCompilerName + ": no output file specified";

            // Input and output files are the same...
            if(strcasecmp(Parameters.sInputFile.c_str(), 
                          Parameters.sOutputFile.c_str()) == 0)
                throw Parameters.sCompilerName + ": input and output are the "
                                                 "same location";

        // Create root directory file name...
        
            // UNIX style paths...
            if(string::npos != (unIndex = Parameters.sInputFile.rfind('/', 
                                            Parameters.sInputFile.length() - 1)))
                sRootPath.assign(Parameters.sInputFile, 0, unIndex + 1);
                
            // Win32 style paths...
            else if(string::npos != (unIndex = Parameters.sInputFile.rfind('\\', 
                                            Parameters.sInputFile.length() - 1)))
                sRootPath.assign(Parameters.sInputFile, 0, unIndex + 1);
                
            // No path...
            else
                sRootPath = "./";

        // Try to load from disk...

            // Be verbose...
            Verbose("loading source code...");

            // Create loader to retrieve structured source code...
            CLoader Loader(Parameters.sInputFile);
            
            // Load the source code and save it in a linked list...
            SourceCodeLinkedList = Loader.Load();

        // If requested, display token stream to stdout...
        if(Parameters.bDumpTokenStream)
        {
            // Variables...
            uint32          unLastCheckedLine   = 0;

            // Temporarily convert linked list to vector for lexer...
            vector<string> TempSourceCodeVector(SourceCodeLinkedList.begin(),
                                                SourceCodeLinkedList.end());
            
            // Initialize temp lexer...
            CLexer TempLexer(TempSourceCodeVector);

            // Dump token stream...
            while(TempLexer.GetNextToken() != CLexer::TOKEN_END_OF_STREAM)
            {
                // This is a new line of source code...
                if(TempLexer.GetCurrentHumanLineIndex() != unLastCheckedLine)
                {
                    // Display the line, token, and its associated lexeme...
                    cout << endl << TempLexer.GetCurrentHumanLineIndex() << ": " 
                         << TempLexer.GetCurrentTokenAsString()
                         << " (\"" << TempLexer.GetCurrentLexeme() << "\") ";
    
                    // Remember the last checked line...
                    unLastCheckedLine = TempLexer.GetCurrentHumanLineIndex();
                }
                
                // This is the same line...
                else
                {
                    // Display the token, along with its associated lexeme...
                    cout << TempLexer.GetCurrentTokenAsString() 
                         << " (\"" << TempLexer.GetCurrentLexeme() << "\") ";
                }
            }
            
            // Done, add a new line...
            cout << endl << endl;
        }

        // Now pre-process it and turn it into a vector...

            // Be verbose...
            Verbose("pre-processing...");

            // Create pre-processor...
            CPreProcessor PreProcessor(sRootPath, SourceCodeLinkedList);
            
            // Pre-process it and spit out compiler ready vector of source...
            SourceCode = PreProcessor.Process();

            // We were instructed to stop after preprocessing...
            if(Parameters.bPreProcessOnly)
            {
                // Be verbose...
                Verbose("pre-processing complete, dumping, then halting...");

                // Print each line... (humans count lines starting from one)
                for(unIndex = 0; unIndex < SourceCode.size(); unIndex++)
                    cout << unIndex + 1 << ":\t" << SourceCode.at(unIndex);

                // Done...
                return true;
            }

        // Parse source code to generate I-code...

            // Be verbose...
            Verbose("generating i-code...");

            // Create parser...
//            CParser Parser(SourceCode);

            // Parse source code...
    }

        // Compilation failed for some reason...
        catch(const string sReason)
        {
            // Output error message...
            cout << Parameters.sCompilerName << ": " << sReason << endl;

            // Abort...
            return false;
        }

    // Done...
    return true;
}

// Be verbose, if appropriate...
void CAgniCompiler::Verbose(const string sMessage) const
{
    // Verbose mode not enabled, ignore...
    if(!Parameters.bVerbose)
        return;

    // Output is standard output device, ingore...
    if(Parameters.sOutputFile == "stdout")
        return;

    // Output...
    cout << Parameters.sCompilerName << ": " << sMessage << endl;
}

// Deconstructor...
CAgniCompiler::~CAgniCompiler()
{

}

