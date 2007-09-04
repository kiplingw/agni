/*
  Name:         CAgni.cpp (implementation)
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  VirtualMachine implementation...
*/

// Includes...

    // Virtual machine definition...
    #include "../include/CAgni.h"

// Using the Agni namespace...
using namespace Agni;

// Constructor initializes runtime enviroment...
VirtualMachine::VirtualMachine(char *_pszHostName, uint8 _HostVersionMajor,
                               uint8 _HostVersionMinor)
{
    // Reset tables and variables to initial state...
    memset(&HostProvidedFunctionTable, '\x0',
           sizeof(HostProvidedFunctionTable));
    memset(&Scripts, '\x0', sizeof(Scripts));
    CurrentThreadingMode            = THREADING_MODE_MULTIPLE;
    hCurrentThread                  = (uint32) -1;
    unCurrentThreadActivationTime   = 0;

    // Remember host version...
    pszHostName         = _pszHostName ? strdup(_pszHostName) : NULL;
    HostVersionMajor = _HostVersionMajor;
    HostVersionMinor = _HostVersionMinor;
}

// Calculate checksum of file at given path...
uint32 VirtualMachine::CalculateCheckSumOfExecutable(const char *pszPath)
{
    // Variables...
    FILE               *hFile                       = NULL;
    Agni_MainHeader     DummyHeader;
    int32               nFileCheckSumFieldStart     = 0x00000000;
    int32               nFileCheckSumFieldEnd       = 0x00000000;
    uint8               byte                        = 0x00;

    // Calculate executable's checksum field's start offset...
    nFileCheckSumFieldStart = (uint8 *) &DummyHeader.unCheckSum -
                              (uint8 *) &DummyHeader;

    // Calculate executable's checksum field's end offset...
    nFileCheckSumFieldEnd = nFileCheckSumFieldStart;
    nFileCheckSumFieldEnd += sizeof(DummyHeader.unCheckSum);

    // Open file...
    hFile = fopen(pszPath, "rb");

        // Failed...
        if(!hFile)
            return 0x00000000;

    // Clear CRC register...
    unTempCheckSum = 0x00000000;

    // Calculate for each byte, while there are some...
    while(1 == fread(&byte, sizeof(byte), 1, hFile))
    {
        // We are reading the checksum field of the executable, assume zero...
        if((nFileCheckSumFieldStart <= ftell(hFile) - 1) &&
           (ftell(hFile) - 1 < nFileCheckSumFieldEnd))
            byte = 0x00;

        // Add byte to computation...
        CheckSum_PutByte(byte);
    }

    // Cleanup...
    fclose(hFile);

    // Return checksum to caller...
    return unTempCheckSum;
}

// Call script function asynchronously... (blocking)
boolean VirtualMachine::CallFunction(Script hScript, char *pszName)
{
    // Variables...
    int32               nFunctionIndex          = 0;
    uint8               PreviousThreadingMode   = 0;
    Script              hPreviousThread         = 0;
    AVM_RuntimeValue    StackBase;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Locate function...
    nFunctionIndex = GetFunctionIndexByName(hScript, pszName);

        // Failed...
        if(nFunctionIndex == -1)
            return false;

    // Save current virtual machine state...
    PreviousThreadingMode   = CurrentThreadingMode;
    hPreviousThread         = hCurrentThread;

    // Switch to single thread execution mode...
    CurrentThreadingMode = THREADING_MODE_SINGLE;

    // Switch active thread to user's script...
    hCurrentThread = hScript;

    // Call the function...
    CallFunctionImplementation(hScript, nFunctionIndex);

    // Set the stack base marker...

        // Find stack base...
        StackBase = GetStackValue(hCurrentThread,
                                  Scripts[hCurrentThread].Stack.nTopIndex - 1);

        // Set it...
        StackBase.OperandType = OT_AVM_STACK_BASE_MARKER;
        SetStackValue(hCurrentThread,
                      Scripts[hCurrentThread].Stack.nTopIndex - 1, StackBase);

    // Let script run until it returns...
    RunScripts(THREAD_PRIORITY_INFINITE);

    // Restore the virtual machine state...
    CurrentThreadingMode = PreviousThreadingMode;
    hCurrentThread = hPreviousThread;

    // Done...
    return true;
}

// The actual implementation to call script functions any way...
void VirtualMachine::CallFunctionImplementation(Script hScript, uint32 unIndex)
{
    // Variables...
    Agni_Function       DestinationFunction;
    int32               nFrameIndex             = 0;
    AVM_RuntimeValue    CallerReturnAddress;
    AVM_RuntimeValue    FunctionIndex;

    // Get the function...
    DestinationFunction = GetFunction(hScript, unIndex);

    // Save current stack frame index...
    nFrameIndex = Scripts[hScript].Stack.unCurrentStackFrameTopIndex;

    // Fetch caller's return address...
    CallerReturnAddress.OperandType = OT_AVM_INDEX_INSTRUCTION;
    CallerReturnAddress.nInstructionIndex =
        Scripts[hScript].InstructionStream.unInstructionPointer;

    // Push caller's return address onto the stack...
    Push(hScript, CallerReturnAddress);

    // Push stack frame plus extra space for function index...
    PushStackFrame(hScript, DestinationFunction.unLocalDataSize + 1);

    // Save new function's index and old stack frame to the top of the stack...
    FunctionIndex.nStackIndex[0] = unIndex;
    FunctionIndex.nStackIndex[1] = nFrameIndex;
    SetStackValue(hScript, Scripts[hScript].Stack.nTopIndex - 1, FunctionIndex);

    // Jump to the script routine's entry point...
    Scripts[hScript].InstructionStream.unInstructionPointer
        = DestinationFunction.unEntryPoint;
}

// Call script function synchronously... (non-blocking)
boolean VirtualMachine::CallFunctionSynchronously(Script hScript, char *pszName)
{
    // Variables..
    int32 nFunctionIndex = 0;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Get the function index...
    nFunctionIndex = GetFunctionIndexByName(hScript, pszName);

        // Not found...
        if(nFunctionIndex == -1)
            return false;

    // Call the function...
    CallFunctionImplementation(hScript, nFunctionIndex);

    // Done...
    return true;
}

// Acknowledge bit in calculation...
inline void VirtualMachine::CheckSum_PutBit(boolean Bit)
{
    // Variables...
    boolean TopBit = 0;

    // Extract top most bit in calculation register...
    TopBit = (unTempCheckSum & 0x80000000) != 0;

    // Shift bits left one and insert new bit at the extreme right...
    unTempCheckSum <<= 1;
    unTempCheckSum ^= (unsigned long) Bit;

    // Calculate checksum...
    if(TopBit)
        unTempCheckSum ^= unCheckSumKey;
}

// Acknowledge byte in calculation...
inline void VirtualMachine::CheckSum_PutByte(uint8 Byte)
{
    // Variables...
    uint16  usBit   = 0;
    uint8   BitMask = 0x00;

    // Iterate through byte, acknowledging each bit from left to right...
    for(usBit = 0, BitMask = 0x80; usBit < 8; usBit++)
    {
        // Acknowledge bit of interest...
        CheckSum_PutBit((Byte & BitMask) != 0);

        // Adjust bit mask for next bit to the right...
        BitMask >>= 1;
    }
}

// Acknowledge stream of bytes in calculation...
void VirtualMachine::CheckSum_PutBytes(uint8 *pBytes, uint32 unSize)
{
    // Variables...
    unsigned long unIndex = 0;

    // Calculate for each byte...
    for(unIndex = 0; unIndex < unSize; unIndex++)
        CheckSum_PutByte(pBytes[unIndex]);
}

// Coerce value to float or throw error string...
float32 VirtualMachine::CoerceValueToFloat(AVM_RuntimeValue RuntimeValue)
{
    // Coerce differently, depending on type...
    switch(RuntimeValue.OperandType)
    {
        // Float...
        case OT_AVM_FLOAT:
            return RuntimeValue.fLiteralFloat;

        // Integer...
        case OT_AVM_INTEGER:
            return (float) RuntimeValue.nLiteralInteger;

        // String...
        case OT_AVM_STRING:
            return (float) atof(RuntimeValue.pszLiteralString);

        // Anything else should be treated as invalid...
        default:
            throw "coercion attempted on unknown operand type";
    }
}

// Coerce value to integer or throw error string...
int32 VirtualMachine::CoerceValueToInteger(AVM_RuntimeValue RuntimeValue)
{
    // Coerce differently, depending on type...
    switch(RuntimeValue.OperandType)
    {
        // Float...
        case OT_AVM_FLOAT:
            return (int) RuntimeValue.fLiteralFloat;

        // Integer...
        case OT_AVM_INTEGER:
            return RuntimeValue.nLiteralInteger;

        // String...
        case OT_AVM_STRING:
            return atoi(RuntimeValue.pszLiteralString);

        // Anything else should be treated as invalid...
        default:
            throw "coercion attempted on unknown operand type";
    }
}

// Coerce value to string or throw error string...
char *VirtualMachine::CoerceValueToString(AVM_RuntimeValue RuntimeValue)
{
    // Variables...
    char   *pszCoercion = NULL;

    // Value is not already a string...
    if(RuntimeValue.OperandType != OT_AVM_STRING)
    {
        // Allocate storage space...
        pszCoercion = (char *) malloc(MAXIMUM_COERCION_LENGTH + 1);

            // Failed...
            if(!pszCoercion)
                throw "memory allocation failed";
    }

    // Coerce differently, depending on type...
    switch(RuntimeValue.OperandType)
    {
        // Float...
        case OT_AVM_FLOAT:
        {
            // Convert to string and return it...
            sprintf(pszCoercion, "%f", RuntimeValue.fLiteralFloat);
            return pszCoercion;
        }

        // Integer...
        case OT_AVM_INTEGER:
        {
            // Convert to string and return it...
//            itoa(RuntimeValue.nLiteralInteger, pszCoercion, 10);
            sprintf(pszCoercion, "%d", RuntimeValue.nLiteralInteger);
            return pszCoercion;
        }

        // String...
        case OT_AVM_STRING:
            return RuntimeValue.pszLiteralString;

        // Anything else is invalid...
        default:
            throw "coercion attempted on unknown operand type";
    }
}

