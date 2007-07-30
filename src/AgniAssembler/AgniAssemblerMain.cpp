/*
  Name:         AgniAssemblerMain.cpp
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Entry point for command line interface to AgniAssembler...
*/

// Includes...

    // Command line option parsing...
    #include <getopt.h>

    // CAgniAssembler definitiion...
    #include "CAgniAssembler.h"

// Prototypes...

    // Generate AgniAssembler parameters from the command line...
    void GenerateParametersFromCommandLine(int nArguments,
                                           char *const ppszArguments[],
                                           CAgniAssembler::AssemblerParameters *pParameters);

    // Print usage...
    void PrintHelp();

    // Print version...
    void PrintVersion();

// Parse command line and store options...
void GenerateParametersFromCommandLine(int nArguments,
                                       char *const ppszArguments[],
                                       CAgniAssembler::AssemblerParameters *pParameters)
{
    // Variables...
    char    cOption                 = 0;
    int     nOption                 = 0;
    char   *pszStart                = NULL;
    char   *pszEnd                  = NULL;

    // Clear settings structure...
    memset(pParameters, 0, sizeof(CAgniAssembler::AssemblerParameters));

    // Extract AgniAssembler executable name...

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
        strncpy(pParameters->szAssemblerName, pszStart, pszEnd - pszStart);
        pParameters->szAssemblerName[pszEnd - pszStart] = '\x0';

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

            // Assemble: File name as mandatory parameter...
            {"assemble", required_argument, NULL, 'a'},

            // Optimization: Takes one mandatory parameter...
            {"optimization", required_argument, NULL, 'O'},

            // Output: File name as parameter...
            {"output", required_argument, NULL, 'o'},

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
        cOption = getopt_long(nArguments, ppszArguments, "ha:O:o:Vv",
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
            // Assemble...
            case 'a':

                // Remember input file name...
                strncpy(pParameters->szInputFile, optarg,
                        sizeof(pParameters->szInputFile) - 1);

                // Done...
                break;

            // Help...
            case 'h':

                // Display help...
                PrintHelp();

                // Exit...
                exit(0);
                break;

            // Optimization...
            case 'O':

                // Remember optimization level...
                pParameters->ucOptimization = atoi(optarg);

                // Done...
                break;

            // Output...
            case 'o':

                // Remember...
                strncpy(pParameters->szOutputFile, optarg,
                        sizeof(pParameters->szOutputFile) - 1);

                // Done...
                break;

            // Verbose...
            case 'V':

                // Remember...
                pParameters->bVerbose = true;

                // Done...
                break;

            // Version...
            case 'v':

                // Display version...
                PrintVersion();

                // Exit...
                exit(0);
                break;

            // Missing parameter or unrecognized switch...
            case '?':
            default:

                // Unknown switch, alert...
                printf("%s: \"%c\" unrecognized option\n",
                       pParameters->szAssemblerName, optopt);

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
            printf("%s: option \"%s\" unknown\n", pParameters->szAssemblerName,
                   ppszArguments[optind++]);
        }
    }
}

// Print usage...
void PrintHelp()
{
    // Display help...
    printf("Usage: aa [option(s)] [input-file] [output-file]\n"
           "Purpose: Provides AgniVirtualMachine backend to AgniCompiler...\n\n"
           " Options:\n"
           "  -a --assemble=<infile>       Input file\n"
           "  -h --help                    Print this help message\n"
           "  -O --optimization=<level>    Optimization level\n"
           "  -o --output=<outfile>        Name output file\n"
           "  -V --verbose                 Be verbose\n"
           "  -v --version                 Print version information\n\n"
           "  INFILE can be \"stdin\" or a file name. Default is \"stdin\".\n"
           "  LEVEL can be an integer value from 0 (disabled) to 1.\n"
           "  OUTFILE can be \"stdout\" or a file name. Default is \"stdout\".\n\n"

           " Examples:\n"
           "  aa -a MyAssemblyListing.agl -o MyGeneratedExecutable\n\n"

           " AgniAssembler comes with NO WARRANTY, to the extent permitted by\n"
           " law. You may redistribute copies of AgniAssembler. Just use your\n"
           " head.\n\n"

           " Shouts and thanks to Alex Varanese, MasterCAD, Curt, Dr. Knorr,\n"
           " Dr. Wolfman, Dr. Eiselt, Peter (TDLSoftware.org), Reed, RP,\n"
           " Sarah, Wayne, and the MinGW, GCC, Dev-C++, Code::Blocks, Gnome,\n"
           " Winamp, and XMMS crews.\n\n"

           " Written by Kip Warner. Questions or comments may be sent to\n"
           " Kip@TheVertigo.com. You can visit me out on the wasteland at\n"
           " http://TheVertigo.com.\n\n");
}

// Print version...
void PrintVersion()
{
    // Version...
    printf("AgniAssembler %d.%d\n\n"
           " Compiled With:\t%s\n"
           " Date:\t\t%s at %s.\n"
           " Platform:\t%s\n"
           " Byte Ordering:\t%s\n",
           AGNI_VERSION_MAJOR, AGNI_VERSION_MINOR,
           __VERSION__,
           __DATE__, __TIME__,
           AGNI_HOST_TARGET, AGNI_HOST_BIG_ENDIAN ? "big-endian"
                                                  : "little-endian");

}

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Variables...
    CAgniAssembler::AssemblerParameters     Parameters;
    CAgniAssembler                         *pAgniAssembler  = NULL;

    // Generate AgniAssembler parameters from the command line...
    GenerateParametersFromCommandLine(nArguments, ppszArguments, &Parameters);

/*
memset(&Parameters, 0, sizeof(CAgniAssembler::AssemblerParameters));
Parameters.bVerbose = true;
strcpy(Parameters.szInputFile,
       "/home/kip/Projects/Agni/trunk/dist/examples/Scripts/Test.agl");
puts("WARNING: USING PRESET INPUT AND OUTPUT");
strcpy(Parameters.szOutputFile,
       "/home/kip/Projects/Agni/trunk/dist/examples/Scripts/Test.age");
*/
    // Create an AgniAssembler...
    pAgniAssembler = new CAgniAssembler(&Parameters);

        // Failed...
        if(!pAgniAssembler)
        {
            // Alert, abort...
            puts("Insufficient memory...");
            return 1;
        }

    // Assemble...
    pAgniAssembler->Assemble();

    // Cleanup...
    delete pAgniAssembler;

    // Done...
    return 0;
}
