/*
  Name:         MachineTest.cpp
  Author:       Kip Warner
  Description:  Code to implement AgniDriver which tests the entire Agni
                system. Link against libAgni.a with -lAgni added to your
                GCC linker options...
*/

// Includes...
#include "../include/CAgni.h"
#include <iostream>
#include <string>

// Using the standard namespace...
using namespace std;

// Agni virtual machine instance...
Agni::VirtualMachine    Machine("AgniDriver", 1, 1);

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
        cout << "Thread " << hScript ": \"" pszString "\"" << endl;

    // Cleanup stack...
    Machine.ReturnVoidFromHost(hScript, 2);
}

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Variables...
    Agni::VirtualMachine::Script    hScript = 0;
    const char     *pszScriptPath =
                    "src/testing/scripts/Random.age";   

    // Greet user...
    printf("] AgniVirtualMachine initialized...\n");

    // Load a script...
    printf("] Loading \"%s\"\n] ", pszScriptPath);
    switch(Machine.LoadScript(pszScriptPath, hScript))
    {
        // Loaded ok...
        case Agni::VirtualMachine::STATUS_OK:

            // Alert user...
            cout << "Load ok..." << endl;
            break;

        // Cannot open...
        case Agni::VirtualMachine::STATUS_ERROR_CANNOT_OPEN:

            // Alert user...
            cout << "Error: Cannot open..." << endl;
            return 1;

        // Cannot assemble...
        case Agni::VirtualMachine::STATUS_ERROR_CANNOT_ASSEMBLE:

            // Alert user...
            cout << "Error: Cannot assemble..." << endl;
            return 1;

        // Cannot compile...
        case Agni::VirtualMachine::STATUS_ERROR_CANNOT_COMPILE:

            // Alert user...
            cout << "Error: Cannot compile..." << endl;
            return 1;

        // Memory allocation problem...
        case Agni::VirtualMachine::STATUS_ERROR_MEMORY_ALLOCATION:

            // Alert user...
            cout << "Error: Memory allocation problem..." << endl;
            return 1;

        // Threads exhausted...
        case Agni::VirtualMachine::STATUS_ERROR_THREADS_EXHAUSTED:

            // Alert user...
            cout << "Error: Too many concurrent threads..." << endl;
            return 1;

        // Bad executable...
        case Agni::VirtualMachine::STATUS_ERROR_BAD_EXECUTABLE:

            // Alert user...
            cout << "Error: Bad executable..." << endl;
            return 1;

        // Bad checksum...
        case Agni::VirtualMachine::STATUS_ERROR_BAD_CHECKSUM:

            // Alert user...
            cout << "Error: Bad checksum..." << endl;
            return 1;

        // Old runtime...
        case Agni::VirtualMachine::STATUS_ERROR_OLD_AGNI_RUNTIME:

            // Alert user...
            cout << "Error: Old Agni runtime..." << endl;
            return 1;

        // Old host...
        case Agni::VirtualMachine::STATUS_ERROR_OLD_HOST:

            // Alert user...
            cout << "Error: Old host..." << endl;
            return 1;

        // Wrong host...
        case Agni::VirtualMachine::STATUS_ERROR_WRONG_HOST:

            // Alert user...
            cout << "Error: Wrong host..." << endl;
            return 1;
    }

    // Register host provided function and check for error...
    printf("] Registering host provided function..." << endl;
    if(!Machine.RegisterHostProvidedFunction(hScript, "PrintString", PrintString))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

        // Done...
        cout << "ok" << endl;

    // Start the script and check for error...
    printf("] Starting script..." << endl;
    if(!Agni.StartScript(hScript))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

        // Done...
        cout << "ok" << endl;

    // Call PrintRandomNumbers asynchronously...
    cout << "] Calling PrintRandomNumbers..." << endl;
    Machine.CallFunction(hScript, "PrintRandomNumbers");

    /* Call TestStuff script function asynchronously...
    printf("] Calling TestStuff() script side function asynchronously...\n");
    if(!Agni.CallFunction(hScript, "TestStuff"))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

        // Done...
        cout << "ok" << endl;

    // Fetch return value...
    printf("] TestStuff returned the value %f\n",
           Agni.GetReturnValueAsFloat(hScript));

    // Invoke a function synchronously...
    printf("] Invoking PrintLoop() synchronously... (press a key to stop)\n");
    if(!Agni.CallFunctionSynchronously(hScript, "PrintLoop"))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

    while(1)
        Agni.RunScripts(50);*/

    // Done...
    cout << "] All done..." << endl;
    return 0;
}
