/*
  Name:         CAgniAssembler.h (definition)
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Routines to produce an Agni executable from an Agni assembly
                listing...
*/

// To-do...

    /* TODO (Kip#2#): Add a C API binding somehow */

    /* TODO (Kip#2#): Port to Sun OS. */

// Multiple include protection...
#ifndef _CAGNIASSEMBLER_H_
#define _CAGNIASSEMBLER_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"

        // Standard I/O...
        #include <stdio.h>

        // Standard library...
        #include <stdlib.h>

        // Standard argument routines...
        #include <stdarg.h>

        // String processing...
        #include <string.h>

// CAgniAssembler class definition...
class CAgniAssembler
{
    // Public data types...
    public:

        // Assembler interface parameters...
        typedef struct
        {
            // Name of assembler...
            char    szAssemblerName[1024];

            // Input file name...
            char    szInputFile[1024];

            // Optimization level...
            uint8   ucOptimization;

            // Output file name...
            char    szOutputFile[1024];

            // Verbosity...
            boolean bVerbose;

        }AssemblerParameters;

    // Executable structures, lexical analysis, and tokenization definitions...
    protected:

        // Common structures...
        #include "../include/AgniCommonDefinitions.h"

        // Operand type domain bitfield mask codes... (for parser)
        enum OTD
        {
            OTD_INTEGER              = 1,
            OTD_FLOAT                = 2,
            OTD_STRING               = 2*2,
            OTD_MEMORY_REFERENCE     = 2*2*2,
            OTD_LINE_LABEL           = 2*2*2*2,
            OTD_FUNCTION_NAME        = 2*2*2*2*2,
            OTD_HOST_FUNCTION_NAME   = 2*2*2*2*2*2,
            OTD_REGISTER             = 2*2*2*2*2*2*2
        };

        // Operand type type...
        typedef int32 AA_OperandType;

        // A single instruction structure...
        typedef struct _AA_InstructionLookup
        {
            // Mnemonic...
            char            szMnemonic[16];

            // Operation code...
            uint16          usOperationCode;

            // Operand count...
            uint8           OperandCount;

            // Operand type list...
            AA_OperandType *pOperandList;

        }AA_InstructionLookup;

        // Operand structure...
        typedef struct _AA_Operand
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

