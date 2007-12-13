/*
  Name:         CParser.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to take a source code vector and translate it into
                intermediate code (i-code). Provides the compiler front end...
*/

// Multiple include protection...
#ifndef _CPARSER_H_
#define _CPARSER_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"
        #include "../include/AgniCommonDefinitions.h"
        
        // Lexer class...
        #include "CLexer.h"
        
        // STL stuff...
        #include <vector>
        #include <string>
        #include <map>
        #include <list>
        #include <utility>
        #include <stack>

// Within the Agni namespace...
namespace Agni
{
    // CParser class definition...
    class CParser
    {
        // Public stuff...
        public:

            // Constants...

                // I-code instruction set for compiler front end...
                enum INSTRUCTION_ICODE
                {
                    // Main...
                    INSTRUCTION_ICODE_MOV = 1,

                    // Arithmetic...
                    INSTRUCTION_ICODE_ADD,        /* 2 */
                    INSTRUCTION_ICODE_SUB,
                    INSTRUCTION_ICODE_MUL,
                    INSTRUCTION_ICODE_DIV,
                    INSTRUCTION_ICODE_MOD,
                    INSTRUCTION_ICODE_EXP,
                    INSTRUCTION_ICODE_NEG,
                    INSTRUCTION_ICODE_INC,
                    INSTRUCTION_ICODE_DEC,

                    // Bitwise...
                    INSTRUCTION_ICODE_AND,        /* 11 */
                    INSTRUCTION_ICODE_OR,
                    INSTRUCTION_ICODE_XOR,
                    INSTRUCTION_ICODE_NOT,
                    INSTRUCTION_ICODE_SHL,
                    INSTRUCTION_ICODE_SHR,

                    // String manipulation...
                    INSTRUCTION_ICODE_CONCAT,     /* 17 */
                    INSTRUCTION_ICODE_GETCHAR,
                    INSTRUCTION_ICODE_SETCHAR,

                    // Branching...
                    INSTRUCTION_ICODE_JMP,        /* 20 */
                    INSTRUCTION_ICODE_JE,
                    INSTRUCTION_ICODE_JNE,
                    INSTRUCTION_ICODE_JG,
                    INSTRUCTION_ICODE_JL,
                    INSTRUCTION_ICODE_JGE,
                    INSTRUCTION_ICODE_JLE,

                    // Stack interface...
                    INSTRUCTION_ICODE_PUSH,       /* 27 */
                    INSTRUCTION_ICODE_POP,

                    // Function interface...
                    INSTRUCTION_ICODE_CALL,       /* 29 */
                    INSTRUCTION_ICODE_RET,
                    INSTRUCTION_ICODE_CALLHOST,

                    // Miscellaneous...
                    INSTRUCTION_ICODE_RAND,       /* 32 */
                    INSTRUCTION_ICODE_PAUSE,
                    INSTRUCTION_ICODE_EXIT
                };
                
                // Operand types for i-code...
                typedef enum ICodeOperandType
                {
                    OT_ICODE_INTEGER                = 0x01,
                    OT_ICODE_FLOAT,
                    OT_ICODE_INDEX_STRING,
                    OT_ICODE_VARIABLE,
                    OT_ICODE_INDEX_ARRAY_ABSOLUTE,
                    OT_ICODE_INDEX_ARRAY_VARIABLE,
                    OT_ICODE_INDEX_ARRAY_REGISTER,
                    OT_ICODE_INDEX_JUMP_TARGET,
                    OT_ICODE_INDEX_FUNCTION,
                    OT_ICODE_REGISTER
                };
                
                // Register i-codes...
                typedef enum ICodeRegister
                {
                    // Two general purpose registers...
                    REGISTER_ICODE_T0   = 0x01,
                    REGISTER_ICODE_T1,

                    // Return value...
                    REGISTER_ICODE_RETURN
                };

            // Data types...

                // Function name...
                typedef std::string                     FunctionName;

                // Instruction list index...
                typedef uint32                          InstructionListIndex;
                
                // String table index...
                typedef uint32                          StringTableIndex;

