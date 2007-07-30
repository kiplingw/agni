/*
  Name:         CAgni.h (definition)
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  AgniVirtualMachine. You need to link against libAgni.a to use
                this (add -lAgni to command line, if using gcc)...
*/

// Multiple include protection...
#if !defined(_CAGNI_H_)
#define _CAGNI_H_

// To-do...

// Includes...

    // Data types...
    #include "AgniPlatformSpecific.h"

    // File I/O...
    #include <stdio.h>

    // Memory allocation...
    #include <stdlib.h>

    // String manipulation...
    #include <string.h>

    // Exponentiation, trigonometric, and other miscellaneous math routines...
    #include <math.h>

// CAgni virtual machine class definition...
class CAgni
{
    // Public macros and data types...
    public:

        // Status codes...
        enum STATUS
        {
            STATUS_OK = 0,
            STATUS_ERROR_CANNOT_OPEN,
            STATUS_ERROR_CANNOT_ASSEMBLE,
            STATUS_ERROR_CANNOT_COMPILE,
            STATUS_ERROR_MEMORY_ALLOCATION,
            STATUS_ERROR_THREADS_EXHAUSTED,
            STATUS_ERROR_BAD_EXECUTABLE,
            STATUS_ERROR_BAD_CHECKSUM,
            STATUS_ERROR_OLD_AGNI_RUNTIME,
            STATUS_ERROR_OLD_HOST,
            STATUS_ERROR_WRONG_HOST
        };

        // Script execution exceptions...
        enum SCRIPT_EXECUTION_EXCEPTION
        {
            SCRIPT_EXECUTION_EXCEPTION_STACK_UNDERFLOW = 0,
            SCRIPT_EXECUTION_EXCEPTION_STACK_OVERFLOW
        };

        // Script handle...
        typedef uint32 Script;

        // Host provided function signature...
        typedef void (HostProvidedFunction)(Script hScript);

        // Global host function flag...
        #define GLOBAL_HOST_FUNCTION -1

    // Public API methods...
    public:

        // Constructor initializes runtime enviroment...
        CAgni(char *_pszHostName, uint8 _HostVersionMajor,
              uint8 _HostVersionMinor);

        // Script function calling...

            // Call script function asynchronously... (blocking)
            boolean CallFunction(Script hScript, char *pszName);

            // Call script function synchronously... (non-blocking)
            boolean CallFunctionSynchronously(Script hScript, char *pszName);

            // Pass float parameter...
            boolean PassFloatParameter(Script hScript, float fValue);

            // Pass integer parameter...
            boolean PassIntegerParameter(Script hScript, int nValue);

            // Pass string parameter...
            boolean PassStringParameter(Script hScript, char *pszValue);

        // Script parameter retrieval for when script invokes host function...

            // Get passed parameter as an integer...
            int     GetParameterAsInteger(Script hScript, uint8 unParameter);

            // Get passed parameter as a float...
            float   GetParameterAsFloat(Script hScript, uint8 unParameter);

            // Get passed parameter as a string...
            char   *GetParameterAsString(Script hScript, uint8 unParameter);

        // Script function return value reading...

            // Get return as a float from an asynchronous call...
            float   GetReturnValueAsFloat(Script hScript);

            // Get return as an integer from an asynchronous call...
            int     GetReturnValueAsInteger(Script hScript);

            // Get return as a string from an asynchronous call...
            char   *GetReturnValueAsString(Script hScript, char *pszBuffer,
                                           uint32 unBufferSize);

        // Script general loading, unloading, and querying...

            // Load script, store handle, return a status code...
            STATUS LoadScript(const char *pszPath, Script &hScript);

            // Unload script...
            boolean UnloadScript(Script &hScript);

        // Script playback...

            // Pause a script for a certain duration...
            boolean PauseScript(Script hScript, uint32 unDuration);

            // Reset script...
            boolean ResetScript(Script hScript);

            // Run scripts for specified milliseconds, or use macros...
            boolean RunScripts(uint32 unDuration);

            // Start the execution of a script...
            boolean StartScript(Script hScript);

            // Stop the execution of a script...
            boolean StopScript(Script hScript);

            // Unpause a script...
            boolean UnPauseScript(Script hScript);

        // Host provided function methods...

            // Register host provided function...
            bool RegisterHostProvidedFunction(
                                Script hThread, const char *pszName,
                                HostProvidedFunction *pHostProvidedFunction);

        // Methods to be called from within a script called host function...

            // Return nothing from within host function...
            void ReturnVoidFromHost(Script hScript, uint8 unParameters);

            // Return an integer from within host function...
            void ReturnIntegerFromHost(Script hScript, uint8 unParameters,
                                       int nReturnValue);

            // Return a float from within host function...
            void ReturnFloatFromHost(Script hScript, uint8 unParameters,
                                     float fReturnValue);

            // Return a string from within a host function...
            void ReturnStringFromHost(Script hScript, uint8 unParameters,
                                      char *pszReturnValue);

        // Deconstructor shuts down runtime enviroment...
       ~CAgni();