// Copy source value into destination or throw error string...
void VirtualMachine::CopyValue(AVM_RuntimeValue *pDestinationValue,
                      AVM_RuntimeValue SourceValue)
{
    // Destination already contains a string, so free it...
    if(pDestinationValue->OperandType == OT_AVM_STRING)
        free(pDestinationValue->pszLiteralString);

    // Copy source to destination...

        // Copy data...
       *pDestinationValue = SourceValue;

        // Make a copy of the string, if necessary...
        if(SourceValue.OperandType == OT_AVM_STRING)
        {
            // Allocate and copy...
            pDestinationValue->pszLiteralString =
                                        strdup(SourceValue.pszLiteralString);

                // Failed...
                if(!pDestinationValue->pszLiteralString)
                {
                    // Cleanup...
                    SourceValue.OperandType = OT_AVM_NULL;

                    // Abort...
                    throw "memory allocation failed";
                }
        }
}

/* Display statistics...
void VirtualMachine::DisplayStatistics(Script hScript) const
{
    // Display statistics...

        // Stack size...
        printf("\t            Stack size: ");

            // Manually specified...
            if(Scripts[hScript].MainHeader.unStackSize != (unsigned) -1)
                printf("%d\n", Scripts[hScript].MainHeader.unStackSize);

            // Default...
            else
                printf("default\n");

        // Thread priority...
        printf("\t       Thread priority: ");
        switch(Scripts[hScript].MainHeader.ThreadPriorityType)
        {
            // User specified...
            case THREAD_PRIORITY_USER:
                printf("%dms time slice\n",
                            Scripts[hScript].MainHeader.unThreadPriorityUser);
                break;

            // Low...
            case THREAD_PRIORITY_LOW:
                printf("low\n");
                break;

            // Medium...
            case THREAD_PRIORITY_MEDIUM:
                printf("medium\n");
                break;

            // High...
            case THREAD_PRIORITY_HIGH:
                printf("high\n");
                break;
        }

        // Symbols and other data...
        printf("\t              Checksum: 0x%08x\n"
                "\tInstructions assembled: %d\n"
               "\t       String literals: %d\n"
               "\t    Host API functions: %d\n"
               "\t             Functions: %d\n"
               "\t         Main present?: %s\n\n",

               Scripts[hScript].MainHeader.unCheckSum,
               Scripts[hScript].InstructionStreamHeader.unSize,
               Scripts[hScript].StringStreamHeader.unSize,
               Scripts[hScript].HostFunctionTableHeader.unSize,
               Scripts[hScript].FunctionTableHeader.unSize,
               (Scripts[hScript].MainHeader.unMainIndex
                                            == (unsigned) -1) ? "No" : "Yes");
}*/

// Get a function by index or return NULL on error...
inline Agni_Function VirtualMachine::GetFunction(Script hScript, uint32 unIndex)
{
    // Return it...
    return Scripts[hScript].pFunctionTable[unIndex];
}

// Get a function index by name or return -1 on error...
int32 VirtualMachine::GetFunctionIndexByName(Script hScript, char *pszName)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return -1;

    // Scan through function table...
    for(uint32 unFunctionTableIndex = 0;
        unFunctionTableIndex < Scripts[hScript].FunctionTableHeader.unSize;
        unFunctionTableIndex++)
    {
        // Located...
        if(strcasecmp(Scripts[hScript].pFunctionTable[unFunctionTableIndex].
                        szName, pszName) == 0)
            return unFunctionTableIndex;
    }

    // Function not found...
    return -1;
}

// Get the script's host function name for the current thread by index...
inline char *VirtualMachine::GetHostFunction(uint32 unIndex)
{
    // Return it...
    return Scripts[hCurrentThread].pHostFunctionTable[unIndex].szName;
}

// Get operand type as exists in instruction stream...
inline uint8 VirtualMachine::GetOperandType(uint8 OperandIndex)
{
    // Variables...
    uint32  unCurrentInstruction    = 0;

    // Get the current instruction's index...
    unCurrentInstruction = Scripts[hCurrentThread].InstructionStream.
                                unInstructionPointer;

    // Return operand type...
    return Scripts[hCurrentThread].InstructionStream.
            pInstructions[unCurrentInstruction].pOperandList[OperandIndex].
            OperandType;
}

// Get passed parameter as an integer...
int VirtualMachine::GetParameterAsInteger(Script hScript, uint8 unParameter)
{
    // Variables...
    int32               nTopIndex           = 0;
    int32               nComputedLocation   = 0;
    AVM_RuntimeValue    Parameter;

    // Find the index of the top of this script's stack...
    nTopIndex = Scripts[hScript].Stack.nTopIndex;

    // Compute the location of the parameter on the stack...
    nComputedLocation = nTopIndex - (unParameter + 1);

    // Extract the parameter...
    Parameter = Scripts[hScript].Stack.pElements[nComputedLocation];

    // Return the parameter coerced as an integer...
    return CoerceValueToInteger(Parameter);
}

// Get passed parameter as a float...
float VirtualMachine::GetParameterAsFloat(Script hScript, uint8 unParameter)
{
    // Variables...
    int32               nTopIndex           = 0;
    int32               nComputedLocation   = 0;
    AVM_RuntimeValue    Parameter;

    // Find the index of the top of this script's stack...
    nTopIndex = Scripts[hScript].Stack.nTopIndex;

    // Compute the location of the parameter on the stack...
    nComputedLocation = nTopIndex - (unParameter + 1);

    // Extract the parameter...
    Parameter = Scripts[hScript].Stack.pElements[nComputedLocation];

    // Return the parameter coerced as a float...
    return CoerceValueToFloat(Parameter);
}

// Get passed parameter as a string...
char *VirtualMachine::GetParameterAsString(Script hScript, uint8 unParameter)
{
    // Variables...
    int32               nTopIndex           = 0;
    int32               nComputedLocation   = 0;
    AVM_RuntimeValue    Parameter;

    // Find the index of the top of this script's stack...
    nTopIndex = Scripts[hScript].Stack.nTopIndex;

    // Compute the location of the parameter on the stack...
    nComputedLocation = nTopIndex - (unParameter + 1);

    // Extract the parameter...
    Parameter = Scripts[hScript].Stack.pElements[nComputedLocation];

    // Return the parameter coerced as a string...
    return CoerceValueToString(Parameter);
}

// Get return as a float from an asynchronous call...
float VirtualMachine::GetReturnValueAsFloat(Script hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return 0.0f;

    // Return it...
    return Scripts[hScript]._RegisterReturn.fLiteralFloat;
}

// Get return as an integer from an asynchronous call...
int VirtualMachine::GetReturnValueAsInteger(Script hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return 0;

    // Return it...
    return Scripts[hScript]._RegisterReturn.nLiteralInteger;
}

// Get return as a string from an asynchronous call...
char *VirtualMachine::GetReturnValueAsString(Script hScript, char *pszBuffer,
                                             uint32 unBufferSize)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return NULL;

    // Supplied buffer too small...
    if(strlen(Scripts[hScript]._RegisterReturn.pszLiteralString) >
       (unBufferSize - 1))
        return NULL;

    // Store it in caller's buffer...
    strcpy(pszBuffer, Scripts[hScript]._RegisterReturn.pszLiteralString);

    // Done...
    return pszBuffer;
}

// Get a runtime value on the stack...
inline VirtualMachine::AVM_RuntimeValue 
    VirtualMachine::GetStackValue(Script hScript, int32 nIndex)
{
    // Get element at specified index...
    return Scripts[hScript].Stack.pElements[ResolveStackIndex(hScript, nIndex)];
}

// Load bytes or throws error code...
void VirtualMachine::LoadBytes(void *pStorageBuffer, uint32 unEachOfSize,
                               uint32 unMembers, FILE *hFile)
{
    // Read and check for error...
    if(unMembers != fread(pStorageBuffer, unEachOfSize, unMembers, hFile))
        throw Bad_Executable;
}