                // Scope...
                typedef uint32                          IdentifierScope;
                typedef enum Scope
                {
                    Global          = 0
                    /* i > Global to denote association with ith function */
                };

                // Variable name is a tuple of its identifier and scope...
                typedef std::pair<std::string, IdentifierScope>
                                                        VariableName;
                
                // Variable table index...
                typedef uint32                          VariableTableIndex;

                // I-code operation code...
                typedef uint16 ICodeOperationCode;
                
                // I-code operand...
                typedef struct
                {
                    // Operand type...
                    ICodeOperandType    Type;
                    
                    // The operand value itself...
                    union
                    {
                        // Literal integer...
                        int32                   nLiteralInteger;

                        // Literal float...
                        float32                 fLiteralFloat;

                        // String table index...
                        StringTableIndex        StringIndex;
                        
                        // Variable table index...
                        VariableTableIndex      VariableIndex;

                        // Jump target index...
                        InstructionListIndex    JumpTargetIndex;

                        // Function index...
                        IdentifierScope         FunctionIndex;

                        // Register identifier...
                        uint8                   Register;
                    };

                    // Incase array indexed with immediate value, goes here...
                    VariableTableIndex  Offset;
                    
                    // Incase array indexed with variable, variable index here...
                    VariableTableIndex  OffsetVariableIndex;
                
                }ICodeOperand;
                
                // I-code instruction...
                typedef struct
                {
                    // The instruction operation code...
                    ICodeOperationCode      OperationCode;
                    
                    // The variable list of operands...
                    std::list<ICodeOperand> OperandList;
                    
                }ICodeInstruction;
                
                // I-code node...
                typedef struct
                {
                    // Node type type...
                    typedef enum
                    {
                        // Line of the original source code for annotation...
                        ANNOTATION,

                        // I-code instruction...
                        INSTRUCTION,
                        
                        // Jump target index...
                        JUMP_TARGET

                    }NodeType;
                    
                    // Node type...
                    NodeType    Type;

                    // Node's data... (can't use union since string has constructor)

                        // I-code instruction...
                        ICodeInstruction        Instruction;
                        
                        // Line of the original source code or something else...
                        std::string             sAnnotation;
                        
                        // Jump target index...
                        InstructionListIndex    JumpTargetIndex;
                    
                }ICodeNode;

            // Classes...
            
                // Function class...
                class CFunction
                {
                    // Public stuff...
                    public:
                    
                        // Constructor...
                        CFunction(uint8 _Parameters, 
                                  boolean _bIsHostFunction)
                            :   Index(NextAvailableFunctionIndex++),
                                Parameters(_Parameters),
                                bIsHostFunction(_bIsHostFunction)
                        {
                        }
                        
                        // Get the function table index...
                        IdentifierScope GetIndex() const { return Index; };
                        
                        // Function table index...
                        const IdentifierScope   Index;
                        
                        // Parameter count...
                        uint8                   Parameters;
                        
                        // Agni function or external host function?
                        boolean                 bIsHostFunction;

                        // Function's i-code list...
                        std::list<ICodeNode>    ICodeList;

                    // Private stuff...
                    private:

                        // The index available in the table for a function...
                        static IdentifierScope NextAvailableFunctionIndex;
                };
                
                // Variable class...
                class CVariable
                {
                    // Public stuff...
                    public:
                    
                        // Datatypes...
                            
                            // Variable type...
                            typedef enum IdentifierType
                            {
                                Parameter  = 0,
                                Variable
                            };

                        // Constructor...
                        CVariable(uint32 _unSize, IdentifierType _Type)
                            :   Index(NextAvailableVariableIndex++),
                                unSize(_unSize), 
                                Type(_Type)
                        {
                        }
                        
                        // Get the variable table index...
                        VariableTableIndex GetIndex() const { return Index; };
                        
                        // Variable index...
                        const VariableTableIndex    Index;
                        
                        // Size...
                        uint32                      unSize;
                        
                        // Parameter or variable?
                        IdentifierType              Type;

                    // Private stuff...
                    private:

                        // Next available variable index...
                        static VariableTableIndex   NextAvailableVariableIndex;
                };

