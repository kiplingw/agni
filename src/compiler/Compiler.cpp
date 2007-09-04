/*
  Name:         Compiler.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Routines to produce an Agni assembly listing from a script...
*/

// Includes...
#include "Compiler.h"

// Using the Agni namespace...
using namespace Agni;

// Constructor...
Compiler::Compiler(Parameters &_UserParameters)
    : UserParameters(_UserParameters)
{

}

// Compile script...
bool Compiler::Compile()
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
        if(Parameters.ShouldDumpTokenStream())
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
void Compiler::Verbose(const string sMessage) const
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
Compiler::~Compiler()
{

}

// Parameters constructor...
Compiler::Parameters::Parameters()
    : OptimizationLevel(0),
      bDumpTokenStream(false),
      bPreProcessOnly(false),
      bStop(false),
      bVerbose(false)
{

}

// Get the process name...
std::string const &Compiler::Parameters::GetProcessName() const
{
    // Return it...
    return sProcessName;
}
                        
// Get the input file name...
std::string const &Compiler::Parameters::GetInputFile() const
{
    // Return it...
    return sInputFile;
}

// Get the optimization level...
uint8 const Compiler::Parameters::GetOptimizationLevel() const
{
    // Return it...
    return OptimizationLevel;
}

// Get the output file name...
std::string const &Compiler::Parameters::GetOutputFile() const
{
    // Return it...
    return sOutputFile;
}

// Initialize from the command line automatically...
bool Compiler::Parameters::ParseCommandLine(int const nArguments,
                                            char * const ppszArguments[])
{
    // Variables...
    char    cOption                 = 0;
    int     nOption                 = 0;

    // Extract compiler executable name...
    sProcessName = ppszArguments[0];
    
        // Remove path...
        if(sProcessName.find_last_of("\\/") != std::string::npos)
            sProcessName.erase(0, sProcessName.find_last_of("\\/") + 1);

    // Parse command line until done...
    for(nOption = 1; true; nOption++)
    {
        // Unused option index...
        int nOptionIndex = 0;

        // Declare valid command line options...
        static struct option LongOptions[] =
        {
            // Help: No parameters...
            {"help", no_argument, NULL, 'h'},

            // Compile: File name as mandatory parameter...
            {"compile", required_argument, NULL, 'c'},

            // Optimization: Takes one mandatory parameter...
            {"optimization", required_argument, NULL, 'O'},

            // Output: File name as parameter...
            {"output", required_argument, NULL, 'o'},

            // Dump token stream to stdout: No parameters...
            {"dump-token-stream", no_argument, NULL, 'k'},

            // Preprocess: Stop after preprocessing and print...
            {"preprocess", no_argument, NULL, 'P'},

            // Stop: Stop after compilation and do not assemble...
            {"stop", no_argument, NULL, 'S'},

            // Verbose: No parameters...
            {"verbose", no_argument, NULL, 'V'},

            // Version: No parameters...
            {"version", no_argument, NULL, 'v'},

            // End of parameter list...
            {0, 0, 0, 0}
        };

        // Prevent getopt_long from printing to stderr...
        opterr = 0;

        // Grab an option...
        cOption = getopt_long(nArguments, ppszArguments, "hc:O:o:PSVv",
                              LongOptions, &nOptionIndex);

            // End of option list...
            if(cOption== -1)
            {
                // No parameters were passed...
                if(nOption == 1)
                {
                    // Display help and recommend no more processing...
                    PrintHelp();
                    return false;
                }

                // Done parsing...
                break;
            }

        // Process option...
        switch(cOption)
        {
            // Compile...
            case 'c': 
                
                // Store the input file...
                sInputFile = optarg; 
                break;

            // Help...
            case 'h': 
            
                // Display help and recommend no more processing...
                PrintHelp();
                return false;

            // Optimization...
            case 'O': 
            
                // Store optimization level...
                OptimizationLevel = atoi(optarg); 
                break;

            // Output...
            case 'o':
                
                // Extension length...
                nTemp = strlen("." AGNI_FILE_EXTENSION_LISTING);

                // Store output file...
                sOutputFile = optarg;

                // Check to make sure contains proper file extension...
                if(sOutputFile.rfind("." AGNI_FILE_EXTENSION_LISTING, 
                                     sOutputFile.length() - 1, nTemp) ==
                   std::string::npos)
                {
                    // Append it then...
                    sOutputFile += "." AGNI_FILE_EXTENSION_LISTING;
                }

                // Done...
                break;

            // Dump token stream to stdout...
            case 'k': 
            
                // Enable flag...
                bDumpTokenStream = true;
                break;

            // Pre-process only...
            case 'P': 
            
                // Enable flag...
                bPreProcessOnly = true;
                break;

            // Stop after compilation...
            case 'S': 
            
                // Enable flag...
                bStop = true; 
                break;

            // Verbose...
            case 'V': 
            
                // Enable flag...
                bVerbose = true;
                break;

            // Version...
            case 'v': 
            
                // Print the version and then recommend no more processing...
                PrintVersion();
                return false;

            // Missing parameter or unrecognized switch...
            case '?':
            default:

                // Unknown switch, alert...
                cout << sProcessName << ": \"" << optopt 
                     << "\" unrecognized option" << endl;

                // Recommend no more parsing...
                return false;
        }
    }

    // Too many options...
    if(optind < nArguments)
    {
        // List unknown parameters...
        while(optind < nArguments)
        {
            // Display...
            cout << sProcessName << ": option \"" 
                 << ppszArguments[optind++] << "\" unknown" << endl;
        }
        
        // No more parsing should be done...
        return false;
    }
    
    // Recommend ok to continue with compilation...
    return true;
}