// Load script, store handle, return a status code...
VirtualMachine::Status 
    VirtualMachine::LoadScript(const char *pszPath, Script &hScript)
{
    // Variables...
    FILE   *hScriptFile                 = NULL;
    char    szBuffer[1024]              = {0};
    uint32  unCurrentInstructionIndex   = 0;
    uint16  usCurrentOperandIndex       = 0;
    uint16  usCurrentStringIndex        = 0;
    uint16  usCurrentFunctionIndex      = 0;
    uint16  usCurrentHostFunctionIndex  = 0;

    // Try to load script...
    try
    {
        // Find a free script handle index...
        for(hScript = 0; hScript < (MAXIMUM_THREADS - 1); hScript++)
        {
            // Found one...
            if(!Scripts[hScript].bLoaded)
                break;
        }

            // Scanned entire list without finding vacancy, failed...
            if(hScript + 1 == (MAXIMUM_THREADS - 1))
                throw Threads_Exhausted;

        // Clear script...
        memset(&Scripts[hScript], 0, sizeof(AVM_Script));

        // Open script...
        hScriptFile = fopen(pszPath, "rb");

            // Failed...
            if(!hScriptFile)
                throw Cannot_Open;

        // Process main header...

            // Load main header...
            LoadBytes(&Scripts[hScript].MainHeader, sizeof(Agni_MainHeader), 1,
                      hScriptFile);

            // Check signature...

                // Initialize...
                memset(szBuffer, '\x90', sizeof(szBuffer));
                strncpy(&szBuffer[2], "AGNI", strlen("AGNI"));

                // Check...
                if(memcmp(&Scripts[hScript].MainHeader.Signature, szBuffer,
                          sizeof(Scripts[hScript].MainHeader.Signature)) != 0)
                    throw Bad_Executable;

            // Check checksum...
            if(CalculateCheckSumOfExecutable(pszPath) !=
               Scripts[hScript].MainHeader.unCheckSum)
                throw Bad_CheckSum;

            // Check required Agni runtime version...
            if(!VersionSafe(AGNI_VERSION_MAJOR, AGNI_VERSION_MINOR,
                            Scripts[hScript].MainHeader.ucMajorRequiredAgniVersion,
                            Scripts[hScript].MainHeader.ucMinorRequiredAgniVersion))
                throw Old_Agni_Runtime;

            // Check host, if any host information provided...
            if(Scripts[hScript].MainHeader.unHostStringIndex != (uint32) -1)
            {
                // Check host version...
                if(!VersionSafe(HostVersionMajor, HostVersionMinor,
                                Scripts[hScript].MainHeader.ucHostMajorVersion,
                                Scripts[hScript].MainHeader.ucHostMinorVersion))
                    throw Old_Host_Runtime;
            }

            // Check for default stack size...
            if(Scripts[hScript].MainHeader.unStackSize == (uint32) -1)
                Scripts[hScript].MainHeader.unStackSize = DEFAULT_STACK_SIZE;

            // Allocate runtime stack...
            Scripts[hScript].Stack.pElements =
                (AVM_RuntimeValue *) calloc(Scripts[hScript].MainHeader.unStackSize,
                                            sizeof(AVM_RuntimeValue));

                // Failed...
                if(!Scripts[hScript].Stack.pElements)
                    throw Memory_Allocation;

            // Set thread's time slice duration...
            switch(Scripts[hScript].MainHeader.ThreadPriorityType)
            {
                // Low...
                case THREAD_PRIORITY_LOW:

                    // Store default thread time slice for low priority...
                    Scripts[hScript].unThreadTimeSlice =
                        THREAD_PRIORITY_LOW_DURATION;

                    // Done...
                    break;

                // Medium...
                case THREAD_PRIORITY_MEDIUM:

                    // Store default thread time slice for medium priority...
                    Scripts[hScript].unThreadTimeSlice =
                        THREAD_PRIORITY_MEDIUM_DURATION;

                    // Done...
                    break;

                // High...
                case THREAD_PRIORITY_HIGH:

                    // Store default thread time slice for high priority...
                    Scripts[hScript].unThreadTimeSlice =
                        THREAD_PRIORITY_HIGH_DURATION;

                    // Done...
                    break;

                // User...
                case THREAD_PRIORITY_USER:

                    // Extract and store user time slice from main header...
                    Scripts[hScript].unThreadTimeSlice =
                        Scripts[hScript].MainHeader.unThreadPriorityUser;

                    // Done...
                    break;

                // Unknown...
                default:
                    throw Bad_Executable;
            }

        // Process instruction stream...

            // Load instruction stream header...
            LoadBytes(&Scripts[hScript].InstructionStreamHeader,
                      sizeof(Agni_InstructionStreamHeader), 1, hScriptFile);

            // Allocate instruction stream...
            Scripts[hScript].InstructionStream.pInstructions =
                (AVM_Instruction *) calloc(Scripts[hScript].
                                            InstructionStreamHeader.unSize,
                                           sizeof(AVM_Instruction));

                // Failed...
                if(!Scripts[hScript].InstructionStream.pInstructions)
                    throw Memory_Allocation;

            // Load instruction stream...
            for(unCurrentInstructionIndex = 0;
                unCurrentInstructionIndex < Scripts[hScript].
                    InstructionStreamHeader.unSize;
                unCurrentInstructionIndex++)
            {
                // Variables...
                uint8               OperandCount    = 0x00;
                AVM_RuntimeValue   *pOperandList   = NULL;

                // Load this instructions operation code... (2 bytes)
                LoadBytes(&Scripts[hScript].InstructionStream.
                            pInstructions[unCurrentInstructionIndex].
                            usOperationCode,
                          sizeof(uint16), 1, hScriptFile);

                // Load operand count... (1 byte)
                LoadBytes(&OperandCount, sizeof(uint8), 1, hScriptFile);
                Scripts[hScript].InstructionStream.
                    pInstructions[unCurrentInstructionIndex].OperandCount =
                        OperandCount;

                // This operation has operands, allocate storage space...
                if(OperandCount > 0)
                {
                    // Allocate...
                    pOperandList = (AVM_RuntimeValue *)
                        calloc(OperandCount, sizeof(AVM_RuntimeValue));
                }

                // Load operand list...
                for(usCurrentOperandIndex = 0;
                    usCurrentOperandIndex < OperandCount;
                    usCurrentOperandIndex++)
                {
                    // Load operand type... (1 byte)
                    LoadBytes(&pOperandList[usCurrentOperandIndex].OperandType,
                              sizeof(uint8), 1, hScriptFile);

                    // Load operand data...
                    switch(pOperandList[usCurrentOperandIndex].OperandType)
                    {
                        // Integer literal... (4 bytes)
                        case OT_AVM_INTEGER:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nLiteralInteger, sizeof(int32), 1,
                                      hScriptFile);
                            break;
                        }

                        // Floating-point literal... (4 bytes)
                        case OT_AVM_FLOAT:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        fLiteralFloat, sizeof(float32), 1,
                                      hScriptFile);
                            break;
                        }

                        // String index... (4 bytes)
                        case OT_AVM_INDEX_STRING:
                        {
                            // Load... (nStringTableIndex -> nLiteralInteger)
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nLiteralInteger, sizeof(int32), 1,
                                      hScriptFile);
                            break;
                        }

                        // Instruction index... (4 bytes)
                        case OT_AVM_INDEX_INSTRUCTION:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nInstructionIndex, sizeof(int32), 1,
                                      hScriptFile);
                            break;
                        }

                        // Absolute stack index... (4 bytes)
                        case OT_AVM_INDEX_STACK_ABSOLUTE:
                        {
                            // Load... (second element useful only for relative)
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nStackIndex[0], sizeof(int32), 1,
                                      hScriptFile);
                            break;
                        }

                        // Relative stack index... (4 + 4 bytes)
                        case OT_AVM_INDEX_STACK_RELATIVE:
                        {
                            // Load base index...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nStackIndex[0], sizeof(int32), 1,
                                      hScriptFile);

                            // Load offset index...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nStackIndex[1], sizeof(int32), 1,
                                      hScriptFile);

                            // Done...
                            break;
                        }

                        // Function index... (4 bytes)
                        case OT_AVM_INDEX_FUNCTION:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                      nFunctionIndex,
                                      sizeof(int32), 1, hScriptFile);
                            break;
                        }

                        // Host function index... (4 bytes)
                        case OT_AVM_INDEX_FUNCTION_HOST:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        nHostFunctionIndex, sizeof(int32), 1,
                                      hScriptFile);
                            break;
                        }

                        // Register... (1 byte)
                        case OT_AVM_REGISTER:
                        {
                            // Load...
                            LoadBytes(&pOperandList[usCurrentOperandIndex].
                                        Register, sizeof(uint8), 1,
                                      hScriptFile);
                            break;
                        }

                        // Unknown...
                        default:
                            throw Bad_Executable;
                    }
                }

                // Store operands in instruction stream...
                Scripts[hScript].InstructionStream.
                    pInstructions[unCurrentInstructionIndex].pOperandList =
                        pOperandList;
            }

        // Process string stream...

            // Process string stream header...
            //  (sizeof(Agni_StringStreamHeader) bytes)
            LoadBytes(&Scripts[hScript].StringStreamHeader,
                      sizeof(Agni_StringStreamHeader), 1, hScriptFile);

            // Load string table, if any strings to load...
            if(Scripts[hScript].StringStreamHeader.unSize > 0)
            {
                // Variables...
                char  **ppszStringTable  = NULL;

                // Allocate...
                ppszStringTable = (char **)
                    calloc(Scripts[hScript].StringStreamHeader.unSize,
                           sizeof(char *));

                    // Failed...
                    if(!ppszStringTable)
                        throw Memory_Allocation;

                // Load each string...
                for(usCurrentStringIndex = 0;
                    usCurrentStringIndex < Scripts[hScript].StringStreamHeader.unSize;
                    usCurrentStringIndex++)
                {
                    // Variables...
                    uint32  unStringLength      = 0;
                    char   *pszCurrentString    = NULL;

                    // Load string length... (4 bytes)
                    LoadBytes(&unStringLength, sizeof(uint32), 1, hScriptFile);

                    // Allocate storage for string...
                    pszCurrentString = (char *) malloc(unStringLength + 1);

                        // Failed...
                        if(!pszCurrentString)
                        {
                            // Cleanup...
                            free(ppszStringTable);

                            // Abort...
                            throw Memory_Allocation;
                        }

                    // Load string and terminate... (unStringLength bytes)
                    LoadBytes(pszCurrentString, unStringLength, 1, hScriptFile);
                    pszCurrentString[unStringLength] = '\x0';

                    // Store in string table...
                    ppszStringTable[usCurrentStringIndex] = pszCurrentString;
                }

                // Scan instruction stream's operands, converting string table
                //  indices to string literals...
                for(unCurrentInstructionIndex = 0;
                    unCurrentInstructionIndex < Scripts[hScript].
                        InstructionStreamHeader.unSize;
                    unCurrentInstructionIndex++)
                {
                    // Variables...
                    uint8               OperandCount    = 0;
                    AVM_RuntimeValue   *pOperandList    = NULL;

                    // Fetch operand count...
                    OperandCount = Scripts[hScript].InstructionStream.
                            pInstructions[unCurrentInstructionIndex].OperandCount;

                    // Fetch operand list...
                    pOperandList = Scripts[hScript].InstructionStream.
                            pInstructions[unCurrentInstructionIndex].pOperandList;

                    // Scan operands for string table indices...
                    for(usCurrentOperandIndex = 0;
                        usCurrentOperandIndex < OperandCount;
                        usCurrentOperandIndex++)
                    {
                        // Variables...
                        uint32  unStringTableIndex      = 0;
                        char   *pszStringLiteralCopy    = NULL;

                        // Not a string index, skip...
                        if(pOperandList[usCurrentOperandIndex].OperandType !=
                           OT_AVM_INDEX_STRING)
                            continue;

                        // Fetch string table index...
                        //  (nStringTableIndex -> nLiteralInteger)
                        unStringTableIndex =
                            pOperandList[usCurrentOperandIndex].nLiteralInteger;

                        // Allocate string literal copy...
                        pszStringLiteralCopy =
                                    strdup(ppszStringTable[unStringTableIndex]);

                            // Failed...
                            if(!pszStringLiteralCopy)
                            {
                                // Cleanup...
                                free(ppszStringTable);

                                // Abort...
                                throw Memory_Allocation;
                            }

                        // Insert instance into instruction's operand data...
                        pOperandList[usCurrentOperandIndex].pszLiteralString =
                                                            pszStringLiteralCopy;

                        // Mark operand as string literal now...
                        pOperandList[usCurrentOperandIndex].OperandType =
                                                                    OT_AVM_STRING;
                    }
                }

                // The host and the script have both identified themselves...
                if(Scripts[hScript].MainHeader.unHostStringIndex != (uint32) -1
                    && pszHostName != NULL)
                {
                    // Host name does not match the scripts host name...
                    if(strcasecmp(ppszStringTable[Scripts[hScript].MainHeader.
                                    unHostStringIndex], pszHostName) != 0)
                    {
                        // Cleanup...

                            // Free temporary string table...

                                // Free original strings...
                                for(usCurrentStringIndex = 0;
                                    usCurrentStringIndex < Scripts[hScript].
                                        StringStreamHeader.unSize;
                                    usCurrentStringIndex++)
                                    free(ppszStringTable[usCurrentStringIndex]);

                                // Free the table itself...
                                free(ppszStringTable);

                        // Abort...
                        throw Wrong_Host;
                    }
                }

                // Free temporary string table...

                    // Free original strings...
                    for(usCurrentStringIndex = 0;
                        usCurrentStringIndex < Scripts[hScript].StringStreamHeader.
                            unSize;
                        usCurrentStringIndex++)
                        free(ppszStringTable[usCurrentStringIndex]);

                    // Free the table itself...
                    free(ppszStringTable);
            }

        // Process function table...

            // Load function table header... (sizeof(AVM_FunctionTableHeader) bytes)
            LoadBytes(&Scripts[hScript].FunctionTableHeader,
                      sizeof(Agni_FunctionTableHeader), 1, hScriptFile);

            // Allocate function table, if necessary...
            if(Scripts[hScript].FunctionTableHeader.unSize > 0)
            {
                // Allocate...
                Scripts[hScript].pFunctionTable = (Agni_Function *)
                    calloc(Scripts[hScript].FunctionTableHeader.unSize,
                           sizeof(Agni_Function));

                    // Failed...
                    if(!Scripts[hScript].pFunctionTable)
                        throw Memory_Allocation;
            }

            // Load each function...
            for(usCurrentFunctionIndex = 0;
                usCurrentFunctionIndex < Scripts[hScript].FunctionTableHeader.
                    unSize;
                usCurrentFunctionIndex++)
            {
                // Variables...
                uint32  unEntryPoint        = 0x00000000;
                uint8   ParameterCount      = 0;
                uint32  unLocalDataSize     = 0;
                uint32  unStackFrameSize    = 0;
                uint8   NameLength          = 0;

                // Load entry point... (4 bytes)
                LoadBytes(&unEntryPoint, sizeof(uint32), 1, hScriptFile);
                Scripts[hScript].pFunctionTable[usCurrentFunctionIndex].
                    unEntryPoint = unEntryPoint;

                // Load parameter count... (1 byte)
                LoadBytes(&ParameterCount, sizeof(uint8), 1, hScriptFile);
                Scripts[hScript].pFunctionTable[usCurrentFunctionIndex].
                    ParameterCount = ParameterCount;

                // Load local data size...
                LoadBytes(&unLocalDataSize, sizeof(uint32), 1, hScriptFile);
                Scripts[hScript].pFunctionTable[usCurrentFunctionIndex].
                    unLocalDataSize = unLocalDataSize;

                // Calculate stack frame size so we don't have to waste cycles
                //  recalculating...
                unStackFrameSize = ParameterCount + 1 + unLocalDataSize;
                Scripts[hScript].pFunctionTable[usCurrentFunctionIndex].
                    unStackFrameSize = unStackFrameSize;

                // Load function name...

                    // Name length... (1 byte)
                    LoadBytes(&NameLength, sizeof(uint8), 1, hScriptFile);

                    // Name... (NameLength bytes)
                    LoadBytes(&Scripts[hScript].
                              pFunctionTable[usCurrentFunctionIndex].szName,
                              NameLength, 1, hScriptFile);

                        // Terminate...
                        Scripts[hScript].pFunctionTable[usCurrentFunctionIndex].
                            szName[NameLength] = '\x0';
            }

        // Process host function table...

            // Load host function table header...
            LoadBytes(&Scripts[hScript].HostFunctionTableHeader,
                  sizeof(Agni_HostFunctionTableHeader), 1, hScriptFile);

            // Allocate host function table, if necessary...
            if(Scripts[hScript].HostFunctionTableHeader.unSize > 0)
            {
                // Allocate...
                Scripts[hScript].pHostFunctionTable = (Agni_HostFunction *)
                    calloc(Scripts[hScript].HostFunctionTableHeader.unSize,
                           sizeof(Agni_HostFunction));

                    // Failed...
                    if(!Scripts[hScript].pHostFunctionTable)
                        throw Memory_Allocation;
            }

            // Load each host function...
            for(usCurrentHostFunctionIndex = 0;
                usCurrentHostFunctionIndex < Scripts[hScript].
                    HostFunctionTableHeader.unSize;
                usCurrentHostFunctionIndex++)
            {
                // Variables...
                uint8 NameLength    = 0;

                // Load name...

                    // Length... (1 byte)
                    LoadBytes(&NameLength, sizeof(uint8), 1, hScriptFile);

                    // Name...
                    LoadBytes(&Scripts[hScript].
                              pHostFunctionTable[usCurrentHostFunctionIndex].
                                szName, NameLength, 1, hScriptFile);

                        // Terminate...
                        Scripts[hScript].
                            pHostFunctionTable[usCurrentHostFunctionIndex].
                            szName[NameLength] = '\x0';
            }
    }

        // Failed to load script...
        catch(Status Reason)
        {
            // Cleanup...

                // Close script file, if necessary...
                if(hScriptFile)
                    fclose(hScriptFile);

                // Runtime stack, if necessary...
                if(Scripts[hScript].Stack.pElements)
                    free(Scripts[hScript].Stack.pElements);

                // Instruction stream, if necessary...
                for(unCurrentInstructionIndex = 0;
                    unCurrentInstructionIndex <
                        Scripts[hScript].InstructionStreamHeader.unSize &&
                    Scripts[hScript].InstructionStream.pInstructions;
                    unCurrentInstructionIndex++)
                {
                    // Variables...
                    uint8               OperandCount    = 0;
                    AVM_RuntimeValue   *pOperandList    = NULL;

                    // Extract number of operands...
                    OperandCount = Scripts[hScript].InstructionStream.
                                    pInstructions[unCurrentInstructionIndex].
                                    OperandCount;

                    // Extract operand list...
                    pOperandList = Scripts[hScript].InstructionStream.
                                    pInstructions[unCurrentInstructionIndex].
                                    pOperandList;

                    // If operand list was allocated, free...
                    if(pOperandList)
                    {
                        // String literals must be freed...
                        for(usCurrentOperandIndex = 0;
                            usCurrentOperandIndex < OperandCount;
                            usCurrentOperandIndex++)
                        {
                            // String found...
                            if(pOperandList[usCurrentOperandIndex].OperandType
                                == OT_AVM_STRING)
                            {
                                // Free...
                                free(pOperandList[usCurrentOperandIndex].
                                     pszLiteralString);
                                pOperandList[usCurrentOperandIndex].
                                     pszLiteralString = NULL;
                            }
                        }

                        // Free the operand list itself...
                        free(pOperandList);
                    }
                }

                // Free instruction stream itself, if necessary...
                if(Scripts[hScript].InstructionStream.pInstructions)
                    free(Scripts[hScript].InstructionStream.pInstructions);

                // Function table, if necessary...
                if(Scripts[hScript].pFunctionTable)
                    free(Scripts[hScript].pFunctionTable);

                // Host function table, if necessary...
                if(Scripts[hScript].pHostFunctionTable)
                    free(Scripts[hScript].pHostFunctionTable);

            // Mark script as fully unloaded...
            memset(&Scripts[hScript], '\x0', sizeof(AVM_Script));

            // Invalidate script thread handle...
            hScript = (Script) -1;

            // Abort...
            return Reason;
        }

    // Cleanup...
    fclose(hScriptFile);

    // Set loaded flag...
    Scripts[hScript].bLoaded = true;

    /* Display statistics... (for debugging purposes only)
    DisplayStatistics(hScript);*/

    // Done...
    return Ok;
}

