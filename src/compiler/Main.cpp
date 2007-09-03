/*
  Name:         Main.cpp
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Entry point for command line interface to AgniCompiler...
*/

// Includes...

    // Command line option parsing...
    #include <getopt.h>

    // CAgniCompiler definitiion...
    #include "CAgniCompiler.h"

// Prototypes...

    // Generate AgniCompiler parameters from the command line...
    void GenerateAgniCompilerParameters(int nArguments, 
                    char *const ppszArguments[],    
                    CAgniCompiler::CompilerParameters &RequestedParameters);

    // Print usage...
    void PrintHelp();

    // Print version...
    void PrintVersion();

// Generate AgniCompiler parameters from the command line...
void GenerateAgniCompilerParameters(int nArguments, char *const ppszArguments[],
                    CAgniCompiler::CompilerParameters &RequestedParameters)
{
    // Variables...
    char    cOption                 = 0;
    int     nOption                 = 0;
    char   *pszStart                = NULL;
    char   *pszEnd                  = NULL;

    // Initialize to defaults...
    RequestedParameters.ucOptimization      = 0;
    RequestedParameters.bDumpTokenStream    = false;
    RequestedParameters.bPreProcessOnly     = false;
    RequestedParameters.bStop               = false;
    RequestedParameters.bVerbose            = false;

    // Extract AgniCompiler executable name...

        // Find start...

            // Look for end of directories...
            if((pszStart = strrchr(ppszArguments[0], '\\')))
                pszStart++;

            // Nope, try other slash...
            else if((pszStart = strrchr(ppszArguments[0], '/')))
                pszStart++;

            // Nope, assume argument begins with executable name...
            else
                pszStart = ppszArguments[0];

        // Find end...
        pszEnd = strchr(pszStart, '.');

            // Nope, use end of string...
            if(!pszEnd)
                pszEnd = strchr(pszStart, '\x0');

        // Extract...
        RequestedParameters.sCompilerName.assign(pszStart, 0, pszEnd - pszStart);

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
                // No parameters were passed, display help...
                if(nOption == 1)
                    PrintHelp();

                // Done parsing...
                break;
            }

        // Process option...
        switch(cOption)
        {
            // Compile...
            case 'c': RequestedParameters.sInputFile = optarg; break;

            // Help...
            case 'h': PrintHelp(); exit(0); break;

            // Optimization...
            case 'O': RequestedParameters.ucOptimization = atoi(optarg); break;

            // Output...
            case 'o': RequestedParameters.sOutputFile = optarg; break;

            // Dump token stream to stdout...
            case 'k': RequestedParameters.bDumpTokenStream = true; break;

            // Preprocess...
            case 'P': RequestedParameters.bPreProcessOnly = true; break;

            // Stop...
            case 'S': RequestedParameters.bStop = true; break;

            // Verbose...
            case 'V': RequestedParameters.bVerbose = true; break;

            // Version...
            case 'v': PrintVersion(); exit(0); break;

            // Missing parameter or unrecognized switch...
            case '?':
            default:

                // Unknown switch, alert...
                cout << RequestedParameters.sCompilerName << ": \"" << optopt 
                     << "\" unrecognized option" << endl;

                // Exit...
                exit(1);
                break;
        }
    }

    // Too many options...
    if(optind < nArguments)
    {
        // List unknown parameters...
        while(optind < nArguments)
        {
            // Display...
            cout << RequestedParameters.sCompilerName << ": option \"" 
                 << ppszArguments[optind++] << "\" unknown" << endl;
        }
    }
}

// Print usage...
void PrintHelp()
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

// Print version...
void PrintVersion()
{
    // Version...
    cout << "AgniCompiler " << AGNI_VERSION_MAJOR << "." << AGNI_VERSION_MINOR 
         << "svn" << AGNI_VERSION_SVN << endl << endl
         << " Compiled With:\t" << __VERSION__ << endl
         << " Date:\t\t" << __DATE__ << " at " << __TIME__ << "." << endl
         << " Platform:\t" << AGNI_HOST_TARGET << endl
         << " Byte Ordering:\t" << (AGNI_HOST_BIG_ENDIAN 
                ? "big-endian" : "little-endian") << endl << endl;
}

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Variables...
    CAgniCompiler::CompilerParameters       RequestedParameters;

    // Generate AgniCompiler parameters from the command line...
    GenerateAgniCompilerParameters(nArguments, ppszArguments, 
                                   RequestedParameters);

    // Create an AgniCompiler object...
    CAgniCompiler AgniCompiler(RequestedParameters);

    // Compile...
    AgniCompiler.Compile();

    // Done...
    return 0;
}
