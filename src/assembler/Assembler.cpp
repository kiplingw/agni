/*
  Name:         Assembler.cpp (implementation)
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Routines to produce an Agni executable from an Agni assembly
                listing...
*/

// Includes...
#include "Assembler.h"
#include <getopt.h>
#include <iostream>
#include <cassert>

// Using the Agni assembler namespace...
using namespace Agni;

// Constructor...
Assembler::Assembler(Parameters &_UserParameters)
    :   UserParameters(_UserParameters),
        ppszListing(NULL),
        unListingLines(0),
        usInstructionCount(0),
        pInstructionStream(NULL),
        LexerState(LEXER_NO_STRING),
        unLexerIndex0(0),
        unLexerIndex1(0),
        unLexerLine(0),
        unLexerCurrentToken(TOKEN_INVALID),
        unOutputBufferAllocated(0),
        pOutputBuffer(NULL),
        unTempCheckSum(0)
{
    // Variables...
    unsigned int unInstructionIndex = 0;

    // Clear out instruction stream header...
    memset(&InstructionStreamHeader, 0, sizeof(Agni_InstructionStreamHeader));
    
    // Initialize our tables...
    List_Initialize(&StringTable);
    List_Initialize(&FunctionTable);
    List_Initialize(&SymbolTable);
    List_Initialize(&LabelTable);
    List_Initialize(&HostFunctionTable);

    // Clear the current lexeme...
    memset(szCurrentLexeme, '\x0', sizeof(szCurrentLexeme));
    
    // Clear the main executable header...
    memset(&MainHeader, 0, sizeof(Agni_MainHeader));

    // Initialize instruction set table...

        // Main...

            // Mov... (destination, source)
            unInstructionIndex = AddInstruction("Mov", INSTRUCTION_AVM_MOV, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

        // Arithmetic...

            // Add... (destination, source)
            unInstructionIndex = AddInstruction("Add", INSTRUCTION_AVM_ADD, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Sub... (destination, source)
            unInstructionIndex = AddInstruction("Sub", INSTRUCTION_AVM_SUB, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Mul... (destination, source)
            unInstructionIndex = AddInstruction("Mul", INSTRUCTION_AVM_MUL, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Div... (destination, source)
            unInstructionIndex = AddInstruction("Div", INSTRUCTION_AVM_DIV, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Mod... (destination, source)
            unInstructionIndex = AddInstruction("Mod", INSTRUCTION_AVM_MOD, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Exp... (destination, source)
            unInstructionIndex = AddInstruction("Exp", INSTRUCTION_AVM_EXP, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Neg... (destination)
            unInstructionIndex = AddInstruction("Neg", INSTRUCTION_AVM_NEG, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Inc... (destination)
            unInstructionIndex = AddInstruction("Inc", INSTRUCTION_AVM_INC, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Dec... (destination)
            unInstructionIndex = AddInstruction("Dec", INSTRUCTION_AVM_DEC, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

        // Bitwise...

            // And... (destination, source)
            unInstructionIndex = AddInstruction("And", INSTRUCTION_AVM_AND, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Or... (destination, source)
            unInstructionIndex = AddInstruction("Or", INSTRUCTION_AVM_OR, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // XOr... (destination, source)
            unInstructionIndex = AddInstruction("XOr", INSTRUCTION_AVM_XOR, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Not... (destination)
            unInstructionIndex = AddInstruction("Not", INSTRUCTION_AVM_NOT, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // ShL... (destination, source)
            unInstructionIndex = AddInstruction("ShL", INSTRUCTION_AVM_SHL, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // ShR... (destination, source)
            unInstructionIndex = AddInstruction("ShR", INSTRUCTION_AVM_SHR, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

        // String manipulation...

            // Concat... (string0, string1)
            unInstructionIndex = AddInstruction("Concat",
                                                INSTRUCTION_AVM_CONCAT, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER |
                           OTD_STRING);

            // GetChar... (destination, source, index)
            unInstructionIndex = AddInstruction("GetChar",
                                                INSTRUCTION_AVM_GETCHAR, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER |
                           OTD_STRING);
            SetOperandType(unInstructionIndex, 2,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER |
                           OTD_INTEGER);

            // SetChar... (destination, index, source)
            unInstructionIndex = AddInstruction("SetChar",
                                                INSTRUCTION_AVM_SETCHAR, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER |
                           OTD_INTEGER);
            SetOperandType(unInstructionIndex, 2,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER |
                           OTD_STRING);

        // Branching...

            // Jmp... (label)
            unInstructionIndex = AddInstruction("Jmp", INSTRUCTION_AVM_JMP, 1);
            SetOperandType(unInstructionIndex, 0, OTD_LINE_LABEL);

            // JE... (op0, op1, label)
            unInstructionIndex = AddInstruction("JE", INSTRUCTION_AVM_JE, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

            // JNE... (op0, op1, label)
            unInstructionIndex = AddInstruction("JNE", INSTRUCTION_AVM_JNE, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

            // JG... (op0, op1, label)
            unInstructionIndex = AddInstruction("JG", INSTRUCTION_AVM_JG, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

            // JL... (op0, op1, label)
            unInstructionIndex = AddInstruction("JL", INSTRUCTION_AVM_JL, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

            // JGE... (op0, op1, label)
            unInstructionIndex = AddInstruction("JGE", INSTRUCTION_AVM_JGE, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

            // JLE... (op0, op1, label)
            unInstructionIndex = AddInstruction("JLE", INSTRUCTION_AVM_JLE, 3);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 2, OTD_LINE_LABEL);

        // Stack interface...

            // Push... (source)
            unInstructionIndex = AddInstruction("Push", INSTRUCTION_AVM_PUSH, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Pop... (destination)
            unInstructionIndex = AddInstruction("Pop", INSTRUCTION_AVM_POP, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

        // Function interface...

            // Call... (functionname)
            unInstructionIndex = AddInstruction("Call", INSTRUCTION_AVM_CALL, 1);
            SetOperandType(unInstructionIndex, 0, OTD_FUNCTION_NAME);

            // Ret...
            unInstructionIndex = AddInstruction("Ret", INSTRUCTION_AVM_RET, 0);

            // CallHost... (hostfunctionname)
            unInstructionIndex = AddInstruction("CallHost",
                                                INSTRUCTION_AVM_CALLHOST, 1);
            SetOperandType(unInstructionIndex, 0, OTD_HOST_FUNCTION_NAME);

        // Miscellaneous...

            // Rand... (destination, range)
            unInstructionIndex = AddInstruction("Rand", INSTRUCTION_AVM_RAND, 2);
            SetOperandType(unInstructionIndex, 0,
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);
            SetOperandType(unInstructionIndex, 1,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Pause... (duration)
            unInstructionIndex = AddInstruction("Pause", INSTRUCTION_AVM_PAUSE, 1);
            SetOperandType(unInstructionIndex, 0,
                           OTD_INTEGER |
                           OTD_FLOAT |
                           OTD_STRING |
                           OTD_MEMORY_REFERENCE |
                           OTD_REGISTER);

            // Exit... (return code using return value register, like Ret)
            unInstructionIndex = AddInstruction("Exit", INSTRUCTION_AVM_EXIT, 0);

    // Be verbose...
    ErrorVerbose("loaded assembler settings");
    ErrorVerbose("settings->assembler: `%s'", 
        UserParameters.GetProcessName().c_str());
    ErrorVerbose("settings->input: `%s'", 
        UserParameters.GetInputFile().c_str());
    ErrorVerbose("settings->optimization: %d", 
        UserParameters.GetOptimizationLevel());
    ErrorVerbose("settings->output: `%s'", 
        UserParameters.GetOutputFile().c_str());
    ErrorVerbose("settings->verbose: %s", 
        UserParameters.ShouldBeVerbose() ? "true" : "false");
    ErrorVerbose("initialized instruction set table with %d instructions",
        GetInstructionSetSize());
}

// Add function and return index, or -1 on error...
int32 Assembler::AddFunction(const char *pszName, uint32 unEntryPoint)
{
    // Variables...
    AA_FunctionNodeData    *pNewFunctionNodeData    = NULL;
    uint32                  unIndex                 = 0;

    // Function already exists...
    if(GetFunctionByName(pszName))
        return -1;

    // Create a new function node data...
    pNewFunctionNodeData = (AA_FunctionNodeData *)
                                        malloc(sizeof(AA_FunctionNodeData));

    // Initialize only the function name and entry point for now...
    memset(pNewFunctionNodeData, 0, sizeof(AA_FunctionNodeData));
    strcpy(pNewFunctionNodeData->szName, pszName);
    pNewFunctionNodeData->unEntryPoint = unEntryPoint;

    // Add to function table and remember table index...
    unIndex = List_Add(&FunctionTable, pNewFunctionNodeData);

    // Also store function table's index in function node's data...
    pNewFunctionNodeData->unIndex = unIndex;

    // Return function table's index...
    return pNewFunctionNodeData->unIndex;
}

// Add instruction and return index, or -1 on error...
int32 Assembler::AddInstruction(const char *pszMnemonic,
                                uint16 usOperationCode,
                                uint8 OperandCount)
{
    // Ensure we haven't run out of instruction indices...
    assert(usInstructionCount < 
           (sizeof(InstructionTable) / sizeof(AA_InstructionLookup)));

    // Store mnemonic...
    strncpy(InstructionTable[usInstructionCount].szMnemonic, pszMnemonic,
            sizeof(InstructionTable[usInstructionCount].szMnemonic) - 1);

    // Store operation code and operand count...
    InstructionTable[usInstructionCount].usOperationCode  = usOperationCode;
    InstructionTable[usInstructionCount].OperandCount     = OperandCount;

    // This operation has operands...
    if(OperandCount > 0)
    {
        // Allocate sufficient storage space...
        InstructionTable[usInstructionCount].pOperandList
            = (AA_OperandType *) malloc(OperandCount * sizeof(AA_OperandType));

        // Clear...
        memset(InstructionTable[usInstructionCount].pOperandList, 0,
               OperandCount * sizeof(AA_OperandType));
    }

    // No operands...
    else
        InstructionTable[usInstructionCount].pOperandList = NULL;

    // Increment instruction index for the next instruction...
    usInstructionCount++;

    // Return the instruction index of this current instruction...
    return (usInstructionCount - 1);
}

// Add a label and return index, or -1 on error...
int32 Assembler::AddLabel(const char *pszIdentifier,
                          uint32 unTargetIndex, uint32 unFunctionIndex)
{
    // Variables...
    AA_LabelNodeData   *pNewLabelNodeData   = NULL;
    uint32              unIndex             = 0;

    // Label already exists, abort...
    if(GetLabelByIdentifier(pszIdentifier, unFunctionIndex))
        return -1;

    // Create new data for a label node...
    pNewLabelNodeData = (AA_LabelNodeData *) malloc(sizeof(AA_LabelNodeData));

    // Initialize...
    memset(pNewLabelNodeData, 0, sizeof(AA_LabelNodeData));
    strcpy(pNewLabelNodeData->szIdentifier, pszIdentifier);
    pNewLabelNodeData->unTargetIndex    = unTargetIndex;
    pNewLabelNodeData->unFunctionIndex  = unFunctionIndex;

    // Add new label to label table and remember index...
    unIndex = List_Add(&LabelTable, pNewLabelNodeData);

    // Store index inside label node's data...
    pNewLabelNodeData->unIndex = unIndex;

    // Return the new label's index...
    return pNewLabelNodeData->unIndex;
}

// Add string to linked list and return index...
uint32 Assembler::AddString(AA_LinkedList *pList, const char *pszString)
{
    // Variables...
    AA_LinkedListNode  *pNode   = NULL;
    uint32              unIndex = 0;
    char               *pszTemp = NULL;

    // Check linked list to see if string already exists...
    for(unIndex = 0, pNode = pList->pHead;
        unIndex < pList->unNodes;
        unIndex++)
    {
        // Match found, return string table index...
        if(strcmp(pszString, (char *) pNode->pData) == 0)
            return unIndex;

        // No match, try next node...
        pNode = pNode->pNext;
    }

    // String is new, add...

        // Allocate storage space...
        pszTemp = (char *) malloc(strlen(pszString) + 1);

        // Initialize...
        strcpy(pszTemp, pszString);

        // Add and return index...
        return List_Add(pList, pszTemp);
}

// Add symbol, or -1 on error...
int32 Assembler::AddSymbol(const char *pszIdentifier, uint32 unSize,
                           int32 nStackIndex, uint32 unFunctionIndex)
{
    // Variables...
    AA_SymbolNodeData  *pNewSymbolNodeData  = NULL;
    uint32              unIndex             = 0;

    // Symbol already exists, abort...
    if(GetSymbolByIdentifier(pszIdentifier, unFunctionIndex))
        return -1;

    // Allocate new symbol...
    pNewSymbolNodeData = (AA_SymbolNodeData *)
                                            malloc(sizeof(AA_SymbolNodeData));

    // Initialize...
    strcpy(pNewSymbolNodeData->szIdentifier, pszIdentifier);
    pNewSymbolNodeData->unSize          = unSize;
    pNewSymbolNodeData->nStackIndex     = nStackIndex;
    pNewSymbolNodeData->unFunctionIndex = unFunctionIndex;

    // Add symbol to symbol table and remember its index...
    unIndex = List_Add(&SymbolTable, pNewSymbolNodeData);

    // Remember symbol node's index inside of symbol node's data...
    pNewSymbolNodeData->unIndex = unIndex;

    // Return the new index...
    return unIndex;
}

// Assemble listing...
boolean Assembler::Assemble()
{
    // Variables...
    int32                   nTemp                           = 0;
    boolean                 bIsCurrentFunctionActive        = false;
    AA_FunctionNodeData    *pCurrentFunctionNodeData        = NULL;
    uint32                  unCurrentFunctionIndex          = 0;
    char                    szCurrentFunctionName[1024]     = {0};
    uint16                  usCurrentFunctionParameterCount = 0;
    uint32                  unCurrentFunctionLocalDataSize  = 0;
    AA_InstructionLookup    CurrentInstruction;
    uint32                  unIndex                         = 0;
    uint32                  unCurrentInstructionIndex       = 0;

    // Verify parameters...

        // No input file specified...
        if(UserParameters.GetInputFile().empty())
        {
            // Abort...
            ErrorGeneral("no input file");
            return false;
        }

        // No output file specified...
        if(UserParameters.GetOutputFile().empty())
        {
            // Abort...
            ErrorGeneral("no output file specified");
            return false;
        }

        // Input and output files are the same...
        if(strcasecmp(UserParameters.GetInputFile().c_str(), 
                      UserParameters.GetOutputFile().c_str()) == 0)
        {
            // Abort...
            ErrorGeneral("input and output are the same location");
            return false;
        }

    // Reset the lexer...
    ResetLexer();

    // Reset main header...

        // Most fields are fine as null bytes...
        memset(&MainHeader, 0, sizeof(Agni_MainHeader));

        // Host string index has all bits enabled when unset...
        MainHeader.unHostStringIndex = (uint32) -1;

        // Main function index has all bits enabled when unset...
        MainHeader.unMainIndex = (uint32) -1;

        // Thread priority has all bits enabled when unset...
        MainHeader.ThreadPriorityType   = (uint8) -1;
        MainHeader.unThreadPriorityUser = (uint32) -1;

    // Try to assemble...
    try
    {
        // Load input...
        LoadInput();

        // Be verbose...
        ErrorVerbose("scanning for directives and calculating instruction "
                     "stream size");

        // Begin pre-processor parsing until done...
        while(GetNextToken() != TOKEN_END)
        {
            // Parse token...
            switch(unLexerCurrentToken)
            {
                // SetHost directive...
                case TOKEN_DIRECTIVE_SETHOST:
                {
                    // We are not in the global scope where we should be...
                    if(bIsCurrentFunctionActive)
                        throw "directive not in global scope";

                    // Already been found...
                    if(MainHeader.unHostStringIndex != 0xffffffff)
                        throw "SetHost already set";

                    // First a quote...
                    if(GetNextToken() != TOKEN_QUOTE)
                        throw '\"';

                    // Next a string describing host...
                    if(GetNextToken() != TOKEN_STRING)
                        throw "invalid host";

                    // Store intended host string...
                    MainHeader.unHostStringIndex
                                = AddString(&StringTable, GetCurrentLexeme());

                    // Next should be a quote...
                    if(GetNextToken() != TOKEN_QUOTE)
                        throw '\"';

                    // Next token should be a comma...
                    if(GetNextToken() != TOKEN_COMMA)
                        throw ',';

                    // Next token should be an integer...
                    if(GetNextToken() != TOKEN_INTEGER)
                        throw "expected integer for second parameter";

                    // Store it...
                    MainHeader.ucHostMajorVersion = atoi(GetCurrentLexeme());

                    // Next token should be a comma...
                    if(GetNextToken() != TOKEN_COMMA)
                        throw ',';

                    // Next token should be an integer...
                    if(GetNextToken() != TOKEN_INTEGER)
                        throw "expected integer for third parameter";

                    // Store it...
                    MainHeader.ucHostMinorVersion = atoi(GetCurrentLexeme());

                    // Done...
                    break;
                }

                // SetStackSize directive...
                case TOKEN_DIRECTIVE_SETSTACKSIZE:
                {
                    // We are not in the global scope where we should be...
                    if(bIsCurrentFunctionActive)
                        throw "directive not in global scope";

                    // Already been found...
                    if(MainHeader.unStackSize != 0)
                        throw "SetStackSize already set";

                    // Parameter should be an integer...
                    if(GetNextToken() != TOKEN_INTEGER)
                        throw "invalid stack size";

                    // Convert parameter from lexeme to integer...
                    nTemp = atoi(GetCurrentLexeme());

                        // Bad parameter...
                        if(nTemp < 0)
                            throw "invalid directive parameter";

                    // Store, with zero meaning default size...
                    MainHeader.unStackSize = (nTemp == 0) ? 0xffffffff
                                                          : nTemp;

                    // Done...
                    break;
                }

                // SetThreadPriority directive...
                case TOKEN_DIRECTIVE_SETTHREADPRIORITY:
                {
                    // We are not in the global scope where we should be...
                    if(bIsCurrentFunctionActive)
                        throw "directive not in global scope";

                    // Already been found...
                    if(MainHeader.ThreadPriorityType != 0xff)
                        throw "SetThreadPriority already set";

                    // Next token can be Low, Medium, High, or n ms...
                    switch(GetNextToken())
                    {
                        // Preset priority...
                        case TOKEN_IDENTIFIER:

                            // Low...
                            if(strcasecmp(GetCurrentLexeme(), "Low") == 0)
                                MainHeader.ThreadPriorityType
                                    = THREAD_PRIORITY_LOW;

                            // Medium...
                            else if(strcasecmp(GetCurrentLexeme(), "Medium") == 0)
                                MainHeader.ThreadPriorityType
                                    = THREAD_PRIORITY_MEDIUM;

                            // High...
                            else if(strcasecmp(GetCurrentLexeme(), "High") == 0)
                                MainHeader.ThreadPriorityType
                                    = THREAD_PRIORITY_HIGH;

                            // Invalid...
                            else
                                throw "invalid directive parameter";

                            // Done...
                            break;

                        // User specified priority...
                        case TOKEN_INTEGER:

                            // Set user priority flag...
                            MainHeader.ThreadPriorityType
                                = THREAD_PRIORITY_USER;

                            // Check for valid time...
                            if(atoi(GetCurrentLexeme()) <= 0)
                                throw "thread priority must be greater than 0 ms";

                            // Store...
                            MainHeader.unThreadPriorityUser
                                = atoi(GetCurrentLexeme());

                            // Next token should be ms...
                            if(GetNextToken() != TOKEN_IDENTIFIER)
                                throw "invalid directive parameter";

                            // Check...
                            if(strcasecmp(GetCurrentLexeme(), "ms") != 0)
                                throw "invalid directive parameter";

                            // Done...
                            break;

                        // Invalid...
                        default:
                            throw "invalid directive parameter";

                    }

                    // Done...
                    break;
                }

                // Function declaration...
                case TOKEN_FUNC:
                {
                    // We are already in a function, abort...
                    if(bIsCurrentFunctionActive)
                        throw "function nested inside of previous function";

                    // Check to make sure function name follows...
                    if(GetNextToken() != TOKEN_IDENTIFIER)
                        throw "expected function identifier";

                    // Function must be less that 128 characters in length...
                    if(strlen(GetCurrentLexeme()) > 128)
                        throw "identifier must be < 128 characters";

                    // Add to function table and check for error...
                    nTemp = AddFunction(GetCurrentLexeme(),
                                        InstructionStreamHeader.unSize);

                        // Already defined...
                        if(nTemp == -1)
                            throw "function redefined";

                    // This is the Main function, store index...
                    if(strcasecmp(GetCurrentLexeme(), 
                                  AGNI_DEFAULT_ENTRY_FUNCTION) == 0)
                        MainHeader.unMainIndex = nTemp;

                    // Initialize function tracking...
                    bIsCurrentFunctionActive        = true;
                    strcpy(szCurrentFunctionName, GetCurrentLexeme());
                    unCurrentFunctionIndex          = nTemp;
                    usCurrentFunctionParameterCount = 0;
                    unCurrentFunctionLocalDataSize  = 0;

                    // Seek passed new lines...
                    while(GetNextToken() == TOKEN_NEW_LINE);

                    // Make sure current token is an open brace...
                    if(GetCurrentToken() != TOKEN_OPEN_BRACE)
                        throw '{';

                    // All functions automatically appended with Ret, so
                    //  increment required size of the instruction stream...
                    InstructionStreamHeader.unSize++;

                    // Done...
                    break;
                }

                // Closed brace...
                case TOKEN_CLOSE_BRACE:
                {
                    // We are not in a function, abort...
                    if(!bIsCurrentFunctionActive)
                        throw "unexpected '}'";

                    // Set the fields we've collected...
                    SetFunctionInfo(szCurrentFunctionName,
                                    usCurrentFunctionParameterCount,
                                    unCurrentFunctionLocalDataSize);

                    // Done with this function...
                    bIsCurrentFunctionActive = false;

                    // Done...
                    break;
                }

                // Variable declaration...
                case TOKEN_VAR:
                {
                    // Variables...
                    uint32          unSize              = 0;
                    int32           nStackIndex         = 0;
                    char            szIdentifier[1024]  = {0};

                    // Verify identifer presence...
                    if(GetNextToken() != TOKEN_IDENTIFIER)
                        throw "identifier expected";

                    // Store identifier...
                    strcpy(szIdentifier, GetCurrentLexeme());

                    // For now, all variables one unit in size...
                    unSize = 1;

                    // Is this an array?
                    if(GetLookAheadCharacter() == '[')
                    {
                        // Consume open bracket and check for error...
                        if(GetNextToken() != TOKEN_OPEN_BRACKET)
                            throw '[';

                        // Get size...

                            // Verify presence...
                            if(GetNextToken() != TOKEN_INTEGER)
                                throw "expected array size";

                            // Bad size...
                            if(atoi(GetCurrentLexeme()) <= 1)
                                throw "invalid array size";

                            // Save...
                            unSize = atoi(GetCurrentLexeme());

                        // Check for closing bracket...
                        if(GetNextToken() != TOKEN_CLOSE_BRACKET)
                            throw ']';
                    }

                    // Calculate stack index...

                        // Local variable. Local data size + 2, subtracted from
                        //  zero...
                        if(bIsCurrentFunctionActive)
                            nStackIndex = -(unCurrentFunctionLocalDataSize + 2);

                        // Global variable. Equal to current global data size...
                        else
                            nStackIndex = MainHeader.unGlobalDataSize;

                    // Add to symbol table and check for error...
                    if(AddSymbol(szIdentifier, unSize, nStackIndex,
                                 unCurrentFunctionIndex) == -1)
                        throw "identifier redefined";

                    // Incremement total data size...

                        // Local variable, so dealing with local data...
                        if(bIsCurrentFunctionActive)
                            unCurrentFunctionLocalDataSize += unSize;

                        // Global variable, so dealing with global data...
                        else
                            MainHeader.unGlobalDataSize += unSize;

                    // Done...
                    break;
                }

                // Parameter declaration...
                case TOKEN_PARAM:
                {
                    // We are not in a function, abort...
                    if(!bIsCurrentFunctionActive)
                        throw "parameter out of context";

                    // Ensure we are not in main...
                    if(strcasecmp(szCurrentFunctionName, 
                                  AGNI_DEFAULT_ENTRY_FUNCTION) == 0)
                        throw "Main cannot accept parameters";

                    // Verify identifier presence...
                    if(GetNextToken() != TOKEN_IDENTIFIER)
                        throw "identifier expected";

                    // Remember that we have found one more parameter...
                    usCurrentFunctionParameterCount++;

                    // Done...
                    break;
                }

                // Line label...
                case TOKEN_IDENTIFIER:
                {
                    // Ensure this is a line label...
                    if(GetLookAheadCharacter() != ':')
                        throw "invalid instruction";

                    // We must be in a function...
                    if(!bIsCurrentFunctionActive)
                        throw "label out of context";

                    // Add the label to the label table and check for error...
                    if(AddLabel(GetCurrentLexeme(),
                                InstructionStreamHeader.unSize - 1,
                                unCurrentFunctionIndex) == -1)
                        throw "label redefined";

                    // Done...
                    break;
                }

                // Instruction...
                case TOKEN_INSTRUCTION:
                {
                    // We are not in a function, abort...
                    if(!bIsCurrentFunctionActive)
                        throw "instruction out of context";

                    // Remember we will need to allocate one more instruction...
                    InstructionStreamHeader.unSize++;

                    // Done...
                    break;
                }

                // Some other token...
                default:
                {
                    // Not a line break, generate error...
                    if(GetCurrentToken() != TOKEN_NEW_LINE)
                        throw "invalid input";

                    // Done...
                    break;
                }
            }

            // Seek to the next line, unless we've reached the end of file...
            if(!SeekToNextLine())
                break;
        }

        // Handle uninvoked directives...

            // Verify application name present...
            if(MainHeader.unHostStringIndex == 0xffffffff)
                ErrorWarning("no host information was specified");

            // SetStackSize was not manually set, use default...
            if(MainHeader.unStackSize == 0)
                MainHeader.unStackSize = 0xffffffff;

            // SetThreadPriority was not manually set...
            if(MainHeader.ThreadPriorityType == (uint8) -1)
            {
                // Use default...
                MainHeader.ThreadPriorityType   = THREAD_PRIORITY_LOW;
                MainHeader.unThreadPriorityUser = 0;

                // Issue warning...
                ErrorWarning("no thread priority was specified, using low");
            }

        // Prepare instruction stream...

            // Be verbose...
            ErrorVerbose("allocating instruction stream");

            // Nothing to allocate...
            if(InstructionStreamHeader.unSize <= 1)
                throw "no instructions";

            // Allocate instruction stream buffer...
            pInstructionStream = (AA_Instruction *)
                malloc(InstructionStreamHeader.unSize * sizeof(AA_Instruction));

                // Failed...
                if(!pInstructionStream)
                    throw "unable to allocate storage for instruction stream";

                // Clear...
                memset(pInstructionStream, 0,
                       InstructionStreamHeader.unSize * sizeof(AA_Instruction));

            // Clear each instruction's operand list for no apparent reason...
            for(unIndex = 0; unIndex < InstructionStreamHeader.unSize;
                unIndex++)
                pInstructionStream[unIndex].pOperandList = NULL;

            // Reset current instruction index...
            unCurrentInstructionIndex = 0;

        // Be verbose...
        ErrorVerbose("assembling instruction stream");

        // Reset lexer...
        ResetLexer();

        // Perform second stage assembler parsing...
        while(GetNextToken() != TOKEN_END)
        {
            // Parse token...
            switch(unLexerCurrentToken)
            {
                // Function... (already validated in first pass)
                case TOKEN_FUNC:
                {
                    // Read the identifier...
                    GetNextToken();

                    // Get this function's information...
                    pCurrentFunctionNodeData
                                    = GetFunctionByName(GetCurrentLexeme());

                        // Failed...
                        if(!pCurrentFunctionNodeData)
                            throw "internal error (function node lookup)";

                    // Enable function tracking...
                    bIsCurrentFunctionActive = true;

                    // We will begin counting parameters at zero...
                    usCurrentFunctionParameterCount = 0;

                    // Save the function's index...
                    unCurrentFunctionIndex = pCurrentFunctionNodeData->unIndex;

                    // Seek passed white space until opening brace found...
                    while(GetNextToken() == TOKEN_NEW_LINE);

                    // Ready to parse function...
                    break;
                }

                // Closing brace...
                case TOKEN_CLOSE_BRACE:
                {
                    // Disable function tracking because function has ended...
                    bIsCurrentFunctionActive = false;

                    // The function we just finished with was Main(), append
                    //  an Exit instruction...
                    if(strcasecmp(pCurrentFunctionNodeData->szName, 
                                  AGNI_DEFAULT_ENTRY_FUNCTION) == 0)
                    {
                        // Store operation code...
                        pInstructionStream[unCurrentInstructionIndex].usOperationCode
                            = INSTRUCTION_AVM_EXIT;

                        // Store zero operands...
                        pInstructionStream[unCurrentInstructionIndex].OperandCount
                            = 0;

                        // Store no operand data...
                        pInstructionStream[unCurrentInstructionIndex].pOperandList
                            = NULL;
                    }

                    // Regular function, append Ret instruction...
                    else
                    {
                        // Store instruction code...
                        pInstructionStream[unCurrentInstructionIndex].usOperationCode
                            = INSTRUCTION_AVM_RET;

                        // Store zero operands...
                        pInstructionStream[unCurrentInstructionIndex].OperandCount
                            = 0;

                        // Store no operand data...
                        pInstructionStream[unCurrentInstructionIndex].pOperandList
                            = NULL;
                    }

                    // Completed appending appropriate instruction, seek to
                    //  next instruction storage location...
                    unCurrentInstructionIndex++;

                    // Done...
                    break;
                }

                // Parameter...
                case TOKEN_PARAM:
                {
                    // Variables...
                    int32 nStackIndex = 0;

                    // Get the parameter name and check for error...
                    if(GetNextToken() != TOKEN_IDENTIFIER)
                        throw "expected identifier";

                    // Calculate parameter's stack index...
                    nStackIndex = -(pCurrentFunctionNodeData->unLocalDataSize
                                    + 2 + (usCurrentFunctionParameterCount + 1));

                    // Add parameter to symbol table and check for error...
                    if(AddSymbol(GetCurrentLexeme(), 1, nStackIndex,
                                 unCurrentFunctionIndex) == -1)
                        throw "identifier redefined";

                    // Done with parameter, remember...
                    usCurrentFunctionParameterCount++;

                    // Done...
                    break;
                }

                // Instruction...
                case TOKEN_INSTRUCTION:
                {
                    // Variables...
                    AA_Operand     *pOperands               = NULL;
                    AA_OperandType  SupportedOperandType    = 0;
                    Token           InitialOperandToken     = 0;
                    int32           nCurrentOperandIndex    = 0;

                    // Get the instruction's information, check for error...
                    if(!GetInstructionByMnemonic(GetCurrentLexeme(),
                                                 &CurrentInstruction))
                        throw "unknown instruction";

                    // Write operation code to the instruction stream...
                    pInstructionStream[unCurrentInstructionIndex].usOperationCode
                        = CurrentInstruction.usOperationCode;

                    // Write operand count to the instruction stream...
                    pInstructionStream[unCurrentInstructionIndex].OperandCount
                        = CurrentInstruction.OperandCount;

                    // Instruction has operands...
                    if(CurrentInstruction.OperandCount)
                    {
                        // Allocate storage space for the operand list, if
                        //  necessary...
                        pOperands = (AA_Operand *)
                            malloc(CurrentInstruction.OperandCount *
                                   sizeof(AA_Operand));

                            // Failed...
                            if(!pOperands)
                                throw "unable to allocate sufficient storage "
                                      "memory for operand list";

                            // Clear...
                            memset(pOperands, 0, CurrentInstruction.OperandCount *
                                                 sizeof(AA_Operand));
                    }

                    // Instruction has no operands, so no need for list...
                    else
                        pOperands = NULL;

                    // Assemble each operand...
                    for(nCurrentOperandIndex = 0;
                        nCurrentOperandIndex < CurrentInstruction.OperandCount;
                        nCurrentOperandIndex++)
                    {
                        // Extract supported operand types...
                        SupportedOperandType
                                = CurrentInstruction.pOperandList[nCurrentOperandIndex];

                        // Store the instruction's operand...
                        InitialOperandToken = GetNextToken();

                        // Process operand...
                        switch(InitialOperandToken)
                        {
                            // Integer literal...
                            case TOKEN_INTEGER:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_INTEGER))
                                    throw "invalid operand";

                                // Store operand type...
                                pOperands[nCurrentOperandIndex].OperandType
                                            = OT_AVM_INTEGER;

                                // Store operand data...
                                pOperands[nCurrentOperandIndex].nLiteralInteger
                                            = atoi(GetCurrentLexeme());

                                // Done...
                                break;
                            }

                            // Floating-point literal...
                            case TOKEN_FLOAT:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_FLOAT))
                                    throw "invalid operand";

                                // Store operand type...
                                pOperands[nCurrentOperandIndex].OperandType
                                            = OT_AVM_FLOAT;

                                // Store operand data...
                                pOperands[nCurrentOperandIndex].fLiteralFloat
                                            = (float32) atof(GetCurrentLexeme());

                                // Done...
                                break;
                            }

                            // String literal...
                            case TOKEN_QUOTE:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_STRING))
                                    throw "invalid operand";

                                // Seek to next token and parse the operand...
                                switch(GetNextToken())
                                {
                                    // Another quote, so empty string...
                                    case TOKEN_QUOTE:
                                    {
                                        // Set type as integer...
                                        pOperands[nCurrentOperandIndex].OperandType
                                            = OT_AVM_INTEGER;

                                        // Store as an integer of zero value...
                                        pOperands[nCurrentOperandIndex].nLiteralInteger
                                            = 0;

                                        // Done...
                                        break;
                                    }

                                    // Normal string...
                                    case TOKEN_STRING:
                                    {
                                        // Variables...
                                        const char     *pszString       = NULL;
                                        uint32          unStringIndex   = 0;

                                        // Fetch string...
                                        pszString = GetCurrentLexeme();

                                        // Add string to string table...
                                        unStringIndex =
                                            AddString(&StringTable, pszString);

                                        // Closing double quote must follow...
                                        if(GetNextToken() != TOKEN_QUOTE)
                                            throw '\\';

                                        // Store operand type...
                                        pOperands[nCurrentOperandIndex].OperandType
                                            = OT_AVM_INDEX_STRING;

                                        // Store operand data...
                                        pOperands[nCurrentOperandIndex].nStringTableIndex
                                            = unStringIndex;

                                        // Done...
                                        break;
                                    }

                                    // Invalid string...
                                    default:
                                        throw "invalid string";
                                }

                                // Done...
                                break;
                            }

                            // First general purpose register...
                            case TOKEN_REGISTER_T0:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_REGISTER))
                                    throw "invalid operand";

                                // Store operand type...
                                pOperands[nCurrentOperandIndex].OperandType
                                    = OT_AVM_REGISTER;

                                // Store operand data...
                                pOperands[nCurrentOperandIndex].Register
                                    = REGISTER_AVM_T0;

                                // Done...
                                break;
                            }
                            
                            // Second general purpose register...
                            case TOKEN_REGISTER_T1:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_REGISTER))
                                    throw "invalid operand";

                                // Store operand type...
                                pOperands[nCurrentOperandIndex].OperandType
                                    = OT_AVM_REGISTER;

                                // Store operand data...
                                pOperands[nCurrentOperandIndex].Register
                                    = REGISTER_AVM_T1;

                                // Done...
                                break;
                            }
                            
                            // Return value register...
                            case TOKEN_REGISTER_RETURN:
                            {
                                // Invalid operand...
                                if(!(SupportedOperandType & OTD_REGISTER))
                                    throw "invalid operand";

                                // Store operand type...
                                pOperands[nCurrentOperandIndex].OperandType
                                    = OT_AVM_REGISTER;

                                // Store operand data...
                                pOperands[nCurrentOperandIndex].Register
                                    = REGISTER_AVM_RETURN;

                                // Done...
                                break;
                            }

                            // Any identifier, such as a variable, array
                            //  indices, line label, function names, or host
                            //  function name...
                            case TOKEN_IDENTIFIER:
                            {
                                // Variables...
                                char        szIdentifier[1024]  = {0};
                                int32       nBaseIndex          = 0;

                                // Operand accepts a memory reference, parse...
                                if(SupportedOperandType &
                                   OTD_MEMORY_REFERENCE)
                                {
                                    // Remember the identifier...
                                    GetCurrentLexeme(szIdentifier);

                                    // Ensure variable / array declared...
                                    if(!GetSymbolByIdentifier(szIdentifier,
                                                              unCurrentFunctionIndex))
                                        throw "undefined memory reference";

                                    // Get identifier's absolute / relative
                                    //  stack index...
                                    nBaseIndex
                                        = GetStackIndexByIdentifier(szIdentifier,
                                                                    unCurrentFunctionIndex);

                                    // This is a single identifer...
                                    if(GetLookAheadCharacter() != '[')
                                    {
                                        // Make sure this isn't suppose to be
                                        //  an array...
                                        if(GetSizeByIdentifier(szIdentifier,
                                                               unCurrentFunctionIndex) > 1)
                                            throw "array not indexed";

                                        // Store instruction's operand type...
                                        pOperands[nCurrentOperandIndex].OperandType
                                            = OT_AVM_INDEX_STACK_ABSOLUTE;

                                        /* Stack index must be absolute...
                                        if(nBaseIndex < 0)
                                            throw "???";*/

                                        // Store operand absolute index now...
                                        pOperands[nCurrentOperandIndex].nStackIndex[0]
                                            = nBaseIndex;
                                    }

                                    // This is an array...
                                    else
                                    {
                                        // Variables...
                                        Token   Token           = 0;
                                        int32   nOffsetIndex    = 0;

                                        // Verify identifier is an array...
                                        if(GetSizeByIdentifier(szIdentifier,
                                                               unCurrentFunctionIndex) == 1)
                                            throw "invalid array";

                                        // Verify open brack present...
                                        if(GetNextToken() != TOKEN_OPEN_BRACKET)
                                            throw '[';

                                        // Next token is index...
                                        Token = GetNextToken();

                                        // Normal integer index...
                                        if(Token == TOKEN_INTEGER)
                                        {
                                            // Get offset...
                                            nOffsetIndex = atoi(GetCurrentLexeme());

                                                // Invalid...
                                                if(nOffsetIndex < 0)
                                                    throw "invalid array index";

                                            // Store operand type...
                                            pOperands[nCurrentOperandIndex].OperandType
                                                = OT_AVM_INDEX_STACK_ABSOLUTE;

                                            // Store absolute stack index...
                                            pOperands[nCurrentOperandIndex].nStackIndex[0]
                                                = nBaseIndex + nOffsetIndex;
                                        }

                                        // Variable index...
                                        else if(Token == TOKEN_IDENTIFIER)
                                        {
                                            // Variables...
                                            char szIndexIdentifier[1024] = {0};

                                            // Save the index identifier...
                                            GetCurrentLexeme(szIndexIdentifier);

                                            // Ensure identifier exists...
                                            if(!GetSymbolByIdentifier(szIndexIdentifier,
                                                                      unCurrentFunctionIndex))
                                                throw "undefined variable used as index";

                                            // Ensure it is a single variable...
                                            if(GetSizeByIdentifier(szIndexIdentifier,
                                                                   unCurrentFunctionIndex) != 1)
                                                throw "invalid array index";

                                            // Get the variables stack index...
                                            nOffsetIndex
                                                = GetStackIndexByIdentifier(szIndexIdentifier,
                                                                            unCurrentFunctionIndex);

                                            // Store operand type...
                                            pOperands[nCurrentOperandIndex].OperandType
                                                = OT_AVM_INDEX_STACK_RELATIVE;

                                            // Store operand data...
                                            pOperands[nCurrentOperandIndex].nStackIndex[0]
                                                = nBaseIndex;
                                            pOperands[nCurrentOperandIndex].nStackIndex[1]
                                                = nOffsetIndex;
                                        }

                                        // Random index, abort...
                                        else
                                            throw "invalid array index";

                                        // Ensure array closing bracket there...
                                        if(GetNextToken() != TOKEN_CLOSE_BRACKET)
                                            throw ']';
                                    }
                                }

                                // Parse a line label...
                                if(SupportedOperandType & OTD_LINE_LABEL)
                                {
                                    // Variables...
                                    AA_LabelNodeData  *pLabelNodeData   = NULL;

                                    // Get label information...
                                    pLabelNodeData =
                                        GetLabelByIdentifier(GetCurrentLexeme(),
                                                             unCurrentFunctionIndex);

                                        // Failed...
                                        if(!pLabelNodeData)
                                            throw "undefined line label";

                                    // Store operand type...
                                    pOperands[nCurrentOperandIndex].OperandType
                                        = OT_AVM_INDEX_INSTRUCTION;

                                    // Store operand data...
                                    pOperands[nCurrentOperandIndex].nInstructionIndex
                                        = pLabelNodeData->unTargetIndex;
                                }

                                // Parse a function name...
                                if(SupportedOperandType & OTD_FUNCTION_NAME)
                                {
                                    // Variables...
                                    AA_FunctionNodeData *pFunctionNodeData = NULL;

                                    // Get function information...
                                    pFunctionNodeData
                                        = GetFunctionByName(GetCurrentLexeme());

                                        // Failed...
                                        if(!pFunctionNodeData)
                                            throw "undefined function";

                                    // Store operand type...
                                    pOperands[nCurrentOperandIndex].OperandType
                                        = OT_AVM_INDEX_FUNCTION;

                                    // Store operand data...
                                    pOperands[nCurrentOperandIndex].nFunctionIndex
                                        = pFunctionNodeData->unIndex;
                                }

                                // Parse a host function call...
                                if(SupportedOperandType & OTD_HOST_FUNCTION_NAME)
                                {
                                    // Variables...
                                    const char *pszHostFunctionName = NULL;
                                    int32       nStringIndex        = 0;

                                    // Get host function name...
                                    pszHostFunctionName = GetCurrentLexeme();

                                    // Add or get host function name in table...
                                    nStringIndex = AddString(&HostFunctionTable,
                                                             pszHostFunctionName);

                                    // Store operand type...
                                    pOperands[nCurrentOperandIndex].OperandType
                                        = OT_AVM_INDEX_FUNCTION_HOST;

                                    // Store operand data...
                                    pOperands[nCurrentOperandIndex].nHostFunctionIndex
                                        = nStringIndex;
                                }

                                // Done parsing identifier...
                                break;
                            }

                            // Invalid operand, abort...
                            default:
                                throw "unknown operand type";
                        }

                        // This is not the last operand...
                        if(nCurrentOperandIndex <
                           (CurrentInstruction.OperandCount - 1))
                        {
                            // A comma should follow...
                            if(GetNextToken() != TOKEN_COMMA)
                                throw ',';
                        }
                    }

                    // Ensure no extraneous data after last operand...
                    if(GetNextToken() != TOKEN_NEW_LINE)
                        throw "invalid input";

                    // Store the operand list into the instruction stream,
                    //  if any...
                    pInstructionStream[unCurrentInstructionIndex].pOperandList
                        = pOperands;

                    // Done with current instruction, seek to next...
                    unCurrentInstructionIndex++;
                    break;
                }
            }

            // Seek to next line, but stop if there are no more...
            if(!SeekToNextLine())
                break;
        }

        // Write executable to disk, catch errors...
        WriteExecutable();
    }

        // Failed because of a listing error...
        catch(const char *pszReason)
        {
            // Generate listing error...
            ErrorListing(pszReason);

            // Abort...
            return false;
        }

    // Be verbose...
    ErrorVerbose("assembly complete");

    // Display statistics, if verbose mode enabled...
    if(UserParameters.ShouldBeVerbose())
        DisplayStatistics();

    // Done...
    return true;
}

// Acknowledge bit in calculation...
inline void Assembler::CheckSum_PutBit(boolean Bit)
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
inline void Assembler::CheckSum_PutByte(uint8 Byte)
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
void Assembler::CheckSum_PutBytes(uint8 *pBytes, uint32 unSize)
{
    // Variables...
    unsigned long unIndex = 0;

    // Calculate for each byte...
    for(unIndex = 0; unIndex < unSize; unIndex++)
        CheckSum_PutByte(pBytes[unIndex]);
}

// Display statistics...
void Assembler::DisplayStatistics() const
{
    // Variables...
    unsigned int        unVariables     = 0;
    unsigned int        unArrays        = 0;
    unsigned int        unGlobals       = 0;
    unsigned int        unIndex         = 0;
    AA_LinkedListNode  *pNode           = NULL;
    AA_SymbolNodeData  *pSymbolNodeData = NULL;

    // Count all of our various symbol types...
    for(pNode = SymbolTable.pHead, unIndex = 0;
        unIndex < SymbolTable.unNodes; unIndex++, pNode = pNode->pNext)
    {
        // Extract symbol node's data...
        pSymbolNodeData = (AA_SymbolNodeData *) pNode->pData;

        // Array or variable?

            // Array...
            if(pSymbolNodeData->unSize > 1)
                unArrays++;

            // Variable...
            else
                unVariables++;

        // Global if stack index is non-negative...
        if(pSymbolNodeData->nStackIndex >= 0)
            unGlobals++;
    }

    // Display statistics...

        // Input lines...
        printf("\n\t     Source lines read: %d\n", unListingLines);

        // Stack size...
        printf("\t            Stack size: ");

            // Manually specified...
            if(MainHeader.unStackSize != (unsigned) -1)
                printf("%d\n", MainHeader.unStackSize);

            // Default...
            else
                printf("default\n");

        // Thread priority...
        printf("\t       Thread priority: ");
        switch(MainHeader.ThreadPriorityType)
        {
            // User specified...
            case THREAD_PRIORITY_USER:
                printf("%dms time slice\n", MainHeader.unThreadPriorityUser);
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
        printf("\tInstructions assembled: %d\n"
               "\t             Variables: %d\n"
               "\t                Arrays: %d\n"
               "\t               Globals: %d\n"
               "\t       String literals: %d\n"
               "\t                Labels: %d\n"
               "\t    Host API functions: %d\n"
               "\t             Functions: %d\n"
               "\t         Main present?: %s\n\n",

               InstructionStreamHeader.unSize,
               unVariables,
               unArrays,
               unGlobals,
               StringTable.unNodes,
               LabelTable.unNodes,
               HostFunctionTable.unNodes,
               FunctionTable.unNodes,
               (MainHeader.unMainIndex == (unsigned) -1) ? "No" : "Yes");
}

// Expected a character, but did not find...
void Assembler::ErrorExpectedCharacter(char cExpected)
{
    // Output...
    std::cout << UserParameters.GetInputFile() << ": " << unLexerLine + 1 
              << ": error: '" << cExpected << "' expected" << std::endl;

    // Cleanup all resources, reset state, and shutdown assembler...
    ShutDown();
}

// General error...
void Assembler::ErrorGeneral(const char *pszFormat, ...)
{
    // Variables...
    va_list pArgumentList;
    char    szBuffer[1024]  = {0};

    // Format message...

        // Get start of argument list...
        va_start(pArgumentList, pszFormat);

        // Format...
        vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, pArgumentList);

        // Zero out pArgumentList for no particular reason...
        va_end(pArgumentList);

    // Output...
    std::cout << UserParameters.GetProcessName() << ": error: " << szBuffer 
              << std::endl;

    // Cleanup all resources, reset state, and shutdown assembler...
    ShutDown();
}

// Error due to listing...
void Assembler::ErrorListing(const char *pszFormat, ...)
{
    // Variables...
    va_list pArgumentList;
    char    szBuffer[1024]  = {0};

    // Format message...

        // Get start of argument list...
        va_start(pArgumentList, pszFormat);

        // Format...
        vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, pArgumentList);

        // Zero out pArgumentList for no particular reason...
        va_end(pArgumentList);

    // Output...
    std::cout << UserParameters.GetProcessName() << ":" << unLexerLine + 1
              << ": error: " << szBuffer << std::endl;

    // Cleanup all resources, reset state, and shutdown assembler...
    ShutDown();
}

// Verbose output...
void Assembler::ErrorVerbose(const char *pszFormat, ...)
{
    // Variables...
    va_list pArgumentList;
    char    szBuffer[1024]  = {0};

    // Verbose mode not enabled, ignore...
    if(!UserParameters.ShouldBeVerbose())
        return;

    // Output is standard output device, ignore to avoid corruption...
    if(UserParameters.GetOutputFile() == "stdout")
        return;

    // Format message...

        // Get start of argument list...
        va_start(pArgumentList, pszFormat);

        // Format...
        vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, pArgumentList);

        // Zero out pArgumentList for no particular reason...
        va_end(pArgumentList);

    // Output...
    std::cout << UserParameters.GetProcessName() << ": " << szBuffer 
              << std::endl;
}

// Warning...
void Assembler::ErrorWarning(const char *pszFormat, ...)
{
    // Variables...
    va_list pArgumentList;
    char    szBuffer[1024]  = {0};

    // Format message...

        // Get start of argument list...
        va_start(pArgumentList, pszFormat);

        // Format...
        vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, pArgumentList);

        // Zero out pArgumentList for no particular reason...
        va_end(pArgumentList);

    // Output...
    std::cout << UserParameters.GetProcessName() << ": " << unLexerLine + 1
              << ": warning: " << szBuffer << std::endl;
}

// Get the current lexeme...
const char *Assembler::GetCurrentLexeme() const
{
    // Return it...
    return szCurrentLexeme;
}

// Get the current lexeme and store for caller...
char *Assembler::GetCurrentLexeme(char *pszBuffer) const
{
    // Store for caller...
    strcpy(pszBuffer, szCurrentLexeme);

    // Return it...
    return pszBuffer;
}

// Get the current token...
Assembler::Token Assembler::GetCurrentToken() const
{
    // Return it...
    return unLexerCurrentToken;
}

// Get function node's data by name...
Assembler::AA_FunctionNodeData *Assembler::GetFunctionByName(
    const char *pszName)
{
    // Variables...
    AA_LinkedListNode      *pNode               = NULL;
    AA_FunctionNodeData    *pFunctionNodeData   = NULL;
    uint32                  unIndex             = 0;

    // Function table is empty...
    if(!FunctionTable.unNodes)
        return NULL;

    // Traverse function table list until matching function found...
    for(pNode = FunctionTable.pHead, unIndex = 0;
        unIndex < FunctionTable.unNodes;
        unIndex++, pNode = pNode->pNext)
    {
        // Grab function node's data for this entry in function table...
        pFunctionNodeData = (AA_FunctionNodeData *) pNode->pData;

        // Match, return pointer to function node's data...
        if(strcasecmp(pFunctionNodeData->szName, pszName) == 0)
            return pFunctionNodeData;
    }

    // Not found...
    return NULL;
}

// Get instruction by mnemonic...
boolean Assembler::GetInstructionByMnemonic(
    const char *pszMnemonic, AA_InstructionLookup *pInstructionLookup) const
{
    // Variables...
    uint16  usIndex= 0;

    // Iterate through instruction lookup table...
    for(usIndex = 0; usIndex < usInstructionCount; usIndex++)
    {
        // Found...
        if(strcasecmp(InstructionTable[usIndex].szMnemonic, pszMnemonic) == 0)
        {
            // Pass to caller...
           *pInstructionLookup = InstructionTable[usIndex];
            return true;
        }
    }

    // Nothing found...
    return false;
}

// Get the instruction set size...
uint16 Assembler::GetInstructionSetSize()
{
    // Return it...
    return usInstructionCount;
}

// Get a label by identifier...
Assembler::AA_LabelNodeData *Assembler::GetLabelByIdentifier(
    const char *pszIdentifier, uint32 unFunctionIndex) const
{
    // Variables...
    AA_LinkedListNode  *pNode           = NULL;
    AA_LabelNodeData   *pLabelNodeData  = NULL;
    uint32              unIndex         = 0;

    // Label table is empty, abort...
    if(!LabelTable.unNodes)
        return NULL;

    // Traverse label table list until matching label found...
    for(pNode = LabelTable.pHead, unIndex = 0;
        unIndex < LabelTable.unNodes;
        unIndex++, pNode = pNode->pNext)
    {
        // Grab label node's data for this entry in label table...
        pLabelNodeData = (AA_LabelNodeData *) pNode->pData;

        // Matching identifier and function....
        if((strcasecmp(pLabelNodeData->szIdentifier, pszIdentifier) == 0) &&
           (pLabelNodeData->unFunctionIndex == unFunctionIndex))
            return pLabelNodeData;
    }

    // Not found...
    return NULL;
}

// Get the first character of next token...
char Assembler::GetLookAheadCharacter() const
{
    // Variables...
    uint32          unLine  = 0;
    uint32          unIndex = 0;

    // Backup lexer line and secondary index...
    unLine  = unLexerLine;
    unIndex = unLexerIndex1;

    // If we are not in a string, seek past potential leading white space...
    if(LexerState != LEXER_IN_STRING)
    {
        // Seek...
        while(true)
        {
            // We've passed the end of the line...
            if(unIndex >= strlen(ppszListing[unLine]))
            {
                // Seek to next source line in assembly listing...
                unLine++;

                // Reached end of source file, return NULL byte...
                if(unLine >= unListingLines)
                    return '\x0';

                // Reset to first character on new line...
                unIndex = 0;
            }

            // Not white space, look ahead character for next lexeme found...
            if(!IsCharacterWhiteSpace(ppszListing[unLine][unIndex]))
                break;

            // White space, keep seeking...
            unIndex++;
        }
    }

    // Return look-ahead character for next lexeme...
    return ppszListing[unLine][unIndex];
}

// Get the next token...
Assembler::Token Assembler::GetNextToken()
{
    // Variables...
    uint32                  unSourceIndex       = 0;
    uint32                  unDestinationIndex  = 0;
    AA_InstructionLookup    InstructionLookup;

    // Extract lexeme...

        // Move the first index (unLexerIndex0) past the end of last token...
        unLexerIndex0 = unLexerIndex1;

        // End of source line reached...
        if(unLexerIndex0 >= strlen(ppszListing[unLexerLine]))
        {
            // Seek to next line and, if end of listing, notify caller...
            if(!SeekToNextLine())
                return TOKEN_END;
        }

        // Just finished lexing a string, return to normal state...
        if(LexerState == LEXER_END_STRING)
            LexerState = LEXER_NO_STRING;

        // We are not still parsing a string lexeme...
        if(LexerState != LEXER_IN_STRING)
        {
            // Seek passed white space...
            while(true)
            {
                // Not white space, stop seeking...
                if(!IsCharacterWhiteSpace(ppszListing[unLexerLine]
                                                     [unLexerIndex0]))
                    break;

                // White space, seek passed...
                else
                    unLexerIndex0++;
            }
        }

        // Seek second index to lexeme's starting character...
        unLexerIndex1 = unLexerIndex0;

        // Seek until a delimeter is detected...
        while(true)
        {
            // We are still scanning within a string...
            if(LexerState == LEXER_IN_STRING)
            {
                // Reached end of line, but no closing quotation...
                if(unLexerIndex1 >= strlen(ppszListing[unLexerLine]))
                {
                    // Assume invalid token...
                    unLexerCurrentToken = TOKEN_INVALID;
                    return unLexerCurrentToken;
                }

                // Current character is a backslash...
                if(ppszListing[unLexerLine][unLexerIndex1] == '\\')
                {
                    // Seek passed by two characters to skip escape sequence...
                    unLexerIndex1 += 2;
                    continue;
                }

                // If current character is a double-quote, done...
                if(ppszListing[unLexerLine][unLexerIndex1] == '\"')
                    break;

                // Seek to next character...
                unLexerIndex1++;
            }

            // Not currently scanning through a string...
            else
            {
                // Reached end of line, lexeme complete...
                if(unLexerIndex1 >= strlen(ppszListing[unLexerLine]))
                    break;

                // Current character is a delimeter, done...
                if(IsCharacterDelimiter(ppszListing[unLexerLine]
                                                   [unLexerIndex1]))
                    break;

                // Not a delimeter, keep building lexeme...
                unLexerIndex1++;
            }
        }

        // Single-character lexemes appear to have zero length, fix...
        if((unLexerIndex1 - unLexerIndex0) == 0)
            unLexerIndex1++;

        // Lexeme now isolated within unLexerIndex0 and 1 inclusive, extract...
        for(unSourceIndex = unLexerIndex0, unDestinationIndex = 0;
            unSourceIndex < unLexerIndex1;
            unSourceIndex++)
        {
            // We're parsing a string still...
            if(LexerState == LEXER_IN_STRING)
            {
                // Skip escape sequence's backslash, if present...
                if(ppszListing[unLexerLine][unSourceIndex] == '\\')
                    unSourceIndex++;
            }

            // Extract character from source line to lexeme...
            szCurrentLexeme[unDestinationIndex] = ppszListing[unLexerLine]
                                                             [unSourceIndex];

            // Seek to next destination address...
            unDestinationIndex++;
        }

        // Terminate new lexeme...
        szCurrentLexeme[unDestinationIndex] = '\x0';

        /* If lememe is not a string, convert to uppercase...
        if(LexerState != LEXER_IN_STRING)
            strupr(szCurrentLexeme);*/

    // Identify token...

        // Default to invalid for now...
        unLexerCurrentToken = TOKEN_INVALID;

        // String is either more than one in length or not a single backslash...
        if((strlen(szCurrentLexeme) > 1) || (szCurrentLexeme[0] != '\"'))
        {
            // String lexeme still active...
            if(LexerState == LEXER_IN_STRING)
            {
                // Must be a string lexeme...
                unLexerCurrentToken = TOKEN_STRING;
                return TOKEN_STRING;
            }
        }

        // Check for single-character tokens...
        if(strlen(szCurrentLexeme) == 1)
        {
            // Process...
            switch(szCurrentLexeme[0])
            {
                // Double-quote...
                case '\"':

                    // Configure lexer state so strings are lexed correctly...
                    switch(LexerState)
                    {
                        // Not in a string...
                        case LEXER_NO_STRING:

                            // We are now...
                            LexerState = LEXER_IN_STRING;
                            break;

                        // In a string...
                        case LEXER_IN_STRING:

                            // No longer any more...
                            LexerState = LEXER_END_STRING;
                            break;
                    }

                    // Token type is a double quote...
                    unLexerCurrentToken = TOKEN_QUOTE;
                    break;

                // Comma...
                case ',':
                    unLexerCurrentToken = TOKEN_COMMA;
                    break;

                // Colon...
                case ':':
                    unLexerCurrentToken = TOKEN_COLON;
                    break;

                // Opening bracket...
                case '[':
                    unLexerCurrentToken = TOKEN_OPEN_BRACKET;
                    break;

                // Closing bracket...
                case ']':
                    unLexerCurrentToken = TOKEN_CLOSE_BRACKET;
                    break;

                // Open brace...
                case '{':
                    unLexerCurrentToken = TOKEN_OPEN_BRACE;
                    break;

                // Closing brace...
                case '}':
                    unLexerCurrentToken = TOKEN_CLOSE_BRACE;
                    break;

                // New line...
                case '\n':
                    unLexerCurrentToken = TOKEN_NEW_LINE;
                    break;
            }
        }

        // Check for multi-character tokens...

            // Integer...
            if(IsStringInteger(szCurrentLexeme))
                unLexerCurrentToken = TOKEN_INTEGER;

            // Float...
            if(IsStringFloat(szCurrentLexeme))
                unLexerCurrentToken = TOKEN_FLOAT;

            // Identifier...
            if(IsStringIdentifier(szCurrentLexeme))
                unLexerCurrentToken = TOKEN_IDENTIFIER;

            // Directive...

                // SetHost...
                if(strcasecmp(szCurrentLexeme, "SetHost") == 0)
                    unLexerCurrentToken = TOKEN_DIRECTIVE_SETHOST;

                // SetStackSize...
                if(strcasecmp(szCurrentLexeme, "SetStackSize") == 0)
                    unLexerCurrentToken = TOKEN_DIRECTIVE_SETSTACKSIZE;

                // Set thread priority...
                if(strcasecmp(szCurrentLexeme, "SetThreadPriority") == 0)
                    unLexerCurrentToken = TOKEN_DIRECTIVE_SETTHREADPRIORITY;

            // Variable or array...
            if(strcasecmp(szCurrentLexeme, "Var") == 0)
                unLexerCurrentToken = TOKEN_VAR;

            // Function...
            if(strcasecmp(szCurrentLexeme, "Func") == 0)
                unLexerCurrentToken = TOKEN_FUNC;

            // Parameter...
            if(strcasecmp(szCurrentLexeme, "Param") == 0)
                unLexerCurrentToken = TOKEN_PARAM;

            // Register...

                // T0...
                if(strcasecmp(szCurrentLexeme, "_RegisterT0") == 0)
                    unLexerCurrentToken = TOKEN_REGISTER_T0;
                    
                // T1...
                if(strcasecmp(szCurrentLexeme, "_RegisterT1") == 0)
                    unLexerCurrentToken = TOKEN_REGISTER_T1;
                
                // Return...
                if(strcasecmp(szCurrentLexeme, "_RegisterReturn") == 0)
                    unLexerCurrentToken = TOKEN_REGISTER_RETURN;

            // Instruction...
            if(GetInstructionByMnemonic(szCurrentLexeme, &InstructionLookup))
                unLexerCurrentToken = TOKEN_INSTRUCTION;

    // Return the current token to caller...
    return unLexerCurrentToken;
}

// Get symbol size by identifier...
uint32 Assembler::GetSizeByIdentifier(const char *pszIdentifier,
                                      uint32 unFunctionIndex) const
{
    // Variables...
    AA_SymbolNodeData  *pSymbolNodeData = NULL;

    // Get symbol node's data...
    pSymbolNodeData = GetSymbolByIdentifier(pszIdentifier, unFunctionIndex);

    // Return size...
    return pSymbolNodeData->unSize;
}

// Get the stack index by identifier...
int32 Assembler::GetStackIndexByIdentifier(const char *pszIdentifier,
                                           uint32 unFunctionIndex) const
{
    // Variables...
    AA_SymbolNodeData  *pSymbolNodeData = NULL;

    // Get the symbol's data...
    pSymbolNodeData = (AA_SymbolNodeData *)
                        GetSymbolByIdentifier(pszIdentifier, unFunctionIndex);

    // Return stack index of symbol...
    return pSymbolNodeData->nStackIndex;
}

// Get symbol data by identifier...
Assembler::AA_SymbolNodeData *Assembler::GetSymbolByIdentifier(
    const char *pszIdentifier, uint32 unFunctionIndex) const
{
    // Variables...
    AA_LinkedListNode  *pNode           = NULL;
    AA_SymbolNodeData  *pSymbolNodeData = NULL;
    uint32              unIndex         = 0;

    // Scan through symbol table until we find a match...
    for(pNode = SymbolTable.pHead, unIndex = 0;
        unIndex < SymbolTable.unNodes;
        unIndex++, pNode = pNode->pNext)
    {
        // Grab this node's symbol data...
        pSymbolNodeData = (AA_SymbolNodeData *) pNode->pData;

        // Identifier matches...
        if(strcasecmp(pSymbolNodeData->szIdentifier, pszIdentifier) == 0)
        {
            // Either the scope matches or this is a global variable...
            if((pSymbolNodeData->unFunctionIndex == unFunctionIndex) ||
               (pSymbolNodeData->nStackIndex >= 0))
                return pSymbolNodeData;
        }
    }

    // Symbol not found...
    return NULL;
}

// Is character part of a delimeter?
boolean Assembler::IsCharacterDelimiter(char cCharacter) const
{
    // Anything that seperates elements is a delimeter...
    if((cCharacter == ':') || (cCharacter == ',')  ||
       (cCharacter == '"') || (cCharacter == '[')  ||
       (cCharacter == ']') || (cCharacter == '{')  ||
       (cCharacter == '}') || (cCharacter == '\n') ||
       IsCharacterWhiteSpace(cCharacter))
        return true;

    // Not a delimeter...
    else
        return false;
}

// Is character valid for being part of an identifier?
boolean Assembler::IsCharacterIdentifier(char cCharacter) const
{
    // Identifier if a number, letter, or underscore...
    if((cCharacter >= '0' && cCharacter <= '9') ||
       (cCharacter >= 'A' && cCharacter <= 'Z') ||
       (cCharacter >= 'a' && cCharacter <= 'z') ||
       (cCharacter == '_'))
        return true;

    // Not an identifier...
    else
        return false;
}

// Is character numeric?
boolean Assembler::IsCharacterNumeric(char cCharacter) const
{
    // Numeric...
    if(cCharacter >= '0' && cCharacter <= '9')
        return true;

    // Not numeric...
    else
        return false;
}

// Is character white space?
boolean Assembler::IsCharacterWhiteSpace(char cCharacter) const
{
    // White space if a space or a horizontal tab...
    if(cCharacter == ' ' || cCharacter == '\t')
        return true;

    // Not white space...
    else
        return false;
}

// Is string a float?
boolean Assembler::IsStringFloat(const char *pszString) const
{
    // Variables...
    uint32  unIndex             = 0;
    boolean bRadixPointFound    = false;

    // Check parameter...

        // Bad pointer...
        if(!pszString)
            return false;

        // Empty string...
        if(!strlen(pszString))
            return false;

    // Check validity of each character individually...
    for(unIndex = 0; unIndex < strlen(pszString); unIndex++)
    {
        // Character is not a number, radix point, or negative sign...
        if(!IsCharacterNumeric(pszString[unIndex]) &&
           !(pszString[unIndex] == '.') &&
           !(pszString[unIndex] == '-'))
            return false;
    }

    // Scan for radix point...
    for(unIndex = 0, bRadixPointFound = false; unIndex < strlen(pszString);
        unIndex++)
    {
        // Found a radix point...
        if(pszString[unIndex] == '.')
        {
            // Radix point already found...
            if(bRadixPointFound)
                return false;

            // First radix found, remember...
            else
                bRadixPointFound = true;
        }
    }

    // Scan for incorrectly positioned negation sign...
    for(unIndex = 1; unIndex < strlen(pszString); unIndex++)
    {
        // Incorrect position...
        if(pszString[unIndex] == '-')
            return false;
    }

    // Radix point was found...
    if(bRadixPointFound)
        return true;

    // No radix point found...
    else
        return false;
}

// Is string a valid identifier?
boolean Assembler::IsStringIdentifier(const char *pszString) const
{
    // Variables...
    uint32  unIndex = 0;

    // Check parameter...

        // Bad pointer...
        if(!pszString)
            return false;

        // Empty string...
        if(!strlen(pszString))
            return false;

    // First character cannot be a digit...
    if((pszString[0] >= '0') && (pszString[0] <= '9'))
        return false;

    // Check each character to see if not a valid identifier...
    for(unIndex = 0; unIndex < strlen(pszString); unIndex++)
    {
        // Bad character...
        if(!IsCharacterIdentifier(pszString[unIndex]))
            return false;
    }

    // String is an identifier...
    return true;
}

// Is string an integer?
boolean Assembler::IsStringInteger(const char *pszString) const
{
    // Variables...
    uint32  unIndex = 0;

    // Check parameter...

        // Bad pointer...
        if(!pszString)
            return false;

        // Empty string...
        if(!strlen(pszString))
            return false;

    // Check to make sure each character is valid...
    for(unIndex = 0; unIndex < strlen(pszString); unIndex++)
    {
        // Character is not an integer or negative sign...
        if(!IsCharacterNumeric(pszString[unIndex]) &&
           !(pszString[unIndex] == '-'))
            return false;
    }

    // Check to make sure
    for(unIndex = 1; unIndex < strlen(pszString); unIndex++)
    {
        // Sign character is in incorrect position...
        if(pszString[unIndex] == '-')
            return false;
    }

    // String is an integer...
    return true;
}

// Is string white space?
boolean Assembler::IsStringWhiteSpace(const char *pszString) const
{
    // Variables...
    uint32  unIndex = 0;

    // Check parameter...

        // Bad pointer...
        if(!pszString)
            return false;

    // Empty string counts as white space...
    if(!strlen(pszString))
        return true;

    // Scan each character and check for non-white space characters...
    for(unIndex = 0; unIndex < strlen(pszString); unIndex++)
    {
        // Bad character...
        if(!IsCharacterWhiteSpace(pszString[unIndex]))
            return false;
    }

    // String is white space...
    return true;
}

// Add node to linked list and return index...
uint32 Assembler::List_Add(AA_LinkedList *pList, void *pData)
{
    // Variables...
    AA_LinkedListNode  *pNewNode = NULL;

    // Create new node...
    pNewNode = (AA_LinkedListNode *) malloc(sizeof(AA_LinkedListNode));

    // Initialize new node's data...
    pNewNode->pData = pData;

    // There is nothing after this element...
    pNewNode->pNext = NULL;

    // List is currently empty...
    if(!pList->unNodes)
    {
        // Both head and tail must point to this new node...
        pList->pHead = pNewNode;
        pList->pTail = pNewNode;
    }

    // List is not currently empty...
    else
    {
        // Place our node as the new tail...
        pList->pTail->pNext = pNewNode;

        // Update list's tail pointer to point to our new node...
        pList->pTail        = pNewNode;
    }

    // Remember that we have one more node now...
    pList->unNodes++;

    // Return new node's index...
    return (pList->unNodes - 1);
}

// Initialize linked list...
void Assembler::List_Initialize(AA_LinkedList *pList)
{
    // Clear head and tail node pointers...
    pList->pHead    = NULL;
    pList->pTail    = NULL;

    // There are zero nodes in a new linked list...
    pList->unNodes  = 0;
}

// Free linked list...
void Assembler::List_Free(AA_LinkedList *pList)
{
    // Variables...
    AA_LinkedListNode  *pNode       = NULL;
    AA_LinkedListNode  *pNextNode   = NULL;

    // Free each node...
    for(pNode = pList->pHead, pNextNode = NULL; pNode; pNode = pNextNode)
    {
        // Remember pointer to next node, if any...
        pNextNode = pNode->pNext;

        // Free current node's data...
        free(pNode->pData);

        // Clear it out to make debugging easier...
        memset(pNode, 0, sizeof(AA_LinkedListNode));

        // Free current node...
        free(pNode);
    }

    // Remember...
    pList->pHead    = NULL;
    pList->pTail    = NULL;
    pList->unNodes  = 0;
}

// Load input file, throw string on error...
void Assembler::LoadInput()
{
    // Variables...
    FILE           *hInput              = NULL;
    uint32          unRawInputOffset    = 0;
    char           *pszRawInput         = NULL;
    char           *pszTemp             = NULL;
    uint32          unRawInputAllocated = 0;
    uint32          unCurrentLine       = 0;
    uint32          unCurrentLineLength = 0;
    char           *pszRawInputSeek     = NULL;

    // Open input...

        // Be verbose...
        ErrorVerbose("opening `%s'", UserParameters.GetInputFile().c_str());

        // Standard input pipe...
        if(UserParameters.GetInputFile() == "stdin")
            hInput = stdin;

        // Listing on disk...
        else
        {
            // Open...
            hInput = fopen(UserParameters.GetInputFile().c_str(), "rb");

                // Failed...
                if(!hInput)
                    throw "unable to open input for reading";
        }

    // Buffer raw input...

        // Be verbose...
        ErrorVerbose("buffering input");

        // Collect data...
        for(unRawInputOffset = 0, pszRawInput = NULL, unRawInputAllocated = 0,
            unListingLines = 1;
            !feof(hInput);
            unRawInputOffset++)
        {
            // We need to enlarge our buffer...
            if((unRawInputOffset + 1) > unRawInputAllocated)
            {
                // Allocate another kilobyte...
                pszTemp = (char *) realloc(pszRawInput, unRawInputAllocated +
                                                        (1024 * sizeof(char)));

                    // Failed...
                    if(!pszTemp)
                    {
                        // Cleanup raw input buffer, if necessary...
                        if(pszRawInput)
                            free(pszRawInput);

                        // If input is not standard input device, close stream...
                        if(hInput != stdin)
                            fclose(hInput);

                        // Remember listing empty...
                        unListingLines = 0;

                        // Abort...
                        throw "insufficient memory";
                    }

                    // Remember...
                    pszRawInput = pszTemp;
                    unRawInputAllocated += (1024 * sizeof(char));
            }

            // Store byte...
            pszRawInput[unRawInputOffset] = fgetc(hInput);

            // Process special characters of interest...
            switch(pszRawInput[unRawInputOffset])
            {
                // New line, remember...
                case '\n':
                    unListingLines++;
                    break;

                // Carriage return, skip...
                case '\r':
                    unRawInputOffset--;
                    break;
            }
        }

        // Terminate raw input...
        pszRawInput[unRawInputOffset - 1] = '\x0';

        // Be verbose...
        ErrorVerbose("read %d lines of %d bytes and allocated %d bytes",
                     unListingLines, unRawInputOffset + 1, unRawInputAllocated);

    // If input is not standard input device, close stream...
    if(hInput != stdin)
        fclose(hInput);

    // Allocate listing buffer...
    ppszListing = (char **) malloc(unListingLines * sizeof(char *));

        // Failed...
        if(!ppszListing)
        {
            // Cleanup raw input buffer...
            free(pszRawInput);

            // Remember...
            unListingLines = 0;

            // Abort...
            throw "insufficient memory";
        }

        // Clear to make debugging easier...
        memset(ppszListing, 0, unListingLines * sizeof(char *));

    // Populate listing buffer...
    for(unCurrentLine = 0, pszRawInputSeek = pszRawInput;
        unCurrentLine < unListingLines;
        unCurrentLine++)
    {
        // Calculate current line length...

            // Reached end of input, so calculate until terminator...
            if(!strchr(pszRawInputSeek, '\n'))
                unCurrentLineLength = strlen(pszRawInputSeek);

            // Still more input lines...
            else
            {
                // Calculate line length...
                unCurrentLineLength = (strchr(pszRawInputSeek, '\n') -
                                       pszRawInputSeek);

                // Include new line byte as well...
                unCurrentLineLength++;
            }

        // Allocate storage space for this line, plus terminator byte...
        ppszListing[unCurrentLine] = (char *) malloc(unCurrentLineLength + 1);

            // Failed...
            if(!ppszListing[unCurrentLine])
            {
                // Free raw input...
                free(pszRawInput);

                // Free each line previously allocated...
                while(unCurrentLine != 0)
                {
                    // Free...
                    free(ppszListing[unCurrentLine - 1]);
                    ppszListing[unCurrentLine - 1] = NULL;

                    // Seek to previous...
                    unCurrentLine--;
                }

                // Free listing base buffer pointer...
                free(ppszListing);
                ppszListing = NULL;
                unListingLines = 0;

                // Abort...
                throw "insufficient memory";
            }

        // Store line...
        strncpy(ppszListing[unCurrentLine], pszRawInputSeek,
                unCurrentLineLength);

        // Terminate...
        ppszListing[unCurrentLine][unCurrentLineLength] = '\x0';

        // Strip comments...
        StripComments(ppszListing[unCurrentLine]);

        // Trim white space...
        TrimWhiteSpace(ppszListing[unCurrentLine]);

        // Line shrunk after being cleaned up, shrink memory accordingly...
        if(strlen(ppszListing[unCurrentLine]) < unCurrentLineLength)
        {
            // Shrink...
            pszTemp = (char *) realloc(ppszListing[unCurrentLine],
                                       (strlen(ppszListing[unCurrentLine]) + 1)
                                        * sizeof(char));

            // Resized ok, otherwise not critical...
            if(pszTemp)
                ppszListing[unCurrentLine] = pszTemp;
        }

        // Seek to next line...
        pszRawInputSeek += unCurrentLineLength;
    }

    // Cleanup...
    free(pszRawInput);
    pszRawInput = NULL;
    unRawInputAllocated = 0;
}

// Reset lexer...
void Assembler::ResetLexer()
{
    // Be verbose...
    ErrorVerbose("resetting lexer");

    // Seek to beginning line of assembly listing...
    unLexerLine = 0;

    // Reset both lexeme indices...
    unLexerIndex0 = 0;
    unLexerIndex1 = 0;

    // Reset token type to invalid...
    unLexerCurrentToken = TOKEN_INVALID;

    // Reset lexeme state...
    LexerState = LEXER_NO_STRING;
}

// Seek to the next line, if there is one...
boolean Assembler::SeekToNextLine()
{
    // Increment line counter...
    unLexerLine++;

    // Past the end of the source...
    if(unLexerLine >= unListingLines)
        return false;

    // Set both lexeme indicies to beginning of new line...
    unLexerIndex0 = 0;
    unLexerIndex1 = 0;

    // Reset lexeme state, since strings cannot span multiple lines...
    LexerState = LEXER_NO_STRING;

    // Done...
    return true;
}

// Set function parameter count and local data size...
void Assembler::SetFunctionInfo(const char *pszName, uint8 ParameterCount,
                                     uint32 unLocalDataSize)
{
    // Variables...
    AA_FunctionNodeData    *pFunctionNodeData   = NULL;

    // Fetch function node's data...
    pFunctionNodeData = GetFunctionByName(pszName);

    // Set parameter count and local data size...
    pFunctionNodeData->ParameterCount   = ParameterCount;
    pFunctionNodeData->unLocalDataSize  = unLocalDataSize;
}

// Set instruction operand type...
void Assembler::SetOperandType(uint16 usInstructionIndex,
                                    uint8 OperandIndex, AA_OperandType Type)
{
    // Set requested operand to given operand type...
    InstructionTable[usInstructionIndex].pOperandList[OperandIndex]
        = Type;
}

// Cleanup all resources, reset state, and shutdown assembler...
void Assembler::ShutDown()
{
    // Variables...
    uint32 unIndex = 0;

    // Free assembly listing, if necessary...
    if(unListingLines)
    {
        // Free each line...
        for(unIndex = 0; unIndex < unListingLines; unIndex++)
            free(ppszListing[unIndex]);

        // Free base pointer...
        free(ppszListing);

        // Remember state...
        unListingLines = 0;
        ppszListing = NULL;
    }

    // Free assembled instruction stream, if existent...
    if(pInstructionStream)
    {
        // Free each instruction's operand list...
        for(unIndex = 0; unIndex < InstructionStreamHeader.unSize; unIndex++)
        {
            // This instruction has an operand list, free...
            if(pInstructionStream[unIndex].pOperandList)
                free(pInstructionStream[unIndex].pOperandList);
        }

        // Free instruction stream...
        free(pInstructionStream);
        pInstructionStream = NULL;
    }

    // Free tables...

        // Function table, if nexessary...
        if(FunctionTable.unNodes)
            List_Free(&FunctionTable);

        // Host function table, if nexessary...
        if(HostFunctionTable.unNodes)
            List_Free(&HostFunctionTable);

        // Label table, if nexessary...
        if(LabelTable.unNodes)
            List_Free(&LabelTable);

        // String table, if nexessary...
        if(StringTable.unNodes)
            List_Free(&StringTable);

        // Symbol table, if nexessary...
        if(SymbolTable.unNodes)
            List_Free(&SymbolTable);

    // Reset lexer...
    ResetLexer();
}

// Strip comments from source line...
void Assembler::StripComments(char *pszSourceLine)
{
    // Variables...
    uint32  unIndex         = 0;
    boolean bInsideString   = false;

    // String empty, abort...
    if(!strlen(pszSourceLine))
        return;

    // Scan through source line and terminate at first legitimate semicolon...
    for(unIndex = 0, bInsideString = false;
        unIndex < (strlen(pszSourceLine) - 1);
        unIndex++)
    {
        // Check for strings because they may contain semicolons...
        if(pszSourceLine[unIndex] == '\"')
            bInsideString = !bInsideString;

        // Found a semicolon...
        if(pszSourceLine[unIndex] == ';')
        {
            // It's not inside a string...
            if(!bInsideString)
            {
                // Terminate...
                pszSourceLine[unIndex]      = '\n';
                pszSourceLine[unIndex + 1]  = '\x0';

                // Done...
                break;
            }
        }
    }
}

// Trim white space from left and right side...
void Assembler::TrimWhiteSpace(char *pszSourceLine)
{
    // Variables...
    boolean bNewLineTerminated = false;

    // Keep shifting left, while left most character is white space...
    while(IsCharacterWhiteSpace(pszSourceLine[0]))
        strcpy(pszSourceLine, pszSourceLine + 1);

    // Check if new line is at the end...
    bNewLineTerminated = pszSourceLine[strlen(pszSourceLine) - 1] == '\n';

    // Keep terminating right most character, while it is white space
    //  or a new line character...
    while(IsCharacterWhiteSpace(pszSourceLine[strlen(pszSourceLine) - 1]) ||
          (pszSourceLine[strlen(pszSourceLine) - 1] == '\n'))
        pszSourceLine[strlen(pszSourceLine) - 1] = '\x0';

    // Don't forget to add new line character, if originally there...
    if(bNewLineTerminated)
        strcat(pszSourceLine, "\n");

    /* String is greater than one character in length...
    if(unStringLength > 1)
    {
        // Calculate left side white space...
        for(unIndex = 0; unIndex < unStringLength; unIndex++)
        {
            // Character is not white space, stop counting...
            if(!IsCharacterWhiteSpace(pszSourceLine[unIndex]))
                break;
        }

        // String was padded on the left, according to previous calculation...
        unPadLength = unIndex;
        if(unPadLength)
        {
            // Shift left by the amount calculated above...
            for(unIndex = unPadLength; unIndex < unStringLength; unIndex++)
                pszSourceLine[unIndex - unPadLength] = pszSourceLine[unIndex];

            // Blank out previous location...
            for(unIndex = unStringLength - unPadLength;
                unIndex < unStringLength;
                unIndex++)
                pszSourceLine[unIndex] = '\x20';
        }

        // Terminate string at beginning of right hand white space...
        for(unIndex = (unStringLength - 1); unIndex > 0; unIndex--)
        {
            // Character is not white space...
            if(!IsCharacterWhiteSpace(pszSourceLine[unIndex]))
            {
                // Terminate, done...
                pszSourceLine[unIndex + 1] = '\x0';
                break;
            }
        }
    }*/
}

// Buffers bytes in memory, throw string on error...
void Assembler::Write_BufferBytes(const void *pBuffer, size_t ulBytes)
{
    // Variables...
    uint8 *pTemp = NULL;

    // Resize allocation buffer to accomodate new data...
    pTemp = (uint8 *) realloc(pOutputBuffer, unOutputBufferAllocated +
                                                     ulBytes);

        // Failed...
        if(!pTemp)
            throw "memory allocated error";

        // Remember memory...
        pOutputBuffer = pTemp;

    // Initialize added chunk...
    memcpy(&pTemp[unOutputBufferAllocated], pBuffer, ulBytes);

    // Remember that we have more memory now...
    unOutputBufferAllocated += ulBytes;
}

// Write executable to disk, throw string on error...
void Assembler::WriteExecutable()
{
    // Variables...
    char                            szBuffer[1024]              = {0};
    FILE                           *hOutput                     = NULL;
    uint32                          unInstructionIndex          = 0;
    uint32                          unOperandIndex              = 0;
    Agni_StringStreamHeader         StringStreamHeader;
    Agni_FunctionTableHeader        FunctionTableHeader;
    Agni_HostFunctionTableHeader    HostFunctionTableHeader;
    AA_LinkedListNode              *pListNode                   = NULL;
    uint32                          unTemp                      = 0;
    AA_FunctionNodeData            *pFunctionNodeData           = NULL;
    uint8                           ucTemp                      = 0;

    // Be verbose...
    ErrorVerbose("writing executable to `%s'", UserParameters.GetOutputFile().c_str());

    // Initialize final fields of Agni executable header...

        // Initialize signature...
        memset(szBuffer, '\x90', sizeof(szBuffer));
        strncpy(&szBuffer[2], "AGNI", strlen("AGNI"));
        memcpy(&MainHeader.Signature, szBuffer, sizeof(MainHeader.Signature));

        // Major and minor assemble / compile time Agni version...
        MainHeader.ucMajorAgniVersion           = AGNI_VERSION_MAJOR;
        MainHeader.ucMinorAgniVersion           = AGNI_VERSION_MINOR;

        // Major and minor version numbers of minimum acceptable runtime...
        MainHeader.ucMajorRequiredAgniVersion   = AGNI_VERSION_NEEDED_MAJOR;
        MainHeader.ucMinorRequiredAgniVersion   = AGNI_VERSION_NEEDED_MINOR;

        // Clear checksum because it will be calculated after final write...
        MainHeader.unCheckSum                   = 0;

    // Try to write executable to disk...
    try
    {
        // Output main header...
        Write_BufferBytes(&MainHeader, sizeof(Agni_MainHeader));

        // Output instruction stream...

            // Output instruction stream header...
            Write_BufferBytes(&InstructionStreamHeader,
                              sizeof(Agni_InstructionStreamHeader));

            // Output each instruction...
            for(unInstructionIndex = 0;
                unInstructionIndex < InstructionStreamHeader.unSize;
                unInstructionIndex++)
            {
                // Output the operation code... (2 bytes)
                Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                  usOperationCode, sizeof(uint16));

                // Output operand count... (1 byte)
                Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                  OperandCount, sizeof(uint8));

                // Output each operand...
                for(unOperandIndex = 0;
                    unOperandIndex <
                            pInstructionStream[unInstructionIndex].OperandCount;
                    unOperandIndex++)
                {
                    // Output operand type... (1 byte)
                    Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                      pOperandList[unOperandIndex].OperandType,
                                      sizeof(uint8));

                    // Output operand data differently depending on type...
                    switch(pInstructionStream[unInstructionIndex].
                           pOperandList[unOperandIndex].OperandType)
                    {
                        // Integer literal...
                        case OT_AVM_INTEGER:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nLiteralInteger, sizeof(int32));

                            // Done...
                            break;
                        }

                        // Floating-point literal...
                        case OT_AVM_FLOAT:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              fLiteralFloat, sizeof(float32));

                            // Done...
                            break;
                        }

                        // String index...
                        case OT_AVM_INDEX_STRING:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nStringTableIndex, sizeof(int32));

                            // Done...
                            break;
                        }

                        // Instruction index...
                        case OT_AVM_INDEX_INSTRUCTION:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nInstructionIndex, sizeof(int32));

                            // Done...
                            break;
                        }

                        // Absolute stack index...
                        case OT_AVM_INDEX_STACK_ABSOLUTE:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nStackIndex[0], sizeof(int32));

                            // Done...
                            break;
                        }

                        // Relative stack index...
                        case OT_AVM_INDEX_STACK_RELATIVE:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nStackIndex, sizeof(int32) * 2);

                            // Done...
                            break;
                        }

                        // Function index...
                        case OT_AVM_INDEX_FUNCTION:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nFunctionIndex, sizeof(int32));

                            // Done...
                            break;
                        }

                        // Host function index...
                        case OT_AVM_INDEX_FUNCTION_HOST:
                        {
                            // Output... (4 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              nHostFunctionIndex, sizeof(int32));

                            // Done...
                            break;
                        }

                        // Register...
                        case OT_AVM_REGISTER:
                        {
                            // Output... (1 bytes)
                            Write_BufferBytes(&pInstructionStream[unInstructionIndex].
                                              pOperandList[unOperandIndex].
                                              Register, sizeof(uint8));

                            // Done...
                            break;
                        }

                        // Unknown operand type...
                        default:
                        {
                            // Format error message...
                            sprintf(szBuffer, "unknown operand type (%2hhxh)",
                                    pInstructionStream[unInstructionIndex].
                                    pOperandList[unOperandIndex].OperandType);

                            // Abort...
                            throw (const char *) szBuffer;
                        }
                    }
                }
            }

        // Output string table...

            // String stream header...

                // Initialize...
                memset(&StringStreamHeader, '\x0',
                       sizeof(Agni_StringStreamHeader));
                StringStreamHeader.unSize = StringTable.unNodes;

                // Output...
                Write_BufferBytes(&StringStreamHeader,
                                  sizeof(Agni_StringStreamHeader));

            // String stream...
            for(pListNode = StringTable.pHead;
                pListNode;
                pListNode = pListNode->pNext)
            {
                // Calculate string length...
                unTemp = strlen((char *) pListNode->pData);

                // Output length... (4 bytes)
                Write_BufferBytes(&unTemp, sizeof(uint32));

                // Output string... (unTemp bytes)
                Write_BufferBytes((char *) pListNode->pData, unTemp);
            }

        // Output function table...

            // Function table header...

                // Initialize...
                memset(&FunctionTableHeader, '\x0',
                       sizeof(Agni_FunctionTableHeader));
                FunctionTableHeader.unSize = FunctionTable.unNodes;

                // Output...
                Write_BufferBytes(&FunctionTableHeader,
                                  sizeof(Agni_FunctionTableHeader));

            // Output each function's information...
            for(pListNode = FunctionTable.pHead;
                pListNode;
                pListNode = pListNode->pNext)
            {
                // Extract function node's data...
                pFunctionNodeData = (AA_FunctionNodeData *) pListNode->pData;

                // Output the entry point... (4 bytes)
                Write_BufferBytes(&pFunctionNodeData->unEntryPoint,
                                  sizeof(uint32));

                // Output parameter count... (1 byte)
                Write_BufferBytes(&pFunctionNodeData->ParameterCount,
                                  sizeof(uint8));

                // Output local data size... (4 bytes)
                Write_BufferBytes(&pFunctionNodeData->unLocalDataSize,
                                  sizeof(uint32));

                // Calculate the length of the function name...
                unTemp = strlen(pFunctionNodeData->szName);
                ucTemp = unTemp;

                    // Longer than we can accomodate in 8 bits...
                    if(unTemp > 0xfe)
                        throw "function name too long";

                // Output the length of the function name...
                Write_BufferBytes(&ucTemp, sizeof(uint8));

                // Output the function name...
                Write_BufferBytes(pFunctionNodeData->szName, unTemp);

                // Be verbose...
                ErrorVerbose("function %s at entry point 0x%08x wrote out...",
                             pFunctionNodeData->szName,
                             pFunctionNodeData->unEntryPoint);

            }

        // Output host function table...

            // Output host function table header...

                // Initialize...
                memset(&HostFunctionTableHeader, '\x0',
                       sizeof(Agni_HostFunctionTableHeader));
                HostFunctionTableHeader.unSize = HostFunctionTable.unNodes;

                // Output... (sizeof(Agni_HostFunctionTableHeader) bytes)
                Write_BufferBytes(&HostFunctionTableHeader,
                                  sizeof(Agni_HostFunctionTableHeader));

            // Output each host function's information...
            for(pListNode = HostFunctionTable.pHead;
                pListNode;
                pListNode = pListNode->pNext)
            {
                // Calculate the length of the host function name...
                unTemp = strlen((char *) pListNode->pData);
                ucTemp = unTemp;

                    // Too long...
                    if(unTemp > 0xfe)
                        throw "host function name too long";

                // Output the length of the host function name...
                Write_BufferBytes(&ucTemp, sizeof(uint8));

                // Output the host function name...
                Write_BufferBytes(pListNode->pData, unTemp);
            }

        // Establish output location...

            // Standard output device...
            if(UserParameters.GetOutputFile() == "stdout")
                hOutput = stdout;

            // Disk file...
            else
            {
                // Open...
                hOutput = fopen(UserParameters.GetOutputFile().c_str(), "wb");

                    // Failed...
                    if(!hOutput)
                        throw "unable to create output for writing";
            }

        // Calculate checksum...

            // Calculate checksum...
            unTempCheckSum = 0;
            CheckSum_PutBytes(pOutputBuffer, unOutputBufferAllocated);

            // Calculate checksum field offset in buffer...
            unTemp = (uint8 *) &MainHeader.unCheckSum -
                     (uint8 *) &MainHeader;

            // Store in checksum field in header...
            memcpy(&pOutputBuffer[unTemp], &unTempCheckSum,
                   sizeof(MainHeader.unCheckSum));

            // Be verbose...
            ErrorVerbose("checksum 0x%08x (key 0x%08x)", unTempCheckSum,
                         unCheckSumKey);

        // Dump to output location and check for error...
        if(fwrite(pOutputBuffer, unOutputBufferAllocated, 1, hOutput) != 1)
            throw "output dump error";

        // Be verbose...
        ErrorVerbose("dumped %u bytes", unOutputBufferAllocated);

        // Output handle is not to standard output..
        if(hOutput != stdout)
        {
            // Close output and remember...
            fclose(hOutput);
            hOutput = NULL;
        }

        // Cleanup...
        free(pOutputBuffer);
        pOutputBuffer           = NULL;
        unOutputBufferAllocated = 0;
    }

        // Failed to write to disk...
        catch(const char *pszReason)
        {
            // Output handle to a disk file is still open...
            if(hOutput && (hOutput != stdout))
            {
                // Close stream...
                fclose(hOutput);

                // Delete incomplete executable and warn on error...
                if(remove(UserParameters.GetOutputFile().c_str()) == -1)
                    ErrorWarning("unable to delete corrupt `%s'",
                                 UserParameters.GetOutputFile().c_str());
            }

            // Output buffer allocated...
            if(unOutputBufferAllocated)
            {
                // Cleanup...
                free(pOutputBuffer);
                pOutputBuffer           = NULL;
                unOutputBufferAllocated = 0;
            }

            // Pass reason to higher level exception handler...
            throw (const char *) pszReason;
        }
}

