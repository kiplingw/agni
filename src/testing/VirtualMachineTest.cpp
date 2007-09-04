/*
  Name:         MachineTest.cpp
  Author:       Kip Warner
  Description:  Code to implement AgniDriver which tests the entire Agni
                system. Link against libAgni.a with -lAgni added to your
                GCC linker options or put in LD_LIBRARY_PATH...
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
void PrintString(Agni::VirtualMachine::Script hScript)
{
    // Variables...
    char   *pszString   = NULL;
    int     nRepeat     = 0;

    // Fetch parameters...
    pszString   = Machine.GetParameterAsString(hScript, 0);
    nRepeat     = Machine.GetParameterAsInteger(hScript, 1);

    // Print the string the requested number of times...
    while(nRepeat-- > 0)
        cout << "Thread " << hScript << ": \"" << pszString << "\"" << endl;

    // Cleanup stack...
    Machine.ReturnVoidFromHost(hScript, 2);
}

// Entry point...
int main(int nArguments, char *ppszArguments[])
{
    // Variables...
    Agni::VirtualMachine::Script    hScript = 0;
    const char     *pszScriptPath = "Random.age";   

    // Greet user...
    cout << "] VirtualMachine initialized..." << endl;

    // Load a script...
    cout << "] Loading \"" << pszScriptPath << "\"" << endl << "] ";
    switch(Machine.LoadScript(pszScriptPath, hScript))
    {
        // Loaded ok...
        case Agni::VirtualMachine::Ok:

            // Alert user...
            cout << "Load ok..." << endl;
            break;

        // Cannot open...
        case Agni::VirtualMachine::Cannot_Open:

            // Alert user...
            cout << "Error: Cannot open..." << endl;
            return 1;

        // Cannot assemble...
        case Agni::VirtualMachine::Cannot_Assemble:

            // Alert user...
            cout << "Error: Cannot assemble..." << endl;
            return 1;

        // Cannot compile...
        case Agni::VirtualMachine::Cannot_Compile:

            // Alert user...
            cout << "Error: Cannot compile..." << endl;
            return 1;

        // Memory allocation problem...
        case Agni::VirtualMachine::Memory_Allocation:

            // Alert user...
            cout << "Error: Memory allocation problem..." << endl;
            return 1;

        // Threads exhausted...
        case Agni::VirtualMachine::Threads_Exhausted:

            // Alert user...
            cout << "Error: Too many concurrent threads..." << endl;
            return 1;

        // Bad executable...
        case Agni::VirtualMachine::Bad_Executable:

            // Alert user...
            cout << "Error: Bad executable..." << endl;
            return 1;

        // Bad checksum...
        case Agni::VirtualMachine::Bad_CheckSum:

            // Alert user...
            cout << "Error: Bad checksum..." << endl;
            return 1;

        // Old runtime...
        case Agni::VirtualMachine::Old_Agni_Runtime:

            // Alert user...
            cout << "Error: Old Agni runtime..." << endl;
            return 1;

        // Old host...
        case Agni::VirtualMachine::Old_Host_Runtime:

            // Alert user...
            cout << "Error: Old host runtime..." << endl;
            return 1;

        // Wrong host...
        case Agni::VirtualMachine::Wrong_Host:

            // Alert user...
            cout << "Error: Wrong host..." << endl;
            return 1;

        // Unknown error...
        default:
        
            // Alert user...
            cout << "Error: Unknown..." << endl;
            return 1;            
    }

    // Register host provided function and check for error...
    cout << "] Registering host provided function...";
    if(!Machine.RegisterHostProvidedFunction(hScript, "PrintString", 
                                             PrintString))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

        // Done...
        cout << "ok" << endl;

    // Start the script and check for error...
    cout << "] Starting script...";
    if(!Machine.StartScript(hScript))
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
    cout << "] Calling TestStuff() script side function asynchronously..." 
         << endl;
    if(!Machine.CallFunction(hScript, "TestStuff"))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

        // Done...
        cout << "ok" << endl;

    // Fetch return value...
    cout << "] TestStuff returned the value " 
         << Machine.GetReturnValueAsFloat(hScript) << endl;

    // Invoke a function synchronously...
    cout << "] Invoking PrintLoop() synchronously... (press a key to stop)" 
         << endl;
    if(!Machine.CallFunctionSynchronously(hScript, "PrintLoop"))
    {
        // Alert and abort...
        cout << "failed" << endl;
        return 1;
    }

    for(int nIteration = 0; nIteration < 50; ++nIteration)
        Machine.RunScripts(50);*/

    // Done...
    cout << "] All done..." << endl;
    return 0;
}