    // Protected data types, macros, and other miscellaneous things...
    protected:

        // Common structures...
        #include "AgniCommonDefinitions.h"

        // Host provided function...
        typedef struct _AVM_HostProvidedFunction
        {
            // Host function loaded flag...
            boolean                 bLoaded;

            // Script this function is visible to...
            Script                  hScriptVisibleTo;

            // Function name...
            char                    szName[256];

            // Host routine's entry point...
            HostProvidedFunction   *pEntryPoint;

        }AVM_HostProvidedFunction;

        // Runtime value... (used in stack, registers, and instruction stream)
        typedef struct _AVM_RuntimeValue
        {
            // Operand type...
            uint8           OperandType;

            // Operand...
            union
            {
                // Literal integer...
                int32           nLiteralInteger;

                // Literal float...
                float32         fLiteralFloat;

                // String literal...
                char           *pszLiteralString;

                // Stack index... (second element needed only with relative
                //  stack indices to store offset)
                int32           nStackIndex[2];

                // Instruction index...
                int32           nInstructionIndex;

                // Function index...
                int32           nFunctionIndex;

                // Host function index...
                int32           nHostFunctionIndex;

                // Register identifier...
                uint8           Register;
            };

        }AVM_RuntimeValue;

        // Instruction structure...
        typedef struct _AVM_Instruction
        {
            // Operation code...
            uint16              usOperationCode;

            // Operand count...
            uint8               OperandCount;

            // Operand list...
            AVM_RuntimeValue   *pOperandList;

        }AVM_Instruction;

        // Instruction stream structure...
        typedef struct _AVM_InstructionStream
        {
            // The instruction stream itself...
            AVM_Instruction    *pInstructions;

            // Instruction pointer...
            uint32              unInstructionPointer;

        }AVM_InstructionStream;

        // Runtime stack structure...
        typedef struct _AVM_RuntimeStack
        {
            // Stack elements...
            AVM_RuntimeValue   *pElements;

            // Top index...
            int32               nTopIndex;

            // Index of the top of the current stack frame...
            uint32              unCurrentStackFrameTopIndex;

        }AVM_RuntimeStack;

        // Script structure...
        typedef struct _AVM_Script
        {
            // Is this script loaded?
            boolean                         bLoaded;

            // Is it executing?
            boolean                         bExecuting;

            // Main header...
            Agni_MainHeader                 MainHeader;

            // Instruction stream header...
            Agni_InstructionStreamHeader    InstructionStreamHeader;

            // Instruction stream...
            AVM_InstructionStream           InstructionStream;

            // String stream header...
            Agni_StringStreamHeader         StringStreamHeader;

            // Function table header...
            Agni_FunctionTableHeader        FunctionTableHeader;

            // Function table...
            Agni_Function                  *pFunctionTable;

            // Host function table header...
            Agni_HostFunctionTableHeader    HostFunctionTableHeader;

            // Host function table...
            Agni_HostFunction              *pHostFunctionTable;

            // Paused, and if so, until what time?
            boolean                         bPaused;
            uint32                          unPauseEndTime;

            // Thread time slice...
            uint32                          unThreadTimeSlice;

            // Registers...
            
                // General purpose...
                AVM_RuntimeValue            _RegisterT0;
                AVM_RuntimeValue            _RegisterT1;

                // Return...
                AVM_RuntimeValue            _RegisterReturn;

            // Runtime stack...
            AVM_RuntimeStack                Stack;

        }AVM_Script;

        // Default stack size...
        #define DEFAULT_STACK_SIZE              1024

        // Maximum string coercion length...
        #define MAXIMUM_COERCION_LENGTH         63

        // Maximum number of threads...
        #define MAXIMUM_THREADS                 1024

        // Maximum number of host provided functions...
        #define MAXIMUM_HOST_PROVIDED_FUNCTIONS 256

        // Threading modes...
        enum THREADING_MODE
        {
            THREADING_MODE_MULTIPLE = 0,
            THREADING_MODE_SINGLE
        };

        // Threading priority time slice durations...
        enum THREAD_PRIORITY_DURATION
        {
            THREAD_PRIORITY_LOW_DURATION    = 20,
            THREAD_PRIORITY_MEDIUM_DURATION = 40,
            THREAD_PRIORITY_HIGH_DURATION   = 80
        };

        // Resolve stack index for current thread if relative to absolute...
        #define ResolveStackIndex(hScript, nIndex) \
            (nIndex < 0 ? nIndex += Scripts[hScript].Stack. \
                unCurrentStackFrameTopIndex  : nIndex)

        // Is thread a valid index and loaded?
        #define IsValidThread(nIndex) \
            ((nIndex < 0 || nIndex > MAXIMUM_THREADS ? false : true) && \
             Scripts[nIndex ].bLoaded ? true : false)

        // Is index refer to a valid function?
        #define IsValidFunctionIndex(hScript, nIndex) \
            (nIndex < 0 || nIndex > Scripts[hScript]. \
                FunctionTableHeader.unSize ? false : true)