// Pass float parameter...
boolean VirtualMachine::PassFloatParameter(Script hScript, float fValue)
{
    // Variables...
    AVM_RuntimeValue    FloatParameter;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Initialize parameter...
    FloatParameter.OperandType      = OT_AVM_FLOAT;
    FloatParameter.fLiteralFloat    = fValue;

    // Try to push parameter onto the script's stack...
    try
    {
        // Push it...
        Push(hScript, FloatParameter);
    }

        // Failed...
        catch(SCRIPT_EXECUTION_EXCEPTION Exception)
        {
            // Let caller know it failed...
            return false;
        }

    // Done...
    return true;
}

// Pass integer parameter...
boolean VirtualMachine::PassIntegerParameter(Script hScript, int nValue)
{
    // Variables...
    AVM_RuntimeValue    IntegerParameter;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Initialize parameter...
    IntegerParameter.OperandType        = OT_AVM_INTEGER;
    IntegerParameter.nLiteralInteger    = nValue;

    // Try to push parameter onto the script's stack...
    try
    {
        // Push it...
        Push(hScript, IntegerParameter);
    }

        // Failed...
        catch(SCRIPT_EXECUTION_EXCEPTION Exception)
        {
            // Let caller know it failed...
            return false;
        }

    // Done...
    return true;
}

// Pass string parameter...
boolean VirtualMachine::PassStringParameter(Script hScript, char *pszValue)
{
    // Variables...
    AVM_RuntimeValue    StringParameter;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Initialize parameter...
    StringParameter.OperandType         = OT_AVM_STRING;
    StringParameter.pszLiteralString    = strdup(pszValue);

        // Failed...
        if(!StringParameter.pszLiteralString)
            return false;

    // Try to push parameter onto the script's stack...
    try
    {
        // Push it...
        Push(hScript, StringParameter);
    }

        // Failed...
        catch(SCRIPT_EXECUTION_EXCEPTION Exception)
        {
            // Let caller know it failed...
            return false;
        }

    // Done...
    return true;
}

// Pause a script for a certain duration...
boolean VirtualMachine::PauseScript(Script hScript, uint32 unDuration)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Trigger pause...
    Scripts[hScript].bPaused = true;
    Scripts[hScript].unPauseEndTime = GetSystemMilliSeconds() + unDuration;

    // Done...
    return true;
}

// Pop value off of the stack or throw execution exception...
inline VirtualMachine::AVM_RuntimeValue VirtualMachine::Pop(Script hScript)
{
    // Variables...
    AVM_RuntimeValue    PoppedValue;
    uint32              unNewTopIndex   = 0;

    // Check for stack underflow...
    if(Scripts[hScript].Stack.nTopIndex <= 0)
        throw SCRIPT_EXECUTION_EXCEPTION_STACK_UNDERFLOW;

    // Decrement top index...
    Scripts[hScript].Stack.nTopIndex--;

    // Get new top index...
    unNewTopIndex = Scripts[hScript].Stack.nTopIndex;

    // Top index + 1 is location of now popped off element...
    CopyValue(&PoppedValue, Scripts[hScript].Stack.pElements[unNewTopIndex]);

    // Return popped off value to caller...
    return PoppedValue;
}