// Deconstructor...
Assembler::~Assembler()
{
    // Cleanup all resources, reset state, and shutdown assembler...
    ShutDown();
}

// Assembler parameters default constructor...
Assembler::Parameters::Parameters()
    : OptimizationLevel(0),
      bVerbose(false)
{

}
                    
// Get the process name...
std::string const &Assembler::Parameters::GetProcessName() const
{
    // Return it...
    return sProcessName;
}

// Get the input file name...
std::string const &Assembler::Parameters::GetInputFile() const
{
    // Return it...
    return sInputFile;
}

// Get the optimization level...
uint8 const Assembler::Parameters::GetOptimizationLevel() const
{
    // Return it...
    return OptimizationLevel;
}

// Get the output file name...
std::string const &Assembler::Parameters::GetOutputFile() const
{
    // Return it...
    return sOutputFile;
}

// Initialize from the command line automatically...
bool Assembler::Parameters::ParseCommandLine(
    int const nArguments,
    char * const ppszArguments[])
{
    // Variables...
    char    cOption = 0;
    int     nOption = 0;
    int     nTemp   = 0;

    // Extract AgniAssembler executable name...
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
        /* cs.duke.edu/courses/spring04/cps108/resources/getoptman.html */
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
                sInputFile = optarg;

                // Done...
                break;

            // Help...
            case 'h':

                // Display help...
                PrintHelp();

                // No more parsing necessary...
                return false;

            // Optimization...
            case 'O':

                // Remember optimization level...
                OptimizationLevel = atoi(optarg);

                // Done...
                break;

            // Output...
            case 'o':

                // Extension length...
                nTemp = strlen("." AGNI_FILE_EXTENSION_EXECUTABLE);

                // Remember...
                sOutputFile = optarg;

                // Check to make sure contains proper file extension...
                if(sOutputFile.rfind("." AGNI_FILE_EXTENSION_EXECUTABLE, 
                                     sOutputFile.length() - 1, nTemp) ==
                   std::string::npos)
                {
                    // Append it then...
                    sOutputFile += "." AGNI_FILE_EXTENSION_EXECUTABLE;
                }

                // Done...
                break;

            // Verbose...
            case 'V':

                // Remember...
                bVerbose = true;

                // Done...
                break;

            // Version...
            case 'v':

                // Display version...
                PrintVersion();

                // No more parsing necessary...
                return false;

            // Missing parameter or unrecognized switch...
            case '?':
            default:

                // Unknown switch, alert...
                std::cout << sProcessName << "\"" << optopt 
                          << "\" unrecognized option" << std::endl;

                // No more parsing necessary...
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
            std::cout << sProcessName << ": option \"" 
                      << ppszArguments[optind++] << "\" unknown" << std::endl;
        }
        
        // No more parsing should be done...
        return false;
    }
    
    // Recommend ok to continue with assembly...
    return true;
}

