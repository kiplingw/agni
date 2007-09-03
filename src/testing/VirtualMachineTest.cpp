/*
  Name:         main.cpp
  Author:       Kip Warner
  Description:  Code to implement AgniDriver which tests the entire Agni
                system. Link against libAgni.a with -lAgni added to your
                GCC linker options...
*/

// Includes...
#include "../include/CAgni.h"
#include <stdio.h>
#include <stdlib.h>

// Agni virtual machine instance...
CAgni   Agni("AgniDriver", 1, 1);

// Print a string from a script...
void PrintString(CAgni::Script hScript)
{
    // Variables...
    char   *pszString   = NULL;
    int     nRepeat     = 0;

    // Fetch parameters...
    pszString   = Agni.GetParameterAsString(hScript, 0);
    nRepeat     = Agni.GetParameterAsInteger(hScript, 1);

    // Print the string the requested number of times...
    while(nRepeat-- > 0)
        printf("Thread %d: \"%s\"\n", hScript, pszString);

    // Cleanup stack...
    Agni.ReturnVoidFromHost(hScript, 2);
}

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Variables...
    CAgni::Script   hScript = 0;
    const char     *pszScriptPath =
                    "/home/kip/Projects/Agni/trunk/dist/examples/Scripts/Random.age";

    // Greet user...
    printf("] AgniVirtualMachine initialized...\n");

    // Load a script...
    printf("] Loading \"%s\"\n] ", pszScriptPath);
    switch(Agni.LoadScript(pszScriptPath, hScript))
    {
        // Loaded ok...
        case CAgni::STATUS_OK:

            // Alert user...
            puts("Load ok...");
            break;

        // Cannot open...
        case CAgni::STATUS_ERROR_CANNOT_OPEN:

            // Alert user...
            puts("Error: Cannot open...");
            return 1;

        // Cannot assemble...
        case CAgni::STATUS_ERROR_CANNOT_ASSEMBLE:

            // Alert user...
            puts("Error: Cannot assemble...");
            return 1;

        // Cannot compile...
        case CAgni::STATUS_ERROR_CANNOT_COMPILE:

            // Alert user...
            puts("Error: Cannot compile...");
            return 1;

        // Memory allocation problem...
        case CAgni::STATUS_ERROR_MEMORY_ALLOCATION:

            // Alert user...
            puts("Error: Memory allocation problem...");
            return 1;

        // Threads exhausted...
        case CAgni::STATUS_ERROR_THREADS_EXHAUSTED:

            // Alert user...
            puts("Error: Too many concurrent threads...");
            return 1;

        // Bad executable...
        case CAgni::STATUS_ERROR_BAD_EXECUTABLE:

            // Alert user...
            puts("Error: Bad executable...");
            return 1;

        // Bad checksum...
        case CAgni::STATUS_ERROR_BAD_CHECKSUM:

            // Alert user...
            puts("Error: Bad checksum...");
            return 1;

        // Old runtime...
        case CAgni::STATUS_ERROR_OLD_AGNI_RUNTIME:

            // Alert user...
            puts("Error: Old Agni runtime...");
            return 1;

        // Old host...
        case CAgni::STATUS_ERROR_OLD_HOST:

            // Alert user...
            puts("Error: Old host...");
            return 1;

        // Wrong host...
        case CAgni::STATUS_ERROR_WRONG_HOST:

            // Alert user...
            puts("Error: Wrong host...");
            return 1;
    }

    // Register host provided function and check for error...
    printf("] Registering host provided function...");
    if(!Agni.RegisterHostProvidedFunction(hScript, "PrintString", PrintString))
    {
        // Alert and abort...
        puts("failed");
        return 1;
    }

        // Done...
        puts("ok");

    // Start the script and check for error...
    printf("] Starting script...");
    if(!Agni.StartScript(hScript))
    {
        // Alert and abort...
        puts("failed");
        return 1;
    }

        // Done...
        puts("ok");

    // Call PrintRandomNumbers asynchronously...
    printf("] Calling PrintRandomNumbers...\n");
    Agni.CallFunction(hScript, "PrintRandomNumbers");

    /* Call TestStuff script function asynchronously...
    printf("] Calling TestStuff() script side function asynchronously...\n");
    if(!Agni.CallFunction(hScript, "TestStuff"))
    {
        // Alert and abort...
        puts("failed");
        return 1;
    }

        // Done...
        puts("ok");

    // Fetch return value...
    printf("] TestStuff returned the value %f\n",
           Agni.GetReturnValueAsFloat(hScript));

    // Invoke a function synchronously...
    printf("] Invoking PrintLoop() synchronously... (press a key to stop)\n");
    if(!Agni.CallFunctionSynchronously(hScript, "PrintLoop"))
    {
        // Alert and abort...
        puts("failed");
        return 1;
    }

    while(1)
        Agni.RunScripts(50);*/

    // Done...
    printf("] All done...\n");
    return 0;
}