// Push a stack frame onto the stack or throw execution exception...
inline void VirtualMachine::PushStackFrame(Script hScript, uint32 unSize)
{
    // Check for stack overflow...
    if(Scripts[hScript].Stack.nTopIndex + unSize >=
       Scripts[hScript].MainHeader.unStackSize - 1)
        throw SCRIPT_EXECUTION_EXCEPTION_STACK_OVERFLOW;

    // Stack must now accomodate nSize elements for new stack frame...
    Scripts[hScript].Stack.nTopIndex += unSize;

    // Shift frame index to match to top of the new stack frame...
    Scripts[hScript].Stack.unCurrentStackFrameTopIndex
        = Scripts[hScript].Stack.nTopIndex;
}

// Pop stack frame off of the stack or throw execution exception...
inline void VirtualMachine::PopStackFrame(Script hScript, uint32 unSize)
{
    // Stack underflow...
    if(Scripts[hScript].Stack.nTopIndex - unSize < 0)
        throw SCRIPT_EXECUTION_EXCEPTION_STACK_UNDERFLOW;

    // Shift stack top down...
    Scripts[hScript].Stack.nTopIndex -= unSize;

    /* Note: We will not modify the stack's frame pointer,
             unCurrentStackFrameTopIndex, because Call and Ret instructions
             will manually set it... */
}

// Push value onto the stack or throw execution exception...
inline void VirtualMachine::Push(Script hScript, AVM_RuntimeValue RuntimeValue)
{
    // Variables...
    int32   nTopIndex   = 0;

    // Stack overflow...
    if(Scripts[hScript].Stack.nTopIndex >=
       (int32) Scripts[hScript].MainHeader.unStackSize - 1)
        throw SCRIPT_EXECUTION_EXCEPTION_STACK_OVERFLOW;

    // Get the current top index...
    nTopIndex = Scripts[hScript].Stack.nTopIndex;

    // nTopIndex + 1 is array element of newly added stack element...
    CopyValue(&Scripts[hScript].Stack.pElements[nTopIndex], RuntimeValue);

    // Increment top index...
    Scripts[hScript].Stack.nTopIndex++;
}

// Register host provided function...
bool VirtualMachine::RegisterHostProvidedFunction(Script hThread,
    const char *pszName, HostProvidedFunction *pHostProvidedFunction)
{
    // Variables...
    uint32                      unCurrentHostProvidedFunction   = 0;

    // Find a vacant host function entry in the table...
    for(unCurrentHostProvidedFunction = 0;
        unCurrentHostProvidedFunction < MAXIMUM_HOST_PROVIDED_FUNCTIONS;
        unCurrentHostProvidedFunction++)
    {
        // Extract...
        AVM_HostProvidedFunction &HostFunctionEntry =
            HostProvidedFunctionTable[unCurrentHostProvidedFunction];

        // Found vacancy...
        if(!HostFunctionEntry.bLoaded)
        {
            // Verify parameters...
            if(!pszName || !pHostProvidedFunction ||
               strlen(pszName) >= sizeof(HostFunctionEntry.szName))
                return false;

            // Clear entry...
            memset(&HostFunctionEntry, 0, sizeof(AVM_HostProvidedFunction));

            // Configure the host provided function...
            HostFunctionEntry.hScriptVisibleTo = hThread;
            strcpy(HostFunctionEntry.szName, pszName);
            HostFunctionEntry.pEntryPoint      = pHostProvidedFunction;

            // Remember that this entry is loaded...
            HostFunctionEntry.bLoaded = true;

            // Done...
            return true;
        }
    }

    // Table is full...
    return false;

}

// Reset a script...
boolean VirtualMachine::ResetScript(Script hScript)
{
    // Variables...
    uint32  unMainIndex     = 0;
    uint32  unStackIndex    = 0;

    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Get Main() function index, if any...
    unMainIndex = Scripts[hScript].MainHeader.unMainIndex;

    // Main() function is present in script...
    if(unMainIndex != (uint32) -1)
    {
        // Initialize instruction pointer...
        Scripts[hScript].InstructionStream.unInstructionPointer =
            Scripts[hScript].pFunctionTable[unMainIndex].unEntryPoint;
    }

    // Reset stack...

        // Clear trackers...
        Scripts[hScript].Stack.nTopIndex                    = 0;
        Scripts[hScript].Stack.unCurrentStackFrameTopIndex  = 0;

        // Set entire stack to NULL...
        for(unStackIndex = 0;
            unStackIndex < Scripts[hScript].MainHeader.unStackSize;
            unStackIndex++)
        {
            // Clear element...
            memset(&Scripts[hScript].Stack.pElements[unStackIndex],
                   '\x0', sizeof(AVM_RuntimeValue));

            // Set operand type to NULL for no apparent reason...
            Scripts[hScript].Stack.pElements[unStackIndex].OperandType
                = OT_AVM_NULL;
        }

    // Reset paused timer...
    Scripts[hScript].bPaused = false;
    Scripts[hScript].unPauseEndTime = 0;

    // Allocate space for script's globals...
    PushStackFrame(hScript, Scripts[hScript].MainHeader.unGlobalDataSize);

    // If Main() is present, accomodate local data on stack plus one more for
    //  the function index (which is a dummy for Main() because it cannot
    //  return control to a calling function)...
    PushStackFrame(hScript, Scripts[hScript].pFunctionTable[unMainIndex].
                    unLocalDataSize + 1);

    // Done...
    return true;
}

// Resolve operand as float or throw error string...
inline float32 VirtualMachine::ResolveOperandAsFloat(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Get requested operand's runtime value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Return coerced float value...
    return CoerceValueToFloat(OperandValue);
}

// Resolve operand as an integer or throw error string...
inline int32 VirtualMachine::ResolveOperandAsInteger(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Get requested operand's runtime value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Return coerced integer value...
    return CoerceValueToInteger(OperandValue);
}

// Resolve operand as string or throw error string...
inline char *VirtualMachine::ResolveOperandAsString(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Get requested operand's runtime value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Return coerced string...
    return CoerceValueToString(OperandValue);
}

// Resolve an operand's stack index, whether absolute or relative or throw
//  error string...
inline int32 VirtualMachine::ResolveOperandStackIndex(uint8 OperandIndex)
{
    // Variables...
    uint32              unCurrentInstruction    = 0;
    AVM_RuntimeValue    OperandValue;
    int32               nBaseIndex              = 0;
    int32               nOffsetIndex            = 0;
    AVM_RuntimeValue    StackValue;

    // Get the current instruction's index...
    unCurrentInstruction = Scripts[hCurrentThread].InstructionStream.
                            unInstructionPointer;

    // Get requested operand's runtime value...
    OperandValue = Scripts[hCurrentThread].InstructionStream.
                pInstructions[unCurrentInstruction].pOperandList[OperandIndex];

    // Resolve stack index based on the type...
    switch(OperandValue.OperandType)
    {
        // Already an absolute stack index, needn't be resolved further...
        case OT_AVM_INDEX_STACK_ABSOLUTE:
            return OperandValue.nStackIndex[0];

        // Relative stack index, resolve...
        case OT_AVM_INDEX_STACK_RELATIVE:
        {
            // Fetch base index...
            nBaseIndex = OperandValue.nStackIndex[0];

            // Fetch offset index...
            nOffsetIndex = OperandValue.nStackIndex[1];

            // Get variable's value...
            StackValue = GetStackValue(hCurrentThread, nOffsetIndex);

            // Variables integer field + base index is absolute index...
            return nBaseIndex + StackValue.nLiteralInteger;
        }

        // Zero for everything else, but this should not happen...
        default:
            throw "cannot resolve stack index on non-stack index operand";
    }
}

// Resolves final type of operand and returns the resolved type or throw error
//  string...
inline uint8 VirtualMachine::ResolveOperandType(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Fetch operand...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Return the runtime value's type to caller...
    return OperandValue.OperandType;
}

// Resolve operand's value or throw error string...
inline VirtualMachine::AVM_RuntimeValue 
    VirtualMachine::ResolveOperandValue(uint8 OperandIndex)
{
    // Variables...
    uint32              unCurrentInstruction    = 0;
    AVM_RuntimeValue    OperandValue;
    int32               nAbsoluteStackIndex     = 0;

    // Get the current instruction's index...
    unCurrentInstruction = Scripts[hCurrentThread].InstructionStream.
                            unInstructionPointer;

    // Get requested operand's runtime value...
    OperandValue = Scripts[hCurrentThread].InstructionStream.
                pInstructions[unCurrentInstruction].pOperandList[OperandIndex];

    // What are we to return?
    switch(OperandValue.OperandType)
    {
        // Stack index, resolve...
        case OT_AVM_INDEX_STACK_ABSOLUTE:
        case OT_AVM_INDEX_STACK_RELATIVE:
        {
            // Calculate absolute stack index...
            nAbsoluteStackIndex = ResolveOperandStackIndex(OperandIndex);

            // Return it...
            return GetStackValue(hCurrentThread, nAbsoluteStackIndex);
        }

        // Register...
        case OT_AVM_REGISTER:
        {
            // Which one?
            switch(OperandValue.Register)
            {
                // First general purpose register...
                case REGISTER_AVM_T0:
                    return Scripts[hCurrentThread]._RegisterT0;
                
                // Second general purpose register...
                case REGISTER_AVM_T1:
                    return Scripts[hCurrentThread]._RegisterT1;

                // Return value...
                case REGISTER_AVM_RETURN:
                    return Scripts[hCurrentThread]._RegisterReturn;
            }
        }

        // Everything else is fine as is...
        default:
            return OperandValue;
    }
}

// Resolve operand as an instruction index or throw error string...
inline int32 
    VirtualMachine::ResolveOperandAsInstructionIndex(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Resolve operand value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Resolve operand value as an instruction index...
    return OperandValue.nInstructionIndex;
}

// Resolve operand as a function index or throw error string...
inline int32 VirtualMachine::ResolveOperandAsFunctionIndex(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Resolve operand value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Resolve operand value as a function index...
    return OperandValue.nFunctionIndex;
}

// Resolve operand as a host function index or throw error string...
int32 VirtualMachine::ResolveOperandAsHostFunctionIndex(uint8 OperandIndex)
{
    // Variables...
    AVM_RuntimeValue    OperandValue;

    // Resolve operand value...
    OperandValue = ResolveOperandValue(OperandIndex);

    // Return host name...
    return OperandValue.nHostFunctionIndex;
}