// Print out the help...
void Compiler::Parameters::PrintHelp() const
{
    // Display help...
    cout << "Usage: ac [option(s)] [input-file] [output-file]\n"
            "Purpose: Compile Agni script, assemble into Agni executable...\n\n"
            " Options:\n"
            "  -c --compile=<infile>        Input file\n"
            "  -h --help                    Print this help message\n"
            "  -k --dump-token-stream       Dump the token stream to stdout\n"
            "  -O --optimization=<level>    Optimization level\n"
            "  -o --output=<outfile>        Name output file\n"
            "  -P --preprocess              Print preprocessed form only\n"
            "  -S --stop                    Do not assemble after compilation\n"
            "  -V --verbose                 Be verbose\n"
            "  -v --version                 Print version information\n\n"
            "  INFILE can be \"stdin\" or a file name. Default is \"stdin\".\n"
            "  LEVEL can be an integer value from 0 (disabled) to 1.\n"
            "  OUTFILE can be \"stdout\" or a file name. Default is \"stdout\".\n\n"

            " Examples:\n"
            "  ac -c MyScript.ags -o MyGeneratedExecutable\n\n"

            " AgniCompiler comes with NO WARRANTY, to the extent permitted by\n"
            " law. You may redistribute copies of AgniCompiler. Just use your\n"
            " head.\n\n"

            " Shouts and thanks to Alex Varanese, MasterCAD, Curt, Dr. Knorr,\n"
            " Dr. Wolfman, Dr. Eiselt, Peter (TDLSoftware.org), Reed, RP,\n"
            " Sarah, Wayne, and the MinGW, GCC, Dev-C++, Code::Blocks, Gnome,\n"
            " Winamp, and XMMS crews.\n\n"

            " Written by Kip Warner. Questions or comments may be sent to\n"
            " Kip@TheVertigo.com. You can visit me out on the wasteland at\n"
            " http://TheVertigo.com.\n\n";
}

// Print out the version...
void Compiler::Parameters::PrintVersion() const
{
    // Version...
    std::cout << "AgniCompiler "    << AGNI_VERSION_MAJOR << "."
                                    << AGNI_VERSION_MINOR << "svn"
                                    << AGNI_VERSION_SVN << std::endl 
                                    << std::endl
              << "Compiler:\t"      << __VERSION__ << std::endl
              << "Date:\t\t"        << __DATE__ << " at " << __TIME__ 
                                    << std::endl
              << "Platform:\t"      << HOST_TARGET << std::endl
              << "Little Endian:\t" << Agni::bLittleEndian << std::endl;
}

// Should we be verbose?
bool Compiler::Parameters::ShouldBeVerbose() const
{
    // Return flag...
    return bVerbose;
}

// Should we dump token stream to stdout?
bool Compiler::Parameters::ShouldDumpTokenStream() const
{
    // Return flag...
    return bDumpTokenStream;
}

// Should we pre-process only?
bool Compiler::Parameters::ShouldPreProcessOnly() const
{
    // Return flag...
    return bPreProcessOnly;
}

// Should we stop after compilation and not assemble?
bool Compiler::Parameters::ShouldStopAfterCompile() const
{
    // Return flag...
    return bStop;
}