            // Methods...
            
                // Default constructor...
                CParser(std::vector<std::string> &InputSourceCode);
                
                // Generate complete i-code representation of entire source code...
                void Parse() throw(std::string const);
                
                // Get the main header...
                Agni_MainHeader const &GetMainHeader() const;
                
                // Get the function table...
                std::map<FunctionName, CFunction> const &GetFunctionTable() 
                    const;
                
                // Get the variable table...
                std::map<VariableName, CVariable> const &GetVariableTable() 
                    const;

        // Protected stuff...
        protected:

            // Data types...

                // Loop...
                typedef struct Loop
                {
                    // Loop's conditional evaluation begins here...
                    InstructionListIndex                StartTargetIndex;
                    
                    // Instruction following the last of the loop's body...
                    InstructionListIndex                EndTargetIndex;            
                };

            // Methods...

                // Add index into an array within a register as an operand...
                void AddArrayIndexRegisterICodeOperand(
                        IdentifierScope FunctionIndex,
                        InstructionListIndex InstructionIndex,
                        VariableTableIndex VariableIndex,
                        ICodeRegister Register)
                    throw(std::string const);

                // Add float operand to i-code instruction...
                void AddFloatICodeOperand(IdentifierScope FunctionIndex, 
                                          InstructionListIndex InstructionIndex,
                                          float32 fFloatOperand) 
                    throw(std::string const);

                // Add a function or throw an error...
                IdentifierScope AddFunction(FunctionName Name, 
                                            boolean bIsHostFunction) 
                    throw(std::string const);

                // Add function index operand to i-code instruction...
                void AddFunctionICodeOperand(
                    IdentifierScope const FunctionIndex, 
                    InstructionListIndex const InstructionIndex,
                    IdentifierScope const OperandFunctionIndex) 
                    throw(std::string const);

                // Add a line of source code or something else into the i-code for human...
                void AddICodeAnnotation(IdentifierScope FunctionIndex, 
                                        std::string const sAnnotation)
                    throw(std::string const);

                // Add i-code instruction to end of function, return index, or throw error...
                InstructionListIndex 
                    AddICodeInstruction(IdentifierScope FunctionIndex, 
                                        ICodeOperationCode OperationCode)
                    throw(std::string const);

                // Add a jump target to the i-code... (not an operand, but the target itself)
                void AddICodeJumpTarget(IdentifierScope FunctionIndex,
                                        InstructionListIndex JumpTargetIndex)
                    throw(std::string const);

                // Add i-code instruction's operand...
                void AddICodeOperand(IdentifierScope FunctionIndex, 
                                     InstructionListIndex InstructionIndex,
                                     ICodeOperand Operand) 
                    throw(std::string const);

                // Add integer operand to i-code instruction...
                void AddIntegerICodeOperand(IdentifierScope FunctionIndex, 
                                            InstructionListIndex InstructionIndex,
                                            int32 nIntegerOperand) 
                    throw(std::string const);

                // Add a jump target operand to i-code instruction...
                void AddJumpTargetICodeOperand(IdentifierScope FunctionIndex,
                                               InstructionListIndex InstructionIndex,
                                               InstructionListIndex JumpTargetIndex)
                    throw(std::string const);

                // Add register operand to i-code instruction...
                void AddRegisterICodeOperand(IdentifierScope FunctionIndex,
                                             InstructionListIndex InstructionIndex,
                                             ICodeRegister Register) 
                    throw(std::string const);

                // Add a string, ignoring duplicates, and return index...
                StringTableIndex const AddString(std::string const sString);

                // Add string operand to i-code instruction...
                void AddStringICodeOperand(IdentifierScope FunctionIndex, 
                                           InstructionListIndex InstructionIndex,
                                           StringTableIndex StringIndex) 
                    throw(std::string const);

                // Add a variable or throw an error...
                uint32 AddVariable(VariableName Name, uint32 unSize, 
                                   CVariable::IdentifierType Type) 
                    throw(std::string const);