// Resolves operand and returns a pointer to it's runtime value or NULL if not
// applicable...
inline VirtualMachine::AVM_RuntimeValue *
    VirtualMachine::ResolveOperandAsPointer(uint8 OperandIndex)
{
    // Variables...
    uint32              unCurrentInstruction    = 0;
    int32               nStackIndex             = 0;

    // Return a pointer to wherever the operand is...
    switch(GetOperandType(OperandIndex))
    {
        // Operand is on the stack...
        case OT_AVM_INDEX_STACK_ABSOLUTE:
        case OT_AVM_INDEX_STACK_RELATIVE:
        {
            // Fetch stack index...
            nStackIndex = ResolveOperandStackIndex(OperandIndex);

            // Return location...
            return &Scripts[hCurrentThread].Stack.
                pElements[ResolveStackIndex(hCurrentThread, nStackIndex)];
        }

        // Register...
        case OT_AVM_REGISTER:
        {
            // Fetch the current instruction...
            unCurrentInstruction = Scripts[hCurrentThread].InstructionStream.
                                unInstructionPointer;

            // Which register do we want the address of?
            switch(Scripts[hCurrentThread].InstructionStream.
                    pInstructions[unCurrentInstruction].
                    pOperandList[OperandIndex].Register)
            {
                // First general purpose register value...
                case REGISTER_AVM_T0:
                    return &Scripts[hCurrentThread]._RegisterT0;
                
                // Second general purpose register value...
                case REGISTER_AVM_T1:
                    return &Scripts[hCurrentThread]._RegisterT1;
                
                // Return value...
                case REGISTER_AVM_RETURN:
                    return &Scripts[hCurrentThread]._RegisterReturn;
            }
        }

        // Anything else is inlined in the instruction stream...
        default:
            return NULL;
    }
}

// Return nothing from within host function...
void VirtualMachine::ReturnVoidFromHost(Script hScript, uint8 unParameters)
{
    // Clear off the parameters that were originally pushed onto the stack...
    Scripts[hScript].Stack.nTopIndex -= unParameters;
}

// Return an integer from within host function...
void VirtualMachine::ReturnIntegerFromHost(Script hScript, uint8 unParameters,
                                           int nReturnValue)
{
    // Clear off the parameters that were originally pushed onto the stack...
    Scripts[hScript].Stack.nTopIndex -= unParameters;

    // Store the return value in the return value register...
    Scripts[hScript]._RegisterReturn.OperandType        = OT_AVM_INTEGER;
    Scripts[hScript]._RegisterReturn.nLiteralInteger    = nReturnValue;
}

// Return a float from within host function...
void VirtualMachine::ReturnFloatFromHost(Script hScript, uint8 unParameters,
                                         float fReturnValue)
{
    // Clear off the parameters that were originally pushed onto the stack...
    Scripts[hScript].Stack.nTopIndex -= unParameters;

    // Store the return value in the return value register...
    Scripts[hScript]._RegisterReturn.OperandType    = OT_AVM_FLOAT;
    Scripts[hScript]._RegisterReturn.fLiteralFloat  = fReturnValue;
}

// Return a string from within a host function...
void VirtualMachine::ReturnStringFromHost(Script hScript, uint8 unParameters,
                                          char *pszReturnValue)
{
    // Variables...
    AVM_RuntimeValue    ReturnValue;

    // Clear off the parameters that were originally pushed onto the stack...
    Scripts[hScript].Stack.nTopIndex -= unParameters;

    // Prepare to store the return value...
    ReturnValue.OperandType         = OT_AVM_STRING;
    ReturnValue.pszLiteralString    = pszReturnValue;

    // Store the return value safely back into the return register...
    CopyValue(&Scripts[hScript]._RegisterReturn, ReturnValue);
}