                // String table index...
                int32           nStringTableIndex;

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

        }AA_Operand;

        // Instruction structure...
        typedef struct _AA_Instruction
        {
            // Operation code...
            uint16              usOperationCode;

            // Operand count...
            uint8               OperandCount;

            // Operand list...
            AA_Operand         *pOperandList;

        }AA_Instruction;

        // Token data type...
        typedef uint32 Token;

        // Token types...
        enum TOKEN
        {
            // Single character tokens...
            TOKEN_QUOTE,
            TOKEN_COMMA,
            TOKEN_COLON,
            TOKEN_OPEN_BRACKET,
            TOKEN_CLOSE_BRACKET,
            TOKEN_NEW_LINE,
            TOKEN_OPEN_BRACE,
            TOKEN_CLOSE_BRACE,

            // Multi-character tokens...
            TOKEN_INTEGER,
            TOKEN_FLOAT,
            TOKEN_IDENTIFIER,
            TOKEN_INSTRUCTION,
            TOKEN_DIRECTIVE_SETHOST,
            TOKEN_DIRECTIVE_SETSTACKSIZE,
            TOKEN_DIRECTIVE_SETTHREADPRIORITY,
            TOKEN_VAR,
            TOKEN_FUNC,
            TOKEN_PARAM,
            TOKEN_STRING,
            TOKEN_REGISTER_T0,
            TOKEN_REGISTER_T1,
            TOKEN_REGISTER_RETURN,

            // Miscellaneous...
            TOKEN_INVALID,
            TOKEN_END
        };

        // Lexer states...
        enum LEXER
        {
            // Sttring stuff...
            LEXER_NO_STRING,
            LEXER_IN_STRING,
            LEXER_END_STRING
        };

    // Data types...
    protected:

        // Linked list node structure...
        typedef struct _AA_LinkedListNode
        {
            // Data...
            void               *pData;

            // Next node...
            _AA_LinkedListNode *pNext;

        }AA_LinkedListNode;

        // Linked list structure...
        typedef struct _AA_LinkedList
        {
            // Head and tail node pointers...
            AA_LinkedListNode  *pHead;
            AA_LinkedListNode  *pTail;

            // Node count...
            uint32              unNodes;

        }AA_LinkedList;

        // Function node...
        typedef struct _AA_FunctionNodeData
        {
            // Index...
            uint32          unIndex;

            // Name...
            char            szName[256];

            // Entry point...
            uint32          unEntryPoint;

            // Parameter count...
            uint8           ParameterCount;

            // Local data size...
            uint32          unLocalDataSize;

        }AA_FunctionNodeData;

        // Symbol node data...
        typedef struct _AA_SymbolNodeData
        {
            // Index...
            uint32          unIndex;

            // Identifier...
            char            szIdentifier[256];

            // Size of identifier... (usually 1, but arrays are of size N)
            uint32          unSize;

            // Stack index which this identifier points to...
            int32           nStackIndex;

            // Function in which symbol resides...
            uint32          unFunctionIndex;

        }AA_SymbolNodeData;

        // Label node data...
        typedef struct _AA_LabelNodeData
        {
            // Index...
            uint32          unIndex;

            // Identifier...
            char            szIdentifier[256];

            // Target instruction index...
            uint32          unTargetIndex;

            // Function index...
            uint32          unFunctionIndex;

        }AA_LabelNodeData;

    // Public stuff...
    public:

        // Constructor...
        CAgniAssembler(AssemblerParameters *pParameters);

        // Assemble listing...
        boolean Assemble();

        // Deconstructor...
       ~CAgniAssembler();

    // Protected methods...
    protected:

        // Instruction management...

            // Add instruction and return index, or -1 on error
            int AddInstruction(const char *pszMnemonic, uint16 usOperationCode,
                               uint8 OperandCount);

            // Get instruction by mnemonic...
            boolean GetInstructionByMnemonic(const char *pszMnemonic,
                                             AA_InstructionLookup *pInstructionLookup) const;

            // Get the instruction set size...
            uint16 GetInstructionSetSize();

            // Set instruction operand type...
            void SetOperandType(uint16 usInstructionIndex, uint8 OperandIndex,
                                AA_OperandType Type);

        // Generic character processing routines...

            // Is character part of a delimeter?
            boolean IsCharacterDelimiter(char cCharacter) const;

            // Is character valid for being part of an identifier?
            boolean IsCharacterIdentifier(char cCharacter) const;

            // Is character numeric?
            boolean IsCharacterNumeric(char cCharacter) const;

            // Is character white space?
            boolean IsCharacterWhiteSpace(char cCharacter) const;

        // Generic string processing routines...

            // Is string a float?
            boolean IsStringFloat(const char *pszString) const;

            // Is string a valid identifier?
            boolean IsStringIdentifier(const char *pszString) const;

            // Is string an integer?
            boolean IsStringInteger(const char *pszString) const;

            // Is string white space?
            boolean IsStringWhiteSpace(const char *pszString) const;

        // Error control...

            // General error...
            void ErrorGeneral(const char *pszFormat, ...);

            // Error due to listing...
            void ErrorListing(const char *pszFormat, ...);

            // Expected a character, but did not find...
            void ErrorExpectedCharacter(char cExpected);

            // Output some verbose stuff...
            void ErrorVerbose(const char *pszFormat, ...);

            // Warning...
            void ErrorWarning(const char *pszFormat, ...);

            // Cleanup all resources, reset state, and shutdown assembler...
            void ShutDown();

        // Lexical anaylsis / tokenization...

            // Get the current lexeme...
            const char *GetCurrentLexeme() const;

            // Get the current lexeme and store for caller...
            char *GetCurrentLexeme(char *pszBuffer) const;

            // Get the current token...
            Token GetCurrentToken() const;

            // Get the first character of next token...
            char GetLookAheadCharacter() const;

            // Get the next token...
            Token GetNextToken();

            // Seek to the next line, if there is one...
            boolean SeekToNextLine();

            // Strip comments from source line...
            void StripComments(char *pszSourceLine);

            // Trim white space from left and right side...
            void TrimWhiteSpace(char *pszSourceLine);

            // Reset lexer...
            void ResetLexer();

        // Linked list management...

            // Add node to linked list and return index...
            uint32 List_Add(AA_LinkedList *pList, void *pData);

            // Initialize linked list...
            void List_Initialize(AA_LinkedList *pList);

            // Free linked list...
            void List_Free(AA_LinkedList *pList);

        // Miscellaneous...

            // Display statistics...
            void DisplayStatistics() const;

            // Load input file, throw string on error...
            void LoadInput();

        // Function management...

            // Add function and return index, or -1 on error...
            int AddFunction(const char *pszName, uint32 unEntryPoint);

            // Get function node's data by name...
            AA_FunctionNodeData *GetFunctionByName(const char *pszName);

            // Set function parameter count and local data size...
            void SetFunctionInfo(const char *pszName, uint8 ParameterCount,
                                 uint32 unLocalDataSize);

        // Label management...

            // Add a label and return index, or -1 on error...
            int AddLabel(const char *pszIdentifier, uint32 unTargetIndex,
                         uint32 unFunctionIndex);

            // Get a label by identifier...
            AA_LabelNodeData *GetLabelByIdentifier(const char *pszIdentifier,
                                                   uint32 unFunctionIndex) const;

        // String management...

            // Add string to list and return index...
            uint32 AddString(AA_LinkedList *pList, const char *pszString);

        // Symbol management...

            // Add symbol, or -1 on error...
            int AddSymbol(const char *pszIdentifier, uint32 unSize,
                          int nStackIndex, uint32 unFunctionIndex);

            // Get symbol size by identifier...
            uint32 GetSizeByIdentifier(const char *pszIdentifier,
                                       uint32 unFunctionIndex) const;

            // Get the stack index by identifier...
            int GetStackIndexByIdentifier(const char *pszIdentifier,
                                          uint32 unFunctionIndex) const;

            // Get symbol data by identifier...
            AA_SymbolNodeData *GetSymbolByIdentifier(const char *pszIdentifier,
                                                     uint32 unFunctionIndex) const;

        // Disk management...

            // Write executable to disk, throw string on error...
            void WriteExecutable();

            // Buffers bytes in memory, throw string on error...
            void Write_BufferBytes(const void *pBuffer, size_t ulBytes);

        // Checksum calculation...

            // Acknowledge bit in calculation...
            void CheckSum_PutBit(boolean Bit);

            // Acknowledge byte in calculation...
            void CheckSum_PutByte(uint8 Byte);

            // Acknowledge stream of bytes in calculation...
            void CheckSum_PutBytes(uint8 *pBytes, uint32 unSize);

        // Variables...

            // Assembler interface parameters...
            AssemblerParameters     Parameters;

            // Listing...
            char                  **ppszListing;
            uint32                  unListingLines;

            // Tables...

                // Instruction table...
                AA_InstructionLookup            InstructionTable[256];
                uint16                          usInstructionCount;

                // Function table...
                AA_LinkedList                   FunctionTable;

                // Host function table...
                AA_LinkedList                   HostFunctionTable;

                // Instruction stream...
                Agni_InstructionStreamHeader    InstructionStreamHeader;
                AA_Instruction                 *pInstructionStream;

                // Label table...
                AA_LinkedList                   LabelTable;

                // String table...
                AA_LinkedList                   StringTable;

                // Symbol table...
                AA_LinkedList                   SymbolTable;

            // Lexical analysis / tokenizer...
            uint8                   LexerState;
            uint32                  unLexerIndex0;
            uint32                  unLexerIndex1;
            uint32                  unLexerLine;
            uint32                  unLexerCurrentToken;
            char                    szCurrentLexeme[1024];

            // Executable header...
            Agni_MainHeader         MainHeader;

            // Output buffer...
            uint32                  unOutputBufferAllocated;
            uint8                  *pOutputBuffer;

            // Checksum temporary scratch pad...
            uint32                  unTempCheckSum;
};

#endif