                // Add variable operand to i-code instruction...
                void AddVariableICodeOperand(IdentifierScope FunctionIndex, 
                                             InstructionListIndex InstructionIndex,
                                             VariableTableIndex VariableIndex) 
                    throw(std::string const);

                // Get function via index, or throw an error...
                CFunction &GetFunctionByIndex(IdentifierScope Index) 
                    const throw(std::string const);

                // Get function via name, or throw an error...
                CFunction &GetFunctionByName(FunctionName Name) 
                    throw(std::string const);

                // Get an i-code node from within a function at the specified instruction...
                ICodeNode &
                    GetICodeNodeByImplicitIndex(
                        IdentifierScope FunctionIndex, 
                        InstructionListIndex InstructionIndex) 
                    const throw(std::string const);

                // Get the next jump target index...
                InstructionListIndex GetNextJumpTargetIndex();

                // Get a string by index...
                std::string const GetStringByIndex(StringTableIndex const Index) 
                    const;

                // Get variable by name at requested scope / global, or throw 
                //  an error...
                CVariable &GetVariableByName(VariableName Name) 
                    throw(std::string const);
                
                // Get variable by index, or throw an error...
                CVariable &GetVariableByIndex(VariableTableIndex Index) 
                    const throw(std::string const);

                // Get size of variable using name as key...
                uint32 GetVariableSize(VariableName Name) 
                    throw(std::string const);

                // Is this a function in the function table?
                boolean IsFunctionInTable(FunctionName Name) const;
                
                // Is this a variable in the variable table at requested 
                //  scope / global?
                boolean IsVariableInTable(VariableName Name) const;

                // Is operator an assignment?
                boolean IsOperatorAssignment(
                    CLexer::Operator const CurrentOperator) const;

                // Is operator logical?
                boolean IsOperatorLogical(
                    CLexer::Operator const CandidateOperator) const;

                // Is the operator relational?
                boolean IsOperatorRelational(
                    CLexer::Operator const CandidateOperator) const;

                // Parse the various valid forms of token sequences...
                void ParseAssignment() throw(std::string const);
                void ParseBreak() throw(std::string const);
                void ParseCodeBlock() throw(std::string const);
                void ParseContinue() throw(std::string const);
                void ParseExpression() throw(std::string const);
                void ParseFactor() throw(std::string const);
                void ParseFor() throw(std::string const);
                void ParseFunction() throw(std::string const);
                void ParseFunctionCall() throw(std::string const);
                void ParseHost() throw(std::string const);
                void ParseIf() throw(std::string const);
                void ParseReturn() throw(std::string const);
                void ParseSubExpression() throw(std::string const);
                void ParseStatement() throw(std::string const);
                void ParseTerm() throw(std::string const);
                void ParseVariable() throw(std::string const);
                void ParseWhile() throw(std::string const);

                // Read a token and verify it is what was expected...
                void ReadToken(const CLexer::Token Expected) 
                    throw(std::string const);

            // Variables...
                
                // Source code vector...
                std::vector<std::string>               &SourceCode;
                
                // Lexer...
                CLexer                                  Lexer;
                
                // Loop stack...
                std::stack<Loop>                        LoopStack;
                
                // Current scope...
                IdentifierScope                         CurrentScope;
                
                // Function table... (<identifier, #params> key)
                std::map<FunctionName, CFunction>       FunctionTable_KeyByName;
                
                // Function table with indices as keys...
                std::map<IdentifierScope, CFunction *>  FunctionTable_KeyByIndex;
                
                // Variable table... (<identifier, scope> key)
                std::map<VariableName, CVariable>       VariableTable_KeyByName;
                
                // Variable table with indices as keys... (<index> key)
                std::map<VariableTableIndex, CVariable *>
                    VariableTable_KeyByIndex;
                
                // String table...
                std::vector<std::string>                StringTableVector;
                std::map<std::string, StringTableIndex> StringTableMap;

                // Script's main header...
                Agni_MainHeader                         MainHeader;
                
                // Next variable index...
                VariableTableIndex                      NextVariableIndex;
                
                // Current jump target index...
                InstructionListIndex                    CurrentJumpTargetIndex;
    };
}

#endif