// Run scripts for specified milliseconds, or 0 until all return...
boolean VirtualMachine::RunScripts(uint32 unDuration)
{
    // Variables...
    bool                bBreakExecution                     = false;
    uint32              unMainTimeSliceStartTime            = 0;
    uint32              unCurrentTime                       = 0;
    bool                bStillRunningSomething              = false;
    Script              hTempCurrentThread                  = 0;
    uint32              unCurrentInstructionPointer         = 0;
    uint16              usOperationCode                     = 0;
    AVM_RuntimeValue    DestinationOperand;
    AVM_RuntimeValue    SourceOperand;
    uint8               DestinationType                     = 0x00;
    char               *pszSource                           = NULL;
    char               *pszNew                              = NULL;
    uint32              unTargetIndex                       = 0;
    AVM_RuntimeValue    Operand0;
    AVM_RuntimeValue    Operand1;
    bool                bJump                               = false;
    uint32              unFunctionIndex                     = 0;
    AVM_RuntimeValue    CurrentFunctionIndex;
    Agni_Function       CurrentFunction;
    uint32              unFrameIndex                        = 0;
    AVM_RuntimeValue    ReturnAddress;
    char               *pszHostFunction                     = NULL;
    AVM_RuntimeValue    HostFunctionIndex;
    uint32              unCurrentHostProvidedFunctionIndex  = 0;
    uint32              unPauseDuration                          = 0;

    // Get the current time the main timeslice started...
    unMainTimeSliceStartTime = GetSystemMilliSeconds();

    // Enter instruction execution loop... (break conditions are nested)
    while(true)
    {
        // If all threads have terminated, then execution needn't continue...
        for(bStillRunningSomething = false, hTempCurrentThread = 0;
            hTempCurrentThread < MAXIMUM_THREADS;
            hTempCurrentThread++)
        {
            // This thread contains a loaded script that is running...
            if(Scripts[hTempCurrentThread].bLoaded &&
               Scripts[hTempCurrentThread].bExecuting)
                bStillRunningSomething = true;
        }

            // Nothing still running, so stop execution...
            if(!bStillRunningSomething)
                break;

        // Remember the current time...
        unCurrentTime = GetSystemMilliSeconds();

        // Machine is configured for multithreading, perform context switch...
        if(CurrentThreadingMode == THREADING_MODE_MULTIPLE)
        {
            // Current thread's time slice has either full elapsed or terminated,
            //  switch to the next valid thread...
            if((unCurrentTime > unCurrentThreadActivationTime +
                                Scripts[hCurrentThread].unThreadTimeSlice) ||
               !Scripts[hCurrentThread].bExecuting)
            {
                // Find the next thread... (there must be at least one)
                while(true)
                {
                    // Seek to next thread...
                    hCurrentThread++;

                        // Reached the end, loop back to the beginning...
                        if(hCurrentThread >= MAXIMUM_THREADS)
                            hCurrentThread = 0;

                    // Check thread to see if anything is loaded and executing...
                    if(Scripts[hCurrentThread].bLoaded &&
                       Scripts[hCurrentThread].bExecuting)
                        break;
                }

                // This thread takes over beginning now...
                unCurrentThreadActivationTime = GetSystemMilliSeconds();
            }
        }

        // Is the script paused?...
        if(Scripts[hCurrentThread].bPaused)
        {
            // If the pause time has elapsed, then unpause script...
            if(unCurrentTime >= Scripts[hCurrentThread].unPauseEndTime)
                Scripts[hCurrentThread].bPaused = false;

            // Otherwise, let the thread idle for this execution cycle...
            else
                continue;
        }

        // Remember the current instruction pointer to compare with later...
        unCurrentInstructionPointer = Scripts[hCurrentThread].InstructionStream.
                                        unInstructionPointer;

        // Extract the current operation code...
        usOperationCode = Scripts[hCurrentThread].InstructionStream.
                            pInstructions[unCurrentInstructionPointer].
                            usOperationCode;

        // This is where the magic happens - the execution of an operation...
        switch(usOperationCode)
        {
            // Binary operations have a lot of redundant logic. We will
            //  specialize for each later where necessary with a switch-case...
            case INSTRUCTION_AVM_MOV:
            case INSTRUCTION_AVM_ADD:
            case INSTRUCTION_AVM_SUB:
            case INSTRUCTION_AVM_MUL:
            case INSTRUCTION_AVM_DIV:
            case INSTRUCTION_AVM_MOD:
            case INSTRUCTION_AVM_EXP:
            case INSTRUCTION_AVM_AND:
            case INSTRUCTION_AVM_OR:
            case INSTRUCTION_AVM_XOR:
            case INSTRUCTION_AVM_SHL:
            case INSTRUCTION_AVM_SHR:
            {
                // Extract destination and source operand...
                DestinationOperand  = ResolveOperandValue(0);
                SourceOperand       = ResolveOperandValue(1);

                // Perform binary operation...
                switch(usOperationCode)
                {
                    // Move...
                    case INSTRUCTION_AVM_MOV:
                    {
                        // Source and destination same, so nothing to move...
                        if(ResolveOperandAsPointer(0) == ResolveOperandAsPointer(1))
                            break;

                        // Copy the source operand into the destination...
                        CopyValue(&DestinationOperand, SourceOperand);

                        // Done...
                        break;
                    }

                    // Add...
                    case INSTRUCTION_AVM_ADD:
                    {
                        // Is destination an integer?
                        if(SourceOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger +=
                            ResolveOperandAsInteger(1);

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat +=
                            ResolveOperandAsFloat(1);

                        // Done...
                        break;
                    }

                    // Subtract...
                    case INSTRUCTION_AVM_SUB:
                    {
                        // Is destination an integer?
                        if(SourceOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger -=
                            ResolveOperandAsInteger(1);

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat -=
                            ResolveOperandAsFloat(1);

                        // Done...
                        break;
                    }

                    // Multiply...
                    case INSTRUCTION_AVM_MUL:
                    {
                        // Is destination an integer?
                        if(SourceOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger *=
                            ResolveOperandAsInteger(1);

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat *=
                            ResolveOperandAsFloat(1);

                        // Done...
                        break;
                    }

                    // Divide...
                    case INSTRUCTION_AVM_DIV:
                    {
                        // Is destination an integer?
                        if(SourceOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger /=
                            ResolveOperandAsInteger(1);

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat /=
                            ResolveOperandAsFloat(1);

                        // Done...
                        break;
                    }

                    // Modulus...
                    case INSTRUCTION_AVM_MOD:
                    {
                        // Modulus defined only for integral values...
                        DestinationOperand.nLiteralInteger %=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }

                    // Exponentiate...
                    case INSTRUCTION_AVM_EXP:
                    {
                        // Is destination an integer?
                        if(SourceOperand.OperandType == OT_AVM_INTEGER)
                        {
                            // Compute...
                            DestinationOperand.nLiteralInteger =
                                (int) pow(DestinationOperand.nLiteralInteger,
                                        ResolveOperandAsInteger(1));
                        }

                        // Assume, then, that it is a float...
                        else
                        {
                            // Compute...
                            DestinationOperand.fLiteralFloat =
                                (float) pow(DestinationOperand.fLiteralFloat,
                                    ResolveOperandAsFloat(1));
                        }

                        // Done...
                        break;
                    }

                    // Bitwise AND...
                    case INSTRUCTION_AVM_AND:
                    {
                        // Only defined for integral values...
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger &=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }

                    // Bitwise OR...
                    case INSTRUCTION_AVM_OR:
                    {
                        // Only defined for integral values...
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger |=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }

                    // Bitwise XOR...
                    case INSTRUCTION_AVM_XOR:
                    {
                        // Only defined for integral values...
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger ^=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }

                    // Shift left...
                    case INSTRUCTION_AVM_SHL:
                    {
                        // Only defined for integral values...
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger <<=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }

                    // Shift right...
                    case INSTRUCTION_AVM_SHR:
                    {
                        // Only defined for integral values...
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger >>=
                            ResolveOperandAsInteger(1);

                        // Done...
                        break;
                    }
                }

                // Now store the result...
               *ResolveOperandAsPointer(0) = DestinationOperand;

                // Done performing binary operation...
                break;
            }

            // Perform unary operation...
            case INSTRUCTION_AVM_NEG:
            case INSTRUCTION_AVM_NOT:
            case INSTRUCTION_AVM_INC:
            case INSTRUCTION_AVM_DEC:
            {
                // Remember the destination operand type...
                DestinationType = GetOperandType(0);

                // Extract the destination value...
                DestinationOperand = ResolveOperandValue(0);

                // Implement unary operators...
                switch(usOperationCode)
                {
                    // Negate...
                    case INSTRUCTION_AVM_NEG:
                    {
                        // Is this an integral value...
                        if(DestinationType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger =
                           -DestinationOperand.nLiteralInteger;

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat =
                           -DestinationOperand.fLiteralFloat;

                        // Done...
                        break;
                    }

                    // Not...
                    case INSTRUCTION_AVM_NOT:
                    {
                        // Defined only for integral values...
                        if(DestinationType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger =
                           ~DestinationOperand.nLiteralInteger;

                        // Done...
                        break;
                    }

                    // Increment...
                    case INSTRUCTION_AVM_INC:
                    {
                        // Increment an integer?
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger++;

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat++;

                        // Done...
                        break;
                    }

                    // Decrement...
                    case INSTRUCTION_AVM_DEC:
                    {
                        // Increment an integer?
                        if(DestinationOperand.OperandType == OT_AVM_INTEGER)
                            DestinationOperand.nLiteralInteger--;

                        // Assume, then, that it is a float...
                        else
                            DestinationOperand.fLiteralFloat--;

                        // Done...
                        break;
                    }
                }

                // Store result of unary operation...
               *ResolveOperandAsPointer(0) = DestinationOperand;

                // Done with unary operation...
                break;
            }

            // String concatenation...
            case INSTRUCTION_AVM_CONCAT:
            {
                // Extract destination operand...
                DestinationOperand = ResolveOperandValue(0);

                    // Destination is not a string, ignore it...
                    if(DestinationOperand.OperandType != OT_AVM_STRING)
                        break;

                // Extract a pointer to the source string....
                pszSource = ResolveOperandAsString(1);

                // Allocate storage space for the new string...
                pszNew = (char *)
                    malloc(strlen(DestinationOperand.pszLiteralString) +
                           strlen(pszSource) + 1);

                // Build new string...
                strcpy(pszNew, DestinationOperand.pszLiteralString);
                strcat(pszNew, pszSource);

                // Replace old string with new one...
                free(DestinationOperand.pszLiteralString);
                DestinationOperand.pszLiteralString = pszNew;

                // Shove the final value back into the instruction stream...
               *ResolveOperandAsPointer(0) = DestinationOperand;

                // Done...
                break;
            }

            // Get character...
            case INSTRUCTION_AVM_GETCHAR:
            {
                // Extract destination operand...
                DestinationOperand = ResolveOperandValue(0);

                // Extract source string...
                pszSource = ResolveOperandAsString(1);

                // Check if the destination operand is already a string...
                if(DestinationOperand.OperandType == OT_AVM_STRING)
                {
                    // It's too small for a character...
                    if(strlen(DestinationOperand.pszLiteralString) < 1)
                    {
                        // Resize and extract it...
                        free(DestinationOperand.pszLiteralString);
                        pszNew = (char *) malloc(2);
                    }

                    // Appropriate size, extract...
                    else
                        pszNew = DestinationOperand.pszLiteralString;
                }

                // Destination operand is not a string...
                else
                {
                    // Create a string and remember operand's new type...
                    pszNew = (char *) malloc(2);
                    DestinationOperand.OperandType = OT_AVM_STRING;
                }

                // Copy the selected character into the source now...
                pszNew[0] = pszSource[ResolveOperandAsInteger(2)];
                pszNew[1] = '\x0';

                // Replace old destination string with newly computed...
                DestinationOperand.pszLiteralString = pszNew;

                // Finally plug computed value back into instruction stream...
               *ResolveOperandAsPointer(0) = DestinationOperand;

                // Done...
                break;
            }

            // Set character...
            case INSTRUCTION_AVM_SETCHAR:
            {
                // Destination is not a string, ignore it...
                if(ResolveOperandType(0) != OT_AVM_STRING)
                    break;

                // Extract source string...
                pszSource = ResolveOperandAsString(2);

                // Set the ith character in the destination to source...
                ResolveOperandAsPointer(0)->
                    pszLiteralString[ResolveOperandAsInteger(1)] = pszSource[0];

                // Done...
                break;
            }

            // Jump...
            case INSTRUCTION_AVM_JMP:
            {
                // Extract new address...
                unTargetIndex = ResolveOperandAsInstructionIndex(0);

                // Shift instruction pointer to new address...
                Scripts[hCurrentThread].InstructionStream.unInstructionPointer
                    = unTargetIndex;

                // Done...
                break;
            }

            // Conditional jumps...
            case INSTRUCTION_AVM_JE:
            case INSTRUCTION_AVM_JNE:
            case INSTRUCTION_AVM_JG:
            case INSTRUCTION_AVM_JL:
            case INSTRUCTION_AVM_JGE:
            case INSTRUCTION_AVM_JLE:
            {
                // Extract that which we are to compare...
                Operand0 = ResolveOperandValue(0);
                Operand1 = ResolveOperandValue(1);

                // Extact target instruction index...
                unTargetIndex = ResolveOperandAsInstructionIndex(2);

                // Perform appropriate comparison now and jump if necessary...
                bJump = false;
                switch(usOperationCode)
                {
                    // Jump if equal...
                    case INSTRUCTION_AVM_JE:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger ==
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat ==
                                         Operand1.fLiteralFloat);
                                break;

                            // String...
                            case OT_AVM_STRING:

                                // Compare...
                                bJump = (strcmp(Operand0.pszLiteralString,
                                                Operand1.pszLiteralString)
                                         == 0);
                                break;
                        }

                        // Done...
                        break;
                    }

                    // Jump if not equal...
                    case INSTRUCTION_AVM_JNE:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger !=
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat !=
                                         Operand1.fLiteralFloat);
                                break;

                            // String...
                            case OT_AVM_STRING:

                                // Compare...
                                bJump = (strcmp(Operand0.pszLiteralString,
                                                Operand1.pszLiteralString)
                                         != 0);
                                break;
                        }

                        // Done...
                        break;
                    }

                    // Jump if greater...
                    case INSTRUCTION_AVM_JG:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger >
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat >
                                         Operand1.fLiteralFloat);
                                break;
                        }

                        // Done...
                        break;
                    }

                    // Jump if less...
                    case INSTRUCTION_AVM_JL:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger <
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat <
                                         Operand1.fLiteralFloat);
                                break;
                        }

                        // Done...
                        break;
                    }

                    // Jump if greater than or equal...
                    case INSTRUCTION_AVM_JGE:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger >=
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat >=
                                         Operand1.fLiteralFloat);
                                break;
                        }

                        // Done...
                        break;
                    }

                    // Jump if less than or equal...
                    case INSTRUCTION_AVM_JLE:
                    {
                        // Handle each operand type...
                        switch(Operand0.OperandType)
                        {
                            // Integer...
                            case OT_AVM_INTEGER:

                                // Compare...
                                bJump = (Operand0.nLiteralInteger <=
                                         Operand1.nLiteralInteger);
                                break;

                            // Float...
                            case OT_AVM_FLOAT:

                                // Compare...
                                bJump = (Operand0.fLiteralFloat <=
                                         Operand1.fLiteralFloat);
                                break;
                        }

                        // Done...
                        break;
                    }
                }

                // Now make the jump only if the comparision was true...
                if(bJump)
                {
                    // Extract target instruction address...
                    unTargetIndex = ResolveOperandAsInstructionIndex(2);

                    // Shift instruction pointer to new address...
                    Scripts[hCurrentThread].InstructionStream.
                        unInstructionPointer = unTargetIndex;
                }

                // Done...
                break;
            }

            // Push value onto the stack...
            case INSTRUCTION_AVM_PUSH:
            {
                // Extract source value operand...
                SourceOperand = ResolveOperandValue(0);

                // Push value onto stack...
                Push(hCurrentThread, SourceOperand);

                // Done...
                break;
            }

            // Pop value off of the stack...
            case INSTRUCTION_AVM_POP:
            {
                // Pop top most value on stack into destination...
               *ResolveOperandAsPointer(0) = Pop(hCurrentThread);

                // Done...
                break;
            }

            // Call a function...
            case INSTRUCTION_AVM_CALL:
            {
                // Extract function index before we lose the operand...
                unFunctionIndex = ResolveOperandAsFunctionIndex(0);

                // Instruction pointer increments to point after call...
                Scripts[hCurrentThread].InstructionStream.
                    unInstructionPointer++;

                // Invoke script function...
                CallFunctionImplementation(hCurrentThread, unFunctionIndex);

                // Done...
                break;
            }

            // Return from a function...
            case INSTRUCTION_AVM_RET:
            {
                // Gather some information on returning function...

                    // Function index is in the top of the stack...
                    CurrentFunctionIndex = Pop(hCurrentThread);

                    // Bottom of stack found, so terminate script...
                    if(CurrentFunctionIndex.OperandType == OT_AVM_STACK_BASE_MARKER)
                        bBreakExecution = true;

                    // Extract frame index from function structure...
                    CurrentFunction =
                        GetFunction(hCurrentThread,
                                    CurrentFunctionIndex.nFunctionIndex);
                    unFrameIndex = CurrentFunctionIndex.nStackIndex[1];

                // Extract the return address which is the next element under
                //  the local data...
                ReturnAddress =
                    GetStackValue(hCurrentThread,
                                  Scripts[hCurrentThread].Stack.nTopIndex -
                                    (CurrentFunction.unLocalDataSize + 1));

                // Remove the stack frame of the returning function...
                PopStackFrame(hCurrentThread, CurrentFunction.unStackFrameSize);

                // Restore the previous stack frame's index...
                Scripts[hCurrentThread].Stack.unCurrentStackFrameTopIndex =
                    unFrameIndex;

                // Finally jump to the return address...
                Scripts[hCurrentThread].InstructionStream.unInstructionPointer
                    = ReturnAddress.nInstructionIndex;

                // Done...
                break;
            }

            // Call a host function...
            case INSTRUCTION_AVM_CALLHOST:
            {
                // Extract the desire host function index...
                HostFunctionIndex = ResolveOperandValue(0);

                // Get the actual name of the desired host function...
                pszHostFunction =
                    GetHostFunction(HostFunctionIndex.nHostFunctionIndex);

                // Search through the provided host function table until we
                //  find the host provided function and that this thread is
                //  privy to it or it is a global host function...
                for(unCurrentHostProvidedFunctionIndex = 0;
                    unCurrentHostProvidedFunctionIndex <
                                            MAXIMUM_HOST_PROVIDED_FUNCTIONS;
                    unCurrentHostProvidedFunctionIndex++)
                {
                    // Extract host provided function...
                    AVM_HostProvidedFunction HostProvidedFunction =
                        HostProvidedFunctionTable
                                        [unCurrentHostProvidedFunctionIndex];

                    // This host function table entry is not loaded, skip...
                    if(!HostProvidedFunction.bLoaded)
                        continue;

                    // Possible match...
                    if(strcasecmp(HostProvidedFunction.szName, pszHostFunction) == 0)
                    {
                        // If the host function is registered to this thread or
                        //  it is a global host function, then match found...
                        if((HostProvidedFunction.hScriptVisibleTo
                                == hCurrentThread) ||
                           (HostProvidedFunction.hScriptVisibleTo
                                == (uint32) GLOBAL_HOST_FUNCTION))
                        {
                            // Match found, invoke host function...
                            HostProvidedFunction.pEntryPoint(hCurrentThread);

                            // Done...
                            break;
                        }
                    }
                }

                // Done...
                break;
            }

            // Rand instruction...
            case INSTRUCTION_AVM_RAND:
            {
                // Variables...
                //static  uint32  unPreviousRandomNumber  = 17489;
                static  uint32  unPreviousRandomNumber
                                                = GetSystemMilliSeconds();

                // Generate random number...
                unPreviousRandomNumber =
                    (25173 * unPreviousRandomNumber + 13849);

                // Store ranged random number in destination...
                DestinationOperand.OperandType      = OT_AVM_INTEGER;
                DestinationOperand.nLiteralInteger
                    = unPreviousRandomNumber % (1 + ResolveOperandAsInteger(1));

                // Store result...
               *ResolveOperandAsPointer(0) = DestinationOperand;

                // Done...
                break;
            }

            // Pause instruction...
            case INSTRUCTION_AVM_PAUSE:
            {
                // Extract the duration time...
                unPauseDuration = ResolveOperandAsInteger(0);

                // Calculate and store the pause ending time...
                Scripts[hCurrentThread].unPauseEndTime =
                                            unCurrentTime + unPauseDuration;

                // Flag the script as paused...
                Scripts[hCurrentThread].bPaused = true;

                // Done...
                break;
            }

            // Exit instruction...
            case INSTRUCTION_AVM_EXIT:
            {
                /* TODO (Kip#3#): Figure out what we want to do with the exit
                                  code in operand zero here. */

                // Flag the script as no longer running...
                Scripts[hCurrentThread].bExecuting = false;

                // Done...
                break;
            }
        }

        // If the instruction pointer wasn't changed by an instruction,
        //  increment it. CALL, for example, increments automatically...
        if(Scripts[hCurrentThread].InstructionStream.unInstructionPointer ==
           unCurrentInstructionPointer)
            Scripts[hCurrentThread].InstructionStream.unInstructionPointer++;

        // We are not running indefinetely...
        if(unDuration != (unsigned) THREAD_PRIORITY_INFINITE)
        {
            // Check if main timeslice has expired...
            if(unCurrentTime > (unMainTimeSliceStartTime + unDuration))
                break;
        }

        // If the script has terminated, then exit the execution loop....
        if(bBreakExecution)
            break;
    }

    // Done...
    return true;
}