// Print usage...
void Assembler::Parameters::PrintHelp() const
{
    // Display help...
    std::cout << 
        "Usage: aga [option(s)] [input-file] [output-file]\n"
        "Purpose: Provides avm target backend to AgniCompiler...\n\n"
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
        "  aga -a MyAssemblyListing.agl -o MyGeneratedExecutable\n\n"

        " AgniAssembler comes with NO WARRANTY, to the extent permitted by\n"
        " law. You may redistribute copies of AgniAssembler. Just use your\n"
        " head.\n\n"

        " Shouts and thanks to Alex Varanese, MasterCAD, Curt, Dr. Knorr,\n"
        " Dr. Wolfman, Dr. Eiselt, Peter (TDLSoftware.org), Reed, RP,\n"
        " Sarah, Wayne, and the MinGW, GCC, Dev-C++, Code::Blocks, Gnome,\n"
        " Winamp, and XMMS crews.\n\n"

        " Written by Kip Warner. Questions or comments may be sent to\n"
        " Kip@TheVertigo.com. You can visit me out on the wasteland at\n"
        " http://TheVertigo.com." << std::endl << std::endl;
}

// Print version...
void Assembler::Parameters::PrintVersion() const
{
    // Version...
    std::cout << "AgniAssembler "   << AGNI_VERSION_MAJOR << "."
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
bool Assembler::Parameters::ShouldBeVerbose() const
{
    // Return flag...
    return bVerbose;
}

