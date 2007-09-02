/*
  Name:         AgniCommonDefinitions.h
  Copyright:    Kip Warner (Kip@TheVertigo.com)
  Description:  Definitons for structures and constants that are used
*/

// Includes...

    // Cross platform types...
    #include "AgniPlatformSpecific.h"

// Versioning...

    // Major and minor assemble / compile time Agni version...
    #define AGNI_VERSION_MAJOR          0
    #define AGNI_VERSION_MINOR          94

    // Major and minor version numbers of minimum acceptable runtime...
    #define AGNI_VERSION_NEEDED_MAJOR   AGNI_VERSION_MAJOR
    #define AGNI_VERSION_NEEDED_MINOR   AGNI_VERSION_MINOR

// Default file extensions...

    // Assembly listing...
    #define AGNI_FILE_EXTENSION_LISTING         "agl"
    
    // Source code...
    #define AGNI_FILE_EXTENSION_SOURCE          "ags"
    
    // Source code header...
    #define AGNI_FILE_EXTENSION_SOURCE_HEADER   "agh"

    // Virtual machine executable...
    #define AGNI_FILE_EXTENSION_EXECUTABLE      "age"

// Default entry point function in script...
#define AGNI_DEFAULT_ENTRY_FUNCTION             "Main"

// Structures found in the Agni executables...

    // Main header...
    typedef struct AGNI_ATTRIBUTE_PACKED
    {
        // Agni executable signature...
        char            Signature[8];

        // Major and minor version numbers at assemble / compile time...
        uint8           ucMajorAgniVersion;
        uint8           ucMinorAgniVersion;

        // Major and minor version numbers of minimum acceptable runtime...
        uint8           ucMajorRequiredAgniVersion;
        uint8           ucMinorRequiredAgniVersion;

        // Host name string index (or -1), major, and minor version for
        //  AgniVirtualMachine backend. eg "Gandalf Quest", 0x01, 0x02...
        uint32          unHostStringIndex;
        uint8           ucHostMajorVersion;
        uint8           ucHostMinorVersion;

        // Checksum of entire executable, assuming this field to be zero...
        uint32          unCheckSum;

        // Stack size... (-1 sets default size)
        uint32          unStackSize;

        // Global data size...
        uint32          unGlobalDataSize;

        // Main function information... (-1 if not present)
        uint32          unMainIndex;

        // Thread priority type...
        uint8           ThreadPriorityType;
        uint32          unThreadPriorityUser;

    }Agni_MainHeader;

    // Instruction stream header...
    typedef struct _Agni_InstructionStreamHeader
    {
        // Number of instructions...
        uint32          unSize;

    }Agni_InstructionStreamHeader;

    // String stream header...
    typedef struct _Agni_StringStreamHeader
    {
        // Number of strings in stream...
        uint32  unSize;

    }Agni_StringStreamHeader;

    // Function table header...
    typedef struct _Agni_FunctionTableHeader
    {
        // Number of functions in table...
        uint32 unSize;

    }Agni_FunctionTableHeader;

    // Function structure...
    typedef struct _Agni_Function
    {
        // Entry point...
        uint32          unEntryPoint;

        // Parameter count...
        uint8           ParameterCount;

        // Total local data size...
        uint32          unLocalDataSize;

        // Total stack frame size...
        uint32          unStackFrameSize;

        // Function name...
        char            szName[256];

    }Agni_Function;

    // Host function table header...
    typedef struct _Agni_HostFunctionTableHeader
    {
        // Number of host functions in table...
        uint32 unSize;

    }Agni_HostFunctionTableHeader;

    // Host function...
    typedef struct _Agni_HostFunction
    {
        // Function names...
        char    szName[256];

    }Agni_HostFunction;

// Components of the virtual hardware...

    // Agni virtual machine instruction set...
    enum INSTRUCTION_AVM
    {
        // Main...
        INSTRUCTION_AVM_MOV = 1,

        // Arithmetic...
        INSTRUCTION_AVM_ADD,        /* 2 */
        INSTRUCTION_AVM_SUB,
        INSTRUCTION_AVM_MUL,
        INSTRUCTION_AVM_DIV,
        INSTRUCTION_AVM_MOD,
        INSTRUCTION_AVM_EXP,
        INSTRUCTION_AVM_NEG,
        INSTRUCTION_AVM_INC,
        INSTRUCTION_AVM_DEC,

        // Bitwise...
        INSTRUCTION_AVM_AND,        /* 11 */
        INSTRUCTION_AVM_OR,
        INSTRUCTION_AVM_XOR,
        INSTRUCTION_AVM_NOT,
        INSTRUCTION_AVM_SHL,
        INSTRUCTION_AVM_SHR,

        // String manipulation...
        INSTRUCTION_AVM_CONCAT,     /* 17 */
        INSTRUCTION_AVM_GETCHAR,
        INSTRUCTION_AVM_SETCHAR,

        // Branching...
        INSTRUCTION_AVM_JMP,        /* 20 */
        INSTRUCTION_AVM_JE,
        INSTRUCTION_AVM_JNE,
        INSTRUCTION_AVM_JG,
        INSTRUCTION_AVM_JL,
        INSTRUCTION_AVM_JGE,
        INSTRUCTION_AVM_JLE,

        // Stack interface...
        INSTRUCTION_AVM_PUSH,       /* 27 */
        INSTRUCTION_AVM_POP,

        // Function interface...
        INSTRUCTION_AVM_CALL,       /* 29 */
        INSTRUCTION_AVM_RET,
        INSTRUCTION_AVM_CALLHOST,

        // Miscellaneous...
        INSTRUCTION_AVM_RAND,       /* 32 */
        INSTRUCTION_AVM_PAUSE,
        INSTRUCTION_AVM_EXIT
    };

    // Operand type codes for assembled executable instruction stream...
    enum OT_AVM
    {
        OT_AVM_NULL = 0x00,
        OT_AVM_INTEGER,
        OT_AVM_FLOAT,
        OT_AVM_INDEX_STRING,
        OT_AVM_STRING,
        OT_AVM_INDEX_STACK_ABSOLUTE,
        OT_AVM_INDEX_STACK_RELATIVE,
        OT_AVM_INDEX_INSTRUCTION,
        OT_AVM_INDEX_FUNCTION,
        OT_AVM_INDEX_FUNCTION_HOST,
        OT_AVM_REGISTER,
        OT_AVM_STACK_BASE_MARKER
    };

    // Threading priorities...
    enum THREAD_PRIORITY
    {
        THREAD_PRIORITY_USER = 0,
        THREAD_PRIORITY_LOW,
        THREAD_PRIORITY_MEDIUM,
        THREAD_PRIORITY_HIGH,
        THREAD_PRIORITY_INFINITE = (unsigned) -1
    };

    // Registers...
    enum REGISTER_AVM
    {
        // Two general purpose registers...
        REGISTER_AVM_T0     = 0x01,
        REGISTER_AVM_T1,

        // Return value...
        REGISTER_AVM_RETURN
    };

// Miscellaneous...

    // Cyclic redundancy check checksum key... (Ethernet, PKZip, etc)
    #define AGNI_CHECKSUM_KEY 0x04c11db7