// Set stack value...
inline void VirtualMachine::SetStackValue(Script hScript, int32 nIndex,
                                          AVM_RuntimeValue RuntimeValue)
{
    // Set element at specified index...
    Scripts[hScript].Stack.pElements[ResolveStackIndex(hScript, nIndex)]
        = RuntimeValue;
}

// Start the execution of a script...
boolean VirtualMachine::StartScript(Script hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Trigger execution flag...
    Scripts[hScript].bExecuting = true;

    // Set current thread to this one...
    hCurrentThread = hScript;

    // Set execution activation time...
    unCurrentThreadActivationTime = GetSystemMilliSeconds();

    // Done...
    return true;
}

// Stop the execution of a script...
boolean VirtualMachine::StopScript(Script hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Stop execution...
    Scripts[hScript].bExecuting = false;

    // Done...
    return true;
}

// Unload script...
boolean VirtualMachine::UnloadScript(Script &hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Instruction stream, if necessary...
    for(uint32 unCurrentInstructionIndex = 0;
        unCurrentInstructionIndex <
            Scripts[hScript].InstructionStreamHeader.unSize &&
        Scripts[hScript].InstructionStream.pInstructions;
        unCurrentInstructionIndex++)
    {
        // Variables...
        uint8               OperandCount    = 0;
        AVM_RuntimeValue   *pOperandList    = NULL;

        // Extract number of operands...
        OperandCount = Scripts[hScript].InstructionStream.
                        pInstructions[unCurrentInstructionIndex].
                        OperandCount;

        // Extract operand list...
        pOperandList = Scripts[hScript].InstructionStream.
                        pInstructions[unCurrentInstructionIndex].
                        pOperandList;

        // String literal operands must be freed...
        for(uint16 usCurrentOperandIndex = 0;
            usCurrentOperandIndex < OperandCount;
            usCurrentOperandIndex++)
        {
            // String found...
            if(pOperandList[usCurrentOperandIndex].OperandType
                == OT_AVM_STRING)
            {
                // Free...
                free(pOperandList[usCurrentOperandIndex].
                     pszLiteralString);
                pOperandList[usCurrentOperandIndex].
                     pszLiteralString = NULL;
            }
        }

        // Free the operand list itself...
        free(pOperandList);
        pOperandList = NULL;
    }

    // Instruction stream itself, if necessary...
    if(Scripts[hScript].InstructionStreamHeader.unSize)
    {
        // Free it...
        free(Scripts[hScript].InstructionStream.pInstructions);
        Scripts[hScript].InstructionStream.pInstructions = NULL;
    }

    // Free any string literals allocated on the runtime stack...
    for(uint32 unCurrentStackIndex = 0;
        unCurrentStackIndex < Scripts[hScript].MainHeader.unStackSize;
        unCurrentStackIndex++)
    {
        // Found one...
        if(Scripts[hScript].Stack.pElements[unCurrentStackIndex].OperandType
            == OT_AVM_STRING)
        {
            // Free...
            free(Scripts[hScript].Stack.pElements[unCurrentStackIndex].
                pszLiteralString);
            Scripts[hScript].Stack.pElements[unCurrentStackIndex].
                pszLiteralString = NULL;
        }
    }

    // Free the runtime stack itself...
    free(Scripts[hScript].Stack.pElements);
    Scripts[hScript].Stack.pElements = NULL;

    // Function table, if necessary...
    if(Scripts[hScript].FunctionTableHeader.unSize)
    {
        // Free it...
        free(Scripts[hScript].pFunctionTable);
        Scripts[hScript].pFunctionTable = NULL;
    }

    // Host function table, if necessary...
    if(Scripts[hScript].HostFunctionTableHeader.unSize)
    {
        // Free it...
        free(Scripts[hScript].pHostFunctionTable);
        Scripts[hScript].pHostFunctionTable = NULL;
    }

    // Clear header...
    memset(&Scripts[hScript], '\x0', sizeof(AVM_Script));

    // Mark as unloaded, for no particular reason...
    Scripts[hScript].bLoaded = false;

    // Invalidate user's script thread handle to make debugging on their part
    //  easier...
    hScript = (Script) -1;

    // Done...
    return true;
}

// Unpause a script...
boolean VirtualMachine::UnPauseScript(Script hScript)
{
    // Check handle...
    if(!IsValidThread(hScript))
        return false;

    // Reset pause flag...
    Scripts[hScript].bPaused = false;

    // Done...
    return true;
}

// Check version...
bool VirtualMachine::VersionSafe(uint8 AvailableMajor, uint8 AvailableMinor,
                                 uint8 RequestedMajor, uint8 RequestedMinor)
{
    // Available major version is newer than requested, ok...
    if(AvailableMajor > RequestedMajor)
        return true;

    // Available major version is the same as requested...
    else if(AvailableMajor == RequestedMajor)
    {
        // Available minor is sufficient though...
        if(AvailableMinor >= RequestedMinor)
            return true;

        // Available minor is insufficient...
        else
            return false;
    }

    // Our major version is older than requested, not ok...
    else
        return false;
}

// Deconstructor shuts down runtime enviroment...
VirtualMachine::~VirtualMachine()
{
    // Unload all loaded scripts, if any...
    for(Script hCurrentScript = 0; hCurrentScript < MAXIMUM_THREADS;
        hCurrentScript++)
    {
        // Unload this script, if it is loaded...
        if(IsValidThread(hCurrentScript))
            UnloadScript(hCurrentScript);
    }

    // Free host name, if necessary...
    if(pszHostName)
        free(pszHostName);
}