        // Is index refer to a valid host function?
        #define IsValidHostFunctionIndex(hScript, nIndex) \
            (nIndex < 0 || nIndex > Scripts[hScript]. \
                HostFunctionTableHeader.unSize ? false : true)

    // Protected data...
    protected:

        // Checksum calculation register...
        uint32  unTempCheckSum;

        // Host version...
        char   *pszHostName;
        uint8   HostVersionMajor;
        uint8   HostVersionMinor;

        // Script array...
        AVM_Script  Scripts[MAXIMUM_THREADS];

        // Host provided function table...
        AVM_HostProvidedFunction
            HostProvidedFunctionTable[MAXIMUM_HOST_PROVIDED_FUNCTIONS];

        // Threading...
        uint8   CurrentThreadingMode;
        Script  hCurrentThread;
        uint32  unCurrentThreadActivationTime;

    // Protected methods...
    protected:

        // Checksum calculation...

            // Calculate checksum of executable file at given path...
            uint32 CalculateCheckSumOfExecutable(const char *pszPath);

            // Acknowledge bit in calculation...
            void CheckSum_PutBit(boolean Bit);

            // Acknowledge byte in calculation...
            void CheckSum_PutByte(uint8 Byte);

            // Acknowledge stream of bytes in calculation...
            void CheckSum_PutBytes(uint8 *pBytes, uint32 unSize);

        // Loading...

            /* Display statistics... (for debugging purposes only)
            void DisplayStatistics(Script hScript) const;*/

            // Load bytes or throw error string...
            void LoadBytes(void *pStorageBuffer, uint32 unEachOfSize,
                           uint32 unMembers, FILE *hFile);

            // Check version...
            bool VersionSafe(uint8 AvailableMajor, uint8 AvailableMinor,
                             uint8 RequestedMajor, uint8 RequestedMinor);

        // Function interfacing...

            // The actual implementation to call script functions any way...
            void CallFunctionImplementation(Script hScript, uint32 unIndex);

            // Get a function by index or return NULL on error...
            Agni_Function GetFunction(Script hScript, uint32 unIndex);

            // Get a function index by name or return -1 on error...
            int32 GetFunctionIndexByName(Script hScript, char *pszName);

            // Get script's host function name for current thread by index...
            char *GetHostFunction(uint32 unIndex);

        // Operand coercion...

            // Coerce value to integer or throw error string...
            int32 CoerceValueToInteger(AVM_RuntimeValue RuntimeValue);

            // Coerce value to float or throw error string...
            float32 CoerceValueToFloat(AVM_RuntimeValue RuntimeValue);

            // Coerce value to string or throw error string...
            char *CoerceValueToString(AVM_RuntimeValue RuntimeValue);

        // Operand resolution...

            // Copy source value into destination or throw error string...
            void CopyValue(AVM_RuntimeValue *pDestinationValue,
                           AVM_RuntimeValue SourceValue);

    		// Get operand type as exists in instruction stream...
            uint8 GetOperandType(uint8 OperandIndex);

            // Resolve operand's stack index, whether relative or absolute
            //   or throw error string...
    		int32 ResolveOperandStackIndex(uint8 OperandIndex);

    		// Resolve an operand's value or throw error string...
            AVM_RuntimeValue ResolveOperandValue(uint8 OperandIndex);

            // Resolves final type of operand and returns the resolved type...
    		uint8 ResolveOperandType(uint8 OperandIndex);

    		// Resolve operand as an integer...
            int32 ResolveOperandAsInteger(uint8 OperandIndex);

            // Resolve operand as a float...
            float32 ResolveOperandAsFloat(uint8 OperandIndex);

    		// Resolve operand as a string...
            char *ResolveOperandAsString(uint8 OperandIndex);

            // Resolve operand as an instruction index...
    		int32 ResolveOperandAsInstructionIndex(uint8 OperandIndex);

    		// Resolve operand as a function index...
            int32 ResolveOperandAsFunctionIndex(uint8 OperandIndex);

            // Resolve operand as a host function call...
    		int32 ResolveOperandAsHostFunctionIndex(uint8 OperandIndex);

            // Resolves operand and returns a pointer to it's runtime value or
            //  NULL if not applicable...
    		AVM_RuntimeValue *ResolveOperandAsPointer(uint8 OperandIndex);

        // Runtime stack interface...

            // Get a runtime value on the stack or throw error string...
            AVM_RuntimeValue GetStackValue(Script hScript, int32 nIndex);

    		// Pop value off of the stack or throw error string...
    		AVM_RuntimeValue Pop(Script hScript);

            // Push value onto the stack or throw error string...
    		void Push(Script hScript, AVM_RuntimeValue RuntimeValue);

    		// Push a stack frame onto the stack or throw error string...
            void PushStackFrame(Script hScript, uint32 unSize);

    		// Pop stack frame off of the stack or throw error string...
            void PopStackFrame(Script hScript, uint32 unSize);

            // Set stack value or throw error string...
    		void SetStackValue(Script hScript, int32 nIndex,
                               AVM_RuntimeValue RuntimeValue);
};

#endif
