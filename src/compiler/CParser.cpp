/*
  Name:         CParser.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to take a source code vector and translate it into
                intermediate code. Provides the compiler front end...
*/

// Includes...
#include "CParser.h"
#include <cassert>

// Using the Agni namepace...
using namespace Agni;

// Initialize static data...

    // Function indicies begin at one, since zero denotes global scope...
    CParser::IdentifierScope 
        CParser::CFunction::NextAvailableFunctionIndex = 1;

    // Variable indicies begin at zero...
    CParser::VariableTableIndex 
        CParser::CVariable::NextAvailableVariableIndex = 0;

// Default constructor...
CParser::CParser(std::vector<std::string> &InputSourceCode)
    : SourceCode(InputSourceCode),
      Lexer(InputSourceCode),
      CurrentJumpTargetIndex(0)
{    
    // Clear out script header...
    memset(&MainHeader, 0, sizeof(Agni_MainHeader));
}

// Add index into an array within a register as an operand...
void CParser::AddArrayIndexRegisterICodeOperand(
    IdentifierScope FunctionIndex,
    InstructionListIndex InstructionIndex,
    VariableTableIndex VariableIndex,
    ICodeRegister Register) throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to array index within register...
    Operand.Type            = OT_ICODE_INDEX_ARRAY_REGISTER;
    Operand.VariableIndex   = VariableIndex;
    Operand.OffsetRegister  = Register;

    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add float operand to i-code instruction...
void CParser::AddFloatICodeOperand(
    IdentifierScope FunctionIndex, 
    InstructionListIndex InstructionIndex,
    float32 fFloatOperand) throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to float...
    Operand.Type            = OT_ICODE_FLOAT;
    Operand.fLiteralFloat   = fFloatOperand;

    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add a function or throw an error...
CParser::IdentifierScope CParser::AddFunction(FunctionName Name,
                                              boolean bIsHostFunction)
    throw(std::string const)
{
    // Check to see if function is already in function table...
    if(FunctionTable_KeyByName.find(Name) != FunctionTable_KeyByName.end())
        throw "function " + Name + " redeclared";

    // Add new function to table...

        // Create a new function and assume no parameters for now...
        CFunction NewFunction(Name, 0, bIsHostFunction);

        // Now use the name and parameter count pair as hash key...
        FunctionTable_KeyByName.insert(make_pair(Name, NewFunction));

        // Also add this to our other function table using the index as key...
        
            // Since we want a reference, get location of function in KeyByName
            //  hash table...

                // We need an iterator...
                std::map<FunctionName, CFunction>::iterator Location;

                // Find the function...
                Location = FunctionTable_KeyByName.find(Name);

                    // Not found... (this should never happen)
                    assert(Location != FunctionTable_KeyByName.end());

            // Now add reference in the table keyed by index...
            FunctionTable_KeyByIndex.insert(
                std::make_pair(NewFunction.GetIndex(), &Location->second));

    // Return the new function's index...
    return NewFunction.GetIndex();
}

// Add a line of source code or something else into the i-code for human...
void CParser::AddICodeAnnotation(IdentifierScope FunctionIndex, 
                                 std::string const &sAnnotation)
    throw(std::string const)
{
    // Make a local copy so we can edit it...
    std::string sLocal = sAnnotation;

    // Trim out any leading white space...
    for(std::string::iterator Iterator = sLocal.begin();
        Iterator != sLocal.end();
      ++Iterator)
    {
        // First non-white space found...
        if(!CLexer::IsCharacterWhiteSpace(*Iterator))
        {
            // Make sure this wasn't at the beginning...
            if(Iterator == sLocal.begin())
                break;
            
            // Otherwise, strip out the white space...
            else
            {
                // Strip...
                sLocal.erase(sLocal.begin(), Iterator);
                
                // Done...
                break;
            }
        }
    }

    // Locate the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);

    // Create the appropriate i-node for the annotation...
    ICodeNode AnnotationNode;
    AnnotationNode.Type         = ICodeNode::ANNOTATION;
    AnnotationNode.sAnnotation  = sLocal;
    
    // Now add node to the requested function's i-code sequence...
    Function.ICodeList.push_back(AnnotationNode);
}

// Add i-code instruction to end of function, return index, or throw error...
CParser::InstructionListIndex CParser::AddICodeInstruction(
    IdentifierScope FunctionIndex, ICodeOperationCode OperationCode)
    throw(std::string const)
{
    // Variables...
    ICodeNode           Node;

    // Find the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);
    
    // Initialize the node for an instruction...
    Node.Type = ICodeNode::INSTRUCTION;

    // Now initialize the instruction with requested operation code, but empty 
    //  operand list...
    Node.Instruction.OperationCode = OperationCode;
    
    // Add instruction to function's i-code list...
    Function.ICodeList.push_back(Node);
    
    // Now return the instruction's index... (begin at zero)
    return Function.ICodeList.size() - 1;
}

// Add a jump target to the i-code... (not an operand, but the target itself)
void CParser::AddICodeJumpTarget(IdentifierScope FunctionIndex,
                                 InstructionListIndex JumpTargetIndex)
    throw(std::string const)
{
    // Variables...
    ICodeNode           Node;

    // Find the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);
    
    // Initialize the node for a jump target type...
    Node.Type = ICodeNode::JUMP_TARGET;

    // Now initialize the node with the actual jump target index...
    Node.JumpTargetIndex = JumpTargetIndex;
    
    // Add jump target to function's i-code list...
    Function.ICodeList.push_back(Node);
}

// Add i-code instruction's operand...
void CParser::AddICodeOperand(IdentifierScope FunctionIndex,
                              InstructionListIndex InstructionIndex,
                              ICodeOperand Operand) throw(std::string const)
{
    // Find the node...
    ICodeNode &Node = GetICodeNodeByImplicitIndex(
        FunctionIndex, InstructionIndex);

    // Verify that this is an instruction...
    if(Node.Type != ICodeNode::INSTRUCTION)
        throw std::string("internal fault, cannot add operand to"
                          " non-instructional node");

    // Now set the operand...
    Node.Instruction.OperandList.push_back(Operand);
}

// Add function index operand to i-code instruction...
void CParser::AddFunctionICodeOperand(IdentifierScope const FunctionIndex, 
    InstructionListIndex const InstructionIndex,
    IdentifierScope const OperandFunctionIndex) 
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to integer...
    Operand.Type            = OT_ICODE_INDEX_FUNCTION;
    Operand.FunctionIndex   = OperandFunctionIndex;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}


// Add integer operand to i-code instruction...
void CParser::AddIntegerICodeOperand(IdentifierScope FunctionIndex,
                                     InstructionListIndex InstructionIndex,
                                     int32 nIntegerOperand) 
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to integer...
    Operand.Type            = OT_ICODE_INTEGER;
    Operand.nLiteralInteger = nIntegerOperand;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add a jump target operand to i-code instruction...
void CParser::AddJumpTargetICodeOperand(IdentifierScope FunctionIndex,
                                        InstructionListIndex InstructionIndex,
                                        InstructionListIndex JumpTargetIndex)
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to jump target index...
    Operand.Type            = OT_ICODE_INDEX_JUMP_TARGET;
    Operand.JumpTargetIndex = JumpTargetIndex;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);    
}

// Add register operand to i-code instruction...
void CParser::AddRegisterICodeOperand(IdentifierScope FunctionIndex,
                                      InstructionListIndex InstructionIndex,
                                      ICodeRegister Register) 
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to register index...
    Operand.Type            = OT_ICODE_REGISTER;
    Operand.Register        = Register;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add a string, ignoring duplicates, and return index...
CParser::StringTableIndex const CParser::AddString(std::string const sString)
{
    // Look for the string...
    std::map<std::string, StringTableIndex>::const_iterator MapIterator
        = StringTableMap.find(sString);

    // The string is not already added...
    if(MapIterator == StringTableMap.end())
    {
        // Add to the string table vector...
        StringTableVector.push_back(sString);
        
        // Now make a note of the string's index in the vector...
        StringTableMap[sString] = StringTableVector.size() - 1;
        
        // Return the index...
        return StringTableMap[sString];
    }

    // The string has already been added, return the index...
    else
        return (*MapIterator).second;
    
}

// Add string operand to i-code instruction...
void CParser::AddStringICodeOperand(IdentifierScope FunctionIndex,
                                    InstructionListIndex InstructionIndex,
                                    StringTableIndex StringIndex) 
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to string index...
    Operand.Type        = OT_ICODE_INDEX_STRING;
    Operand.StringIndex = StringIndex;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add variable operand to i-code instruction...
void CParser::AddVariableICodeOperand(IdentifierScope FunctionIndex,
                                      InstructionListIndex InstructionIndex,
                                      VariableTableIndex VariableIndex) 
    throw(std::string const)
{
    // Variables...
    ICodeOperand    Operand;

    // Initialize operand to string index...
    Operand.Type            = OT_ICODE_VARIABLE;
    Operand.VariableIndex   = VariableIndex;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add a variable or throw an error...
CParser::VariableTableIndex CParser::AddVariable(VariableName Name,
                                                 uint32 unSize,
                                                 CVariable::IdentifierType Type)
    throw(std::string const)
{
    // Check if the <identifier, local scope / Global> already exist in table...
    if(IsVariableInTable(Name))
        throw "variable '" + Name.first + "' already declared";

    // Add new variable to table...

        // Create new variable and set its attributes...
        CVariable NewVariable(Name, unSize, Type);
    
        // Add it to the variable table using name as hash key...
        VariableTable_KeyByName.insert(std::make_pair(Name, NewVariable));

        // Also add this to our other variable table using index as key...

            // Since we want a reference, get location of function in KeyByName
            //  hash table...

                // We need an iterator...
                std::map<VariableName, CVariable>::iterator Location;

                // Find the variable...
                Location = VariableTable_KeyByName.find(Name);

                    // Not found... (this should never happen)
                    assert(Location != VariableTable_KeyByName.end());

            // Now add reference in the table keyed by index...
            VariableTable_KeyByIndex.insert(
                std::make_pair(NewVariable.GetIndex(), &Location->second));
    
    // Return the variable index to caller...
    return NewVariable.GetIndex();
}

// Get function via index, or throw an error...
CParser::CFunction &CParser::GetFunctionByIndex(IdentifierScope Index) 
    const throw(std::string const)
{
    // Variables...
    std::map<IdentifierScope, CFunction *>::const_iterator  Location;

    // Find the function...
    Location = FunctionTable_KeyByIndex.find(Index);

        // Not found...
        if(Location == FunctionTable_KeyByIndex.end())
            throw std::string("internal fault, function index not found");
          
    // Return the function object...
    return *(Location)->second;
}

// Get function via name, or throw an error...
CParser::CFunction &CParser::GetFunctionByName(FunctionName Name) 
    throw(std::string const)
{
    // Variables...
    std::map<FunctionName, CFunction>::iterator Location;

    // Find the function...
    Location = FunctionTable_KeyByName.find(Name);

        // Not found...
        if(Location == FunctionTable_KeyByName.end())
            throw std::string("internal fault, function name not found");

    // Return the function object...
    return Location->second;
}

// Get the function table...
std::map<Agni::CParser::FunctionName, Agni::CParser::CFunction> const &
    CParser::GetFunctionTable() const
{
    // Return constant reference...
    return FunctionTable_KeyByName;
}

// Get an i-code node from within a function at the specified instruction...
CParser::ICodeNode &CParser::GetICodeNodeByImplicitIndex(
    IdentifierScope FunctionIndex, InstructionListIndex InstructionIndex) 
    const throw(std::string const)
{
    // Variables...
    InstructionListIndex    Index    = 0;

    // First find the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);
    
    // Seek to index...
    std::list<ICodeNode>::iterator Location = Function.ICodeList.begin();
    while(Index != InstructionIndex)
    {
        // Out of bounds...
        if(Location == Function.ICodeList.end())
            throw std::string("internal fault, instruction index out of bounds");

        // Not there yet, try again...
      ++Index;
      ++Location;
    }

    // Now return the address of the node to the caller...
    return *Location;
}

// Get the next jump target index...
CParser::InstructionListIndex CParser::GetNextJumpTargetIndex()
{
    // Return the next available jump target...
    return CurrentJumpTargetIndex++;
}

// Get a string by index...
std::string const CParser::GetStringByIndex(
    CParser::StringTableIndex const Index) const
{
    // The index can't ever be out of bounds...
    assert(Index <= StringTableVector.size() - 1);
    
    // Return it...
    return StringTableVector.at(Index);
}

// Get variable by name at requested scope / global, or throw an error...
CParser::CVariable &CParser::GetVariableByName(VariableName Name) 
    throw(std::string const)
{
    // Variables...
    std::map<VariableName, CVariable>::iterator Location;

    // Try to find the variable first with requested scope...
    if(VariableTable_KeyByName.end() != 
       (Location = VariableTable_KeyByName.find(Name)))
        return Location->second;

    // Not found, so try global scope then...
    else if(VariableTable_KeyByName.end() != 
            (Location = VariableTable_KeyByName.find(
                std::make_pair(Name.first, Global))))
        return Location->second;

    // Not found in either local or global scope...
    else
        throw Name.first + "was not declared";
}
            
// Get variable by index, or throw an error...
CParser::CVariable &CParser::GetVariableByIndex(VariableTableIndex Index)
    const throw(std::string const)
{
    // Variables...
    std::map<VariableTableIndex, CVariable *>::const_iterator   Location;

    // Find the variable...
    Location = VariableTable_KeyByIndex.find(Index);

        // Not found...
        if(Location == VariableTable_KeyByIndex.end())
            throw std::string("internal fault, variable index not found");
          
    // Return the variable object...
    return *(Location)->second;
}

// Get size of variable using <identifier, scope> as key, or throw an error...
uint32 CParser::GetVariableSize(VariableName Name) throw(std::string const)
{
    // Find the variable...
    CVariable const &Variable = GetVariableByName(Name);
    
    // Return the size to caller...
    return Variable.unSize;
}

// Get the variable table...
std::map<Agni::CParser::VariableName, Agni::CParser::CVariable> const &
    CParser::GetVariableTable() const
{
    // Return reference...
    return VariableTable_KeyByName;
}

// Is this a function in the function table?
boolean CParser::IsFunctionInTable(FunctionName Name) const
{
    // Variables...
    std::map<FunctionName, CFunction>::const_iterator   Location;

    // Find the function...
    Location = FunctionTable_KeyByName.find(Name);

        // Not found...
        if(Location == FunctionTable_KeyByName.end())
            return false;

    // Found...
    return true;
}

// Is this a variable in the variable table at requested scope / global?
boolean CParser::IsVariableInTable(VariableName Name) const
{
    // Iterator...
    std::map<VariableName, CVariable>::const_iterator Location;

    // Try to find the variable first with requested scope...
    if(VariableTable_KeyByName.end() != 
        (Location = VariableTable_KeyByName.find(Name)))
        return true;

    // Not found, so try global scope then...
    else if(VariableTable_KeyByName.end() != 
        (Location = VariableTable_KeyByName.find(
            std::make_pair(Name.first, Global))))
        return true;

    // Not found in either local or global scope...
    else
        return false;
}

// Is operator an assignment?
boolean CParser::IsOperatorAssignment(CLexer::Operator const CurrentOperator) 
    const
{
    // Check operator...
    switch(CurrentOperator)
    {
        // Valid assignments...
        case CLexer::OPERATOR_ASSIGNMENT:
        case CLexer::OPERATOR_ASSIGNMENT_ADD:
        case CLexer::OPERATOR_ASSIGNMENT_SUBTRACT:
        case CLexer::OPERATOR_ASSIGNMENT_MULTIPLY:
        case CLexer::OPERATOR_ASSIGNMENT_DIVIDE:
        case CLexer::OPERATOR_ASSIGNMENT_MODULUS:
        case CLexer::OPERATOR_ASSIGNMENT_EXPONENT:
        case CLexer::OPERATOR_ASSIGNMENT_CONCATENATE:
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_AND:
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_OR:
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_XOR:
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_SHIFT_LEFT:
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_SHIFT_RIGHT:
            return true;

        // Not an assignment...
        default:
            return false;
    }
}

// Is operator logical?
boolean CParser::IsOperatorLogical(CLexer::Operator const CandidateOperator) 
    const
{
    // A relational operator...
    if(CandidateOperator >= CLexer::__OPERATOR_LOGICAL_FIRST__ && 
       CandidateOperator <= CLexer::__OPERATOR_LOGICAL_LAST__)
        return true;
    
    // Not a relational operator...
    else
        return false;
}


// Is the operator relational?
boolean CParser::IsOperatorRelational(CLexer::Operator const CandidateOperator)
    const
{
    // A relational operator...
    if(CandidateOperator >= CLexer::__OPERATOR_RELATIONAL_FIRST__ && 
       CandidateOperator <= CLexer::__OPERATOR_RELATIONAL_LAST__)
        return true;
    
    // Not a relational operator...
    else
        return false;
}

// Generate a complete i-code representation of the entire source code...
void CParser::Parse() throw(std::string const)
{
    // Reset the lexer...
    Lexer.Reset();
    
    // Clear the loop stack...
    while(!LoopStack.empty())
        LoopStack.pop();
    
    // Commence parsing in the global scope...
    CurrentScope = Global;
    
    // Try to parse...
    try
    {
        // Parse each statement in succession...
        for(;;)
        {
            // Parse the next statement...
            ParseStatement();
            
            // Tokenizer has bottomed out, we're done...
            if(Lexer.GetNextToken() == CLexer::TOKEN_END_OF_STREAM)
                break;
            
            // Not done yet, prepare for the next statement...
            else
                Lexer.Rewind();     
        }
    }
    
        // Failed to parse...
        catch(std::string const sReason)
        {
            // Prepend line number...
            std::string const sBetterReason = 
                std::string(":") + Lexer.GetCurrentHumanLineString() + 
                std::string(": error: ") + sReason;

            // Pass up the error chain...
            throw sBetterReason;
        }
}

// Parse an assignment...
void CParser::ParseAssignment() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex    = 0;
    CLexer::Operator        AssignmentOperator  = CLexer::___OPERATOR_INVALID___;
    bool                    bIsArray            = false;

    // Assignments only make sense within a function...
    if(CurrentScope == Global)
        throw std::string("assignments can only occur within a function");

    // Annotate the assembly listing with the original source line...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // Retrieve the variable / array...
    CVariable const &Variable = GetVariableByName(
        VariableName(Lexer.GetCurrentLexeme(), CurrentScope));

    // This is being used as an array...
    if(Lexer.GetLookAheadCharacter() == '[')
    {
        // The size cannot be of a single unit...
        if(Variable.unSize <= 1)
            throw std::string("invalid array size");

        // Ingest the opening brace...
        ReadToken(CLexer::TOKEN_DELIMITER_OPEN_BRACE);

        // There should be an expression before the next closing brace...
        if(Lexer.GetLookAheadCharacter() == ']')
            throw std::string("array index has bad expression");

        // Generate the array index expression logic...
        ParseExpression();
        
        // Ingest the closing brace...
        ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_BRACE);
        
        // Remember that this was an array for later...
        bIsArray = true;
    }
    
    // This is not being used as an array...
    else
    {
        // They've accidentally tried to assign to an array without an index...
        if(Variable.unSize != 1)
            throw std::string("you cannot assign to an array without an index"
                              " specified");
    }
    
    // An assignment did not follow...
    if(Lexer.GetNextToken() != CLexer::TOKEN_OPERATOR &&
       !IsOperatorAssignment(Lexer.GetCurrentOperator()))
        throw std::string("assignment must use one of the assignment operators");

    // Valid assignment operator, remember...
    else
        AssignmentOperator = Lexer.GetCurrentOperator();

    // Generate the logic for the expression's value...
    ParseExpression();
    
    // Ingest the end of the statement semicolon...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
    
    // The value gets popped off into first machine register...
    InstructionIndex = AddICodeInstruction(
        CurrentScope, INSTRUCTION_ICODE_POP);
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);

    // If assigning to an array, the top of the stack contains index...
    if(bIsArray)
    {
        // POP off into second machine register...
        InstructionIndex = AddICodeInstruction(
            CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(
            CurrentScope, InstructionIndex, REGISTER_ICODE_T1);
    }
    
    // Generate appropriate instruction for the type of assignment operator...
    switch(AssignmentOperator)
    {
        // Vanilla assignment =...
        case CLexer::OPERATOR_ASSIGNMENT:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_MOV); 
             break;
        
        // Additive assignment +=...
        case CLexer::OPERATOR_ASSIGNMENT_ADD:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_ADD); 
             break;
        
        // Subtraction assignment -=...
        case CLexer::OPERATOR_ASSIGNMENT_SUBTRACT:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_SUB); 
             break;
        
        // Multiplication assignment *=...
        case CLexer::OPERATOR_ASSIGNMENT_MULTIPLY:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_MUL);
             break;
        
        // Division assignment /=...
        case CLexer::OPERATOR_ASSIGNMENT_DIVIDE:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_DIV); 
             break;
        
        // Modulus assignment %=...
        case CLexer::OPERATOR_ASSIGNMENT_MODULUS:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_MOD);
             break;

        // Exponent assignment ^=...
        case CLexer::OPERATOR_ASSIGNMENT_EXPONENT:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_EXP);
             break;
        
        // Concatenation assignment...
        case CLexer::OPERATOR_ASSIGNMENT_CONCATENATE:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_CONCAT);
             break;
        
        // Bitwise and assignment...
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_AND:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_AND);
             break;
        
        // Bitwise or assignment |=...
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_OR:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_OR);
             break;
        
        // Bitwise xor assignment #=...
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_XOR:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_XOR);
             break;
        
        // Bitwise shift left assignment <<=...
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_SHIFT_LEFT:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_SHL);
             break;
        
        // Bitwise shift right assignment >>=...
        case CLexer::OPERATOR_ASSIGNMENT_BITWISE_SHIFT_RIGHT:
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_SHR);
             break;

        // Internal fault...
        default:
            throw std::string("internal fault while parsing expression");
    }
    
    // The first operand is the destination...
    
        // Since the assignment was to an array, destination must be treated as
        //  such by also encoding the index in...
        if(bIsArray)
            AddArrayIndexRegisterICodeOperand(
                CurrentScope, InstructionIndex, Variable.Index, 
                REGISTER_ICODE_T1);

        // Assignment was not to an array, add destination...
        else
            AddVariableICodeOperand(
                CurrentScope, InstructionIndex, Variable.Index);

    // Second operand is the source... (always Intel style syntax)
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
}

// Parse a break...
void CParser::ParseBreak() throw(std::string const)
{
    // You can only break when within a loop or switch...
    if(LoopStack.empty())
        throw std::string("break keyword can only be used within a loop or"
                          " switch");

    // Annotate the assembly listing with the original line...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // There must be a semicolon next...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
    
    // Prepare the jump target to the end of this loop...
    InstructionListIndex const LoopEndJumpTargetIndex = 
        LoopStack.top().EndTargetIndex;

    // JMP unconditionally to the recovered jump target...
    InstructionListIndex const InstructionIndex =
        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
    AddJumpTargetICodeOperand(
        CurrentScope, InstructionIndex, LoopEndJumpTargetIndex);
}

// Parse a code block...
void CParser::ParseCodeBlock() throw(std::string const)
{
    // Code blocks cannot exist in the global scope...
    if(CurrentScope == Global)
        throw std::string("global code blocks illegal");

    // Parse every statement within this block until we read its end...
    while(Lexer.GetLookAheadCharacter() != '}')
        ParseStatement();

    // Consume the closing curly brace at the end of the code block...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_CURLY_BRACE);
}

// Parse continue...
void CParser::ParseContinue() throw(std::string const)
{
    // You can only break when within a loop or switch...
    if(LoopStack.empty())
        throw std::string("continue keyword can only be used within a loop");

    // Annotate the assembly listing with the original line...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // There must be a semicolon next...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
    
    // Prepare the jump target to the start of this loop...
    InstructionListIndex const LoopStartJumpTargetIndex = 
        LoopStack.top().StartTargetIndex;

    // JMP unconditionally back to the beginning of the loop...
    InstructionListIndex const InstructionIndex =
        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
    AddJumpTargetICodeOperand(
        CurrentScope, InstructionIndex, LoopStartJumpTargetIndex);
}

// Parse an expression...
void CParser::ParseExpression() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex    = 0;
    CLexer::Operator        CurrentOperator;

    // Parse the subexpression...
    ParseSubExpression();
    
    // Now parse any remaining logical or relational operators...
    for(;;)
    {
        // There are no further logical or relational operators...
        if(Lexer.GetNextToken() != CLexer::TOKEN_OPERATOR ||
           (!IsOperatorRelational(Lexer.GetCurrentOperator()) && 
            !IsOperatorLogical(Lexer.GetCurrentOperator())))
        {
            // Back up then for the next appropriate kind of parsing...        
            Lexer.Rewind();
            break;
        }
        
        // Extract the current operator...
        CurrentOperator = Lexer.GetCurrentOperator();
        
        // Parse the second term...
        ParseSubExpression();

        // The first operand goes into the T1 register...
        
            // Add the POP instruction...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_POP);
            
            // Now add the T1 register operand...
            AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                    REGISTER_ICODE_T1);

        // The second operand goes into the T0 register...
        
            // Add the POP instruction...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_POP);
            
            // Now add the T0 register operand...
            AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                    REGISTER_ICODE_T0);

        // Evaluate relational operator...
        if(IsOperatorRelational(CurrentOperator))
        {
            // Prepare new jump target indices...
            InstructionListIndex TrueJumpTargetIndex = GetNextJumpTargetIndex();
            InstructionListIndex ExitJumpTargetIndex = GetNextJumpTargetIndex();
            
            // Decide what kind of relational operator...
            switch(CurrentOperator)
            {
                // Equality...
                case CLexer::OPERATOR_RELATIONAL_EQUAL:
                
                    // Generate JE instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JE);

                    // Done...
                    break;

                // Not equal...
                case CLexer::OPERATOR_RELATIONAL_NOT_EQUAL:
                
                    // Generate JNE instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JNE);

                    // Done...
                    break;

                // Less than...
                case CLexer::OPERATOR_RELATIONAL_LESS:
                
                    // Generate JL instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JL);

                    // Done...
                    break;

                
                // Greater than...
                case CLexer::OPERATOR_RELATIONAL_GREATER:
                
                    // Generate JG instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JG);

                    // Done...
                    break;

                // Less than or equal...
                case CLexer::OPERATOR_RELATIONAL_LESS_OR_EQUAL:
                
                    // Generate LE instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JLE);

                    // Done...
                    break;

                // Greater than or equal...
                case CLexer::OPERATOR_RELATIONAL_GREATER_OR_EQUAL:
                
                    // Generate JGE instruction...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JGE);

                    // Done...
                    break;

                // This should never happen, but also appeases the compiler...
                default:
                    throw std::string("internal fault, unknown relational"
                                      " operator");
            }
            
            // Jump operands are TO, T1, TrueTarget
            AddRegisterICodeOperand(CurrentScope, InstructionIndex,
                                    REGISTER_ICODE_T0);
            AddRegisterICodeOperand(CurrentScope, InstructionIndex,
                                    REGISTER_ICODE_T1);
            AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                      TrueJumpTargetIndex);

            // The outcome for false expressions is a zero on the stack...
            InstructionIndex = 
                AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
            
            // Generate a jump past the true condition...
            InstructionIndex = 
                AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
            AddJumpTargetICodeOperand(CurrentScope, InstructionIndex, 
                                      ExitJumpTargetIndex);
            
            // Create a target for truth to begin...
            AddICodeJumpTarget(CurrentScope, TrueJumpTargetIndex);
            
            // The outcome for true expressions is a one on the stack...
            InstructionIndex = 
                AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 1);
            
            // In order to seek past true code, we need the exit target...
            AddICodeJumpTarget(CurrentScope, ExitJumpTargetIndex);
        }
        
        // Evaluate logical operator...
        else
        {
            // Decide what kind of logical operator...
            switch(CurrentOperator)
            {
                // Binary and...
                case CLexer::OPERATOR_LOGICAL_AND:
                {
                    // Prepare new jump target indices...
                    InstructionListIndex FalseJumpTargetIndex = GetNextJumpTargetIndex();
                    InstructionListIndex ExitJumpTargetIndex = GetNextJumpTargetIndex();

                    // Second operand was false, so jump to false target...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JE);
                    AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                            REGISTER_ICODE_T0);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                              FalseJumpTargetIndex);
                                              
                    // First operand was false, so jump to false target...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JE);
                    AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                            REGISTER_ICODE_T1);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                              FalseJumpTargetIndex);
                                              
                    // Both operands were true, so expression resolves to one...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 1);
                    
                    // Jump to exit target...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex, 
                                              ExitJumpTargetIndex);

                    // Actual false jump target...
                    AddICodeJumpTarget(CurrentScope, FalseJumpTargetIndex);
                    
                    // False expressions resolve to zero...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
                    
                    // Actual exit label...
                    AddICodeJumpTarget(CurrentScope, ExitJumpTargetIndex);
                    
                    // Done...
                    break;
                }
                
                // Binary or...
                case CLexer::OPERATOR_LOGICAL_OR:
                {
                    // Prepare new jump target indices...
                    InstructionListIndex TrueJumpTargetIndex = GetNextJumpTargetIndex();
                    InstructionListIndex ExitJumpTargetIndex = GetNextJumpTargetIndex();
                    
                    // Second operand was true, so whole expression is true...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JNE);
                    AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                            REGISTER_ICODE_T0);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                              TrueJumpTargetIndex);
                                              
                    // First operand was true, so whole expression is true...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JNE);
                    AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                            REGISTER_ICODE_T1);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                              TrueJumpTargetIndex);
                    
                    // False expressions resolve to zero...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);                
                    
                    // Jump to exit target...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
                    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex, 
                                              ExitJumpTargetIndex);

                    // The actual true jump target...
                    AddICodeJumpTarget(CurrentScope, TrueJumpTargetIndex);
                    
                    // True expressions resolve to one...
                    InstructionIndex = 
                        AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 1);
                    
                    // The actual exit jump target...
                    AddICodeJumpTarget(CurrentScope, ExitJumpTargetIndex);
                    
                    // Done...
                    break;
                }
                
                // This should never happen, but also appeases the compiler...
                default:
                    throw std::string("internal fault, unknown logical"
                                      " operator");
            }
        }
    }
}

// Parse factor...
void CParser::ParseFactor() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex        = 0;
    CLexer::Operator        CurrentOperator;
    bool                    bUnaryOperatorPending   = false;
    
    // A unary operator was found...
    if(Lexer.GetNextToken() == CLexer::TOKEN_OPERATOR &&
       (Lexer.GetCurrentOperator() == CLexer::OPERATOR_ADD || 
        Lexer.GetCurrentOperator() == CLexer::OPERATOR_SUBTRACT ||
        Lexer.GetCurrentOperator() == CLexer::OPERATOR_BITWISE_NOT ||
        Lexer.GetCurrentOperator() == CLexer::OPERATOR_LOGICAL_NOT))
    {
        // Set the unary operator pending flag...
        bUnaryOperatorPending = true;
        
        // Store the current operator...
        CurrentOperator = Lexer.GetCurrentOperator();
    }
    
    // No unary operator was found...
    else
    {
        // Rewind token stream for next kind of parsing...
        Lexer.Rewind();
    }
    
    // Determine the kind of factor and parse it...
    switch(Lexer.GetNextToken())
    {
        // True / false constant should push 1 / 0 respectively onto stack...
        case CLexer::TOKEN_RESERVED_TRUE:
        case CLexer::TOKEN_RESERVED_FALSE:
        {
            // Push...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_PUSH);

            // Operand...
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 
                Lexer.GetCurrentToken() == CLexer::TOKEN_RESERVED_TRUE ? 1 : 0);
                
            // Done...
            break;
        }
   
        // Integer literals should be pushed onto stack...
        case CLexer::TOKEN_INTEGER:
        {        
            // Push...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_PUSH);
            
            // Operand...
            AddIntegerICodeOperand(
                CurrentScope, InstructionIndex, 
                atoi(Lexer.GetCurrentLexeme().c_str()));

            // Done...
            break;
        }

        // Float literals should be pushed onto stack...
        case CLexer::TOKEN_FLOAT:
        {
            // Push...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_PUSH);

            // Operand...
            AddFloatICodeOperand(CurrentScope, InstructionIndex, 
                (float) atof(Lexer.GetCurrentLexeme().c_str()));

            // Done...
            break;
        }

        // String literal should have its index added to table...
        case CLexer::TOKEN_STRING:
        {
            // Add to string table and get index...
            StringTableIndex const StringIndex = 
                AddString(Lexer.GetCurrentLexeme());

            // Add the string index into the i-code stream...
            InstructionIndex = 
                AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_PUSH);

            // Add the push instruction's operand, the string table index...
            AddStringICodeOperand(CurrentScope, InstructionIndex, StringIndex);
            
            // Done...
            break;
        }
        
        // Identifier found...
        case CLexer::TOKEN_IDENTIFIER:
        {
            // This is a variable / array...
            if(IsVariableInTable(
                VariableName(Lexer.GetCurrentLexeme(), CurrentScope)))
            {
                // Get the variable / array object...
                CVariable const &Variable = 
                GetVariableByName(
                    VariableName(Lexer.GetCurrentLexeme(), CurrentScope));
            
                // This is an array...
                if(Lexer.GetLookAheadCharacter() == '[')
                {
                    // Make sure this is an array, since arrays must be > 1 size...
                    if(Variable.unSize <= 1)
                        throw std::string("invalid array");

                    // Chomp the opening brace...
                    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_BRACE);
                    
                    // An expression of some kind must follow before parsing...
                    if(Lexer.GetLookAheadCharacter() == ']')
                        throw std::string("index required for array");

                    // Parse expression...
                    ParseExpression();
                    
                    // Chomp the closing brace...
                    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_BRACE);
                    
                    // Parsing expression will leave value in register T0, so we
                    //  want to PUSH it onto the stack...
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_POP);
                    AddRegisterICodeOperand(
                        CurrentScope, InstructionIndex, REGISTER_ICODE_T0);

                    // The original identifier should be PUSHed onto stack as
                    //  an array with its index in first machine register...
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddArrayIndexRegisterICodeOperand(
                        CurrentScope, InstructionIndex, Variable.Index,
                        REGISTER_ICODE_T0);
                }
                
                // This is a variable...
                else
                {
                    // Verify this is a variable...
                    if(Variable.unSize != 1)
                        throw std::string("expected an array index");
                    
                    // PUSH the variable onto the stack...
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddVariableICodeOperand(
                        CurrentScope, InstructionIndex, Variable.Index);
                }
            }

            // Identifier is not an array or variable so could be function...
            else
            {
                // Identified as a function...
                if(IsFunctionInTable(Lexer.GetCurrentLexeme()))
                {
                    // Parse the function call...
                    ParseFunctionCall();

                    // The return value should be PUSHed onto the stack...
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_PUSH);
                    AddRegisterICodeOperand(
                        CurrentScope, InstructionIndex, REGISTER_ICODE_RETURN);
                }
            }

            // Done...
            break;        
        }

        // Nested expression...
        case CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS:
        {
            // The expression should be parsed...
            ParseExpression();

            // The expression should be terminated by a closing parenthesis...
            ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);

            // Done...
            break;
        }

        // Any other token is considered invalid within a factor...
        default:
            throw std::string("junk input");
    }

    // A unary operator is pending...
    if(bUnaryOperatorPending)
    {
        // The result should be popped off stack into first machine register...
        InstructionIndex = AddICodeInstruction(
            CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(
            CurrentScope, InstructionIndex, REGISTER_ICODE_T0);

        // Write out instructions for logical not application...
        if(CurrentOperator == CLexer::OPERATOR_LOGICAL_NOT)
        {
            // Generate true and exit jump targets...
            InstructionListIndex const TrueJumpTargetIndex = 
                GetNextJumpTargetIndex();
            InstructionListIndex const ExitJumpTargetIndex =
                GetNextJumpTargetIndex();

            // Generate JE _RegisterT0, 0, TrueJumpTargetIndex
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_JE);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
            AddJumpTargetICodeOperand(
                CurrentScope, InstructionIndex, TrueJumpTargetIndex);

            // Generate logic for false condition...

                // Push false on stack... PUSH 0
                InstructionIndex = AddICodeInstruction(
                    CurrentScope, INSTRUCTION_ICODE_PUSH);
                AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);

                // Jump to exit...
                InstructionIndex = AddICodeInstruction(
                    CurrentScope, INSTRUCTION_ICODE_JMP);
                AddJumpTargetICodeOperand(
                    CurrentScope, InstructionIndex, ExitJumpTargetIndex);

            // Generate logic for true condition...

                // True jump target...
                AddICodeJumpTarget(CurrentScope, TrueJumpTargetIndex);

                // Expression resolved to true, so push it on to stack...
                InstructionIndex = AddICodeInstruction(
                    CurrentScope, INSTRUCTION_ICODE_PUSH);
                AddIntegerICodeOperand(CurrentScope, InstructionIndex, 1);

            // The exit jump target...
            AddICodeJumpTarget(CurrentScope, ExitJumpTargetIndex);
        }
        
        // Write out instructions for some other operator type...
        else
        {
            // Detect current operator...
            switch(CurrentOperator)
            {
                // Subtraction symbol becomes negation when applied unary...
                case CLexer::OPERATOR_SUBTRACT:
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_NEG);
                    break;
                
                // Bitwise not symbol becomes bitwise not instruction...
                case CLexer::OPERATOR_BITWISE_NOT:
                    InstructionIndex = AddICodeInstruction(
                        CurrentScope, INSTRUCTION_ICODE_NOT);
                    break;
                
                // Something is busted...
                default:
                    throw std::string("internal fault while parsing factor");
            }
            
            // The operand is the result in the first machine register...
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);

            // The result gets PUSHed onto the stack...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_PUSH);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
        }
    }
}

// Parse for loop...
void CParser::ParseFor() throw(std::string const)
{
    // We cannot have a for loop in the global scope...
    if(CurrentScope == Global)
        throw std::string("for loops are forbidden from the global scope");

    // Annotate the listing with the original line of code...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    /*
        TODO: Implement for loop parser...
    */
        throw std::string("for loop is not implemented");
}

// Parse a function definition...
void CParser::ParseFunction() throw(std::string const)
{
    // No nested functions permitted...
    if(CurrentScope != Global)
        throw std::string("nested functions are not permitted");

    // Read the function identifier...
    ReadToken(CLexer::TOKEN_IDENTIFIER);
    FunctionName NewFunction(Lexer.GetCurrentLexeme());

    // Add to the function table as a local function and switch parser scope...
    CurrentScope = AddFunction(NewFunction, false);

    // After the identifier comes the opening parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS);

    // The function takes parameters...
    if(Lexer.GetLookAheadCharacter() != ')')
    {
        // The function is the entry point, which cannot accept parameters...
        if(MainHeader.unMainIndex == CurrentScope)
            throw std::string("entry point cannot accept parameters (yet)");

        // Create a stack to store the parameters, reading them left to right...
        std::stack<std::string> ParametersLeftToRight;
        for(;;)
        {
            // Read a parameter...
            ReadToken(CLexer::TOKEN_IDENTIFIER);
            
            // Remember it...
            ParametersLeftToRight.push(Lexer.GetCurrentLexeme());
            
            // Is that it?
            if(Lexer.GetLookAheadCharacter() == ')')
                break;

            // More to come, so skip past the comma...
            ReadToken(CLexer::TOKEN_DELIMITER_COMMA);
        }
        
        // Remember the number of parameters the function takes...
        CFunction &NewFunction = GetFunctionByIndex(CurrentScope);
        NewFunction.Parameters = ParametersLeftToRight.size();
        
        // Add the parameters to the variable table, from right to left...
        while(!ParametersLeftToRight.empty())
        {
            // Create variable name from identifier and scope...
            VariableName    NewParameter(ParametersLeftToRight.top(), 
                                         CurrentScope);

            // Add to variable table... (can't pass arrays yet)
            AddVariable(NewParameter, 1, CVariable::Parameter);
            
            // Done with it...
            ParametersLeftToRight.pop();
        }
    }

    // Consume the closing parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
    
    // Consume the opening curly brace to the function's code block...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_CURLY_BRACE);
    
    // Parse the code block...
    ParseCodeBlock();
    
    // Back in the global scope now...
    CurrentScope = Global;
}

// Parse a function invokation...
void CParser::ParseFunctionCall() throw(std::string const)
{
    // Variables...
    unsigned int            unParametersFound   = 0;
    InstructionListIndex    InstructionIndex    = 0;

    // Retrieve the function descriptor object...
    CFunction const &Function = GetFunctionByName(Lexer.GetCurrentLexeme());
    
    // Eat the opening parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS);
    
    // Keep parsing parameters, while there are some...
    while(Lexer.GetLookAheadCharacter() != ')')
    {
        // Generate the parameter expression logic...
        ParseExpression();
        
        // Remember the number of parameters we've found...
      ++unParametersFound;
        
        // Ensure local functions accept correct parameter count...
        if(!Function.bIsHostFunction && unParametersFound > Function.Parameters)
            throw std::string("too many parameters for local function");

        // A comma follows each parameter, if not the last...
        if(Lexer.GetLookAheadCharacter() != ')')
            ReadToken(CLexer::TOKEN_DELIMITER_COMMA);
    }
    
    // Function call terminates with closing brace...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
    
    // Parameters expected and parameters provided must match for local calls...
    if(!Function.bIsHostFunction && unParametersFound != Function.Parameters)
        throw std::string("too few parameters for local function");

    // Generate call instruction for host or local call...
    InstructionIndex = AddICodeInstruction(
        CurrentScope, Function.bIsHostFunction ? INSTRUCTION_ICODE_CALLHOST
                                               : INSTRUCTION_ICODE_CALL);

    // Function index is first and only operand to CALL(HOST) instruction...
    AddFunctionICodeOperand(CurrentScope, InstructionIndex, Function.Index);
}

// Parse a host function import...
void CParser::ParseHost() throw(std::string const)
{
    // The next token after "host" should be the function identifier...
    ReadToken(CLexer::TOKEN_IDENTIFIER);
    
    // Create a function name...
    FunctionName    Name(Lexer.GetCurrentLexeme());
    
    // Add the function as a host function...
    AddFunction(Name, true);
    
    // There must be an open and close parentheses following the identifier...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS);
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
    
    // The import ends with a semicolon, like any statement...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
}

// Parse an if block...
void CParser::ParseIf() throw(std::string const)
{
    // Variables...
    InstructionListIndex InstructionIndex   = 0;
    
    // Conditional logic can only occur within a function...
    if(CurrentScope == Global)
        throw std::string("conditional logic illegal in global scope");

    // Add annotation of the original line of source code...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // Generate jump target to start of false code block...
    InstructionListIndex const FalseJumpTargetIndex = GetNextJumpTargetIndex();
    
    // Chomp the opening parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS);
    
    // Generate the assembly logic to evaluate the conditional expression...
    ParseExpression();
    
    // Chomp the closing parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
    
    // POP the result of the expression into the first machine register...
    InstructionIndex = AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
    
    // Generate logic to jump if false to false code block start...
    InstructionIndex = AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JE);
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
    AddJumpTargetICodeOperand(CurrentScope, InstructionIndex, 
                              FalseJumpTargetIndex);

    // Generate the true block assembly logic...
    ParseStatement();
    
    // An if block can be optionally followed by an else clause...
    if(Lexer.GetNextToken() == CLexer::TOKEN_RESERVED_ELSE)
    {
        // True block should skip past the false block... JMP SkipToEndOfFalse
        InstructionListIndex const SkipFalseJumpTargetIndex = 
            GetNextJumpTargetIndex();
        InstructionIndex = AddICodeInstruction(
            CurrentScope, INSTRUCTION_ICODE_JMP);
        AddJumpTargetICodeOperand(
            CurrentScope, InstructionIndex, SkipFalseJumpTargetIndex);

        // The false jump target precedes the false code block...
        AddICodeJumpTarget(CurrentScope, FalseJumpTargetIndex);

        // Generate assembly logic for the false code block...
        ParseStatement();
        
        // Generate the actual skip false code block jump target at the end...
        AddICodeJumpTarget(CurrentScope, SkipFalseJumpTargetIndex);
    }
    
    // No else block followed...
    else
    {
        // Push the token stream back...
        Lexer.Rewind();
        
        // Without an else block, the false target should follow the true...
        AddICodeJumpTarget(CurrentScope, FalseJumpTargetIndex);
    }
}

// Parse a function return...
void CParser::ParseReturn() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex;
    
    // You can't return within the global scope...
    if(CurrentScope == Global)
        throw std::string("you cannot return from within the global scope");

    // Annotate assembly listing with the original line...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // Programmer wanted to return a value...
    if(Lexer.GetLookAheadCharacter() != ';')
    {
        // Generate assembly listing for the return expression...
        ParseExpression();
        
        // A main function is present and that is what we are returning from...
        if(MainHeader.unMainIndex != (unsigned) -1 &&
           MainHeader.unMainIndex == CurrentScope)
        {
            // Result should be popped off into first machine register...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_POP);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
        }
        
        // We are not returning from the main function...
        else
        {
            // Result should be popped off into return value machine register...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_POP);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_RETURN);
        }
    }
    
    // No value specified to return...
    else
    {
        // A main function is present and that is what we are returning from...
        if(MainHeader.unMainIndex != (unsigned) -1 &&
           MainHeader.unMainIndex == CurrentScope)    
        {
            // Clear the first machine register...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_MOV);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
        }
    }
    
    // Time for the RET / EXIT instruction...
    
        // This is the main function exiting...
        if(MainHeader.unMainIndex != (unsigned) -1 &&
           MainHeader.unMainIndex == CurrentScope)
        {
            // EXIT _RegisterT0...
            InstructionIndex = AddICodeInstruction(
                CurrentScope, INSTRUCTION_ICODE_EXIT);
            AddRegisterICodeOperand(
                CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
        }
        
        // This is a function other than main exiting...
        else
            AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_RET);
}

// Parse a statement...
void CParser::ParseStatement() throw(std::string const)
{
    // Recursion base case: Are we done with the statement?
    if(Lexer.GetLookAheadCharacter() == ';')
    {
        // Eat the semicolon up...
        ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
        
        // Unwind...
        return;
    }
    
    // Get the first token of the statement...
    CLexer::Token InitialToken = Lexer.GetNextToken();
    
    // Now handle the statement based on the kind it is...
    switch(InitialToken)
    {
        // Oops...
        case CLexer::TOKEN_END_OF_STREAM:
            
            // Kill the parser...
            throw std::string("unexpected end of file");

        // New code block has just begun, a '{' signals this...
        case CLexer::TOKEN_DELIMITER_OPEN_CURLY_BRACE:
            
            // Parse the code block...
            ParseCodeBlock();
            break;

        // Variable / array declaration...
        case CLexer::TOKEN_RESERVED_VAR:
        
            // Parse...
            ParseVariable();
            break;

        // Host function imported...
        case CLexer::TOKEN_RESERVED_HOST:
        
            // Parse...
            ParseHost();
            break;

        // Function definition...
        case CLexer::TOKEN_RESERVED_FUNC:
        
            // Parse...
            ParseFunction();
            break;

        // If block...
        case CLexer::TOKEN_RESERVED_IF:
            
            // Parse...
            ParseIf();
            break;

        // While block...
        case CLexer::TOKEN_RESERVED_WHILE:
            
            // Parse...
            ParseWhile();
            break;

        // For block...
        case CLexer::TOKEN_RESERVED_FOR:
            
            // Parse...
            ParseFor();
            break;
        
        // Break...
        case CLexer::TOKEN_RESERVED_BREAK:
            
            // Parse...
            ParseBreak();
            break;
        
        // Continue...
        case CLexer::TOKEN_RESERVED_CONTINUE:
        
            // Parse...
            ParseContinue();
            break;
        
        // Return...
        case CLexer::TOKEN_RESERVED_RETURN:
            
            // Parse...
            ParseReturn();
            break;

        // Either an assignment is being made or a function is being called...
        case CLexer::TOKEN_IDENTIFIER:
        {
            // Is this a variable?
            if(IsVariableInTable(VariableName(Lexer.GetCurrentLexeme(), 
                                              CurrentScope)))
                ParseAssignment();

            // Is this a function?
            else if(IsFunctionInTable(Lexer.GetCurrentLexeme()))
            {
                // Annotate the line...
                AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());

                // Parse the function call...
                ParseFunctionCall();
                
                // Verify function call ended with a semicolon...
                ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
            }
            
            // Invalid...
            else
                throw std::string("invalid identifier");

            // Done...
            break;
        }
        
        // Nothing else is considered a valid statement...
        default:
            throw std::string("unexpected input");
    }
}

// Parse a sub expression...
void CParser::ParseSubExpression() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex    = 0;
    CLexer::Operator        CurrentOperator;
    
    // Parse the first term...
    ParseTerm();
    
    // Terms are flanked by +, -, and $ operators, so process next terms...
    for(;;)
    {
        // Get next token and ensure suitable for this kind of parsing...
        if(Lexer.GetNextToken() != CLexer::TOKEN_OPERATOR ||
           (Lexer.GetCurrentOperator() != CLexer::OPERATOR_ADD &&
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_SUBTRACT &&
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_CONCATENATE))
        {
            // Back up then for the next appropriate kind of parsing...        
            Lexer.Rewind();
            break;
        }

        // Remember the current operator...
        CurrentOperator = Lexer.GetCurrentOperator();
        
        // Now parse the second term...
        ParseTerm();
        
        // First operand popped off into register T1...
        InstructionIndex = 
            AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T1);

        // Second operand popped off into register T0...
        InstructionIndex = 
            AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);
        
        // Write appropriate instruction out for binary operator...
        switch(CurrentOperator)
        {
            // Binary addition...
            case CLexer::OPERATOR_ADD:
                
                // Add instruction...
                InstructionIndex = 
                    AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_ADD);
                break;
            
            // Binary subtraction...
            case CLexer::OPERATOR_SUBTRACT:
            
                // Sub instruction...
                InstructionIndex = 
                    AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_SUB);
                break;
            
            // Binary concatenation...
            case CLexer::OPERATOR_CONCATENATE:
            
                // Concat instruction...
                InstructionIndex = 
                    AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_CONCAT);
                break;

            // This should never happen...
            default:
                throw std::string("internal fault, unknown binary operator in"
                                  " subexpression");
        }

        // Binary operator instruction also needs its two operands...
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T1);

        // Result in register T0 should be pushed onto the stack now...
        InstructionIndex = AddICodeInstruction(CurrentScope, 
                                               INSTRUCTION_ICODE_PUSH);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);
    }
}

// Parse a term within an expression...
void CParser::ParseTerm() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex    = 0;
    CLexer::Operator        CurrentOperator;
    
    // Parse the first factor...
    ParseFactor();
    
    // Keep parsing any more *, /, %, ^, &, |, #, <<, and >>, operators...
    for(;;)
    {
        // Get the next token and make sure we can process it here...
        if(Lexer.GetNextToken() != CLexer::TOKEN_OPERATOR ||
           (Lexer.GetCurrentOperator() != CLexer::OPERATOR_MULTIPLY && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_DIVIDE && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_MODULUS && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_EXPONENT && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_BITWISE_AND && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_BITWISE_OR && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_BITWISE_XOR && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_BITWISE_SHIFT_LEFT && 
            Lexer.GetCurrentOperator() != CLexer::OPERATOR_BITWISE_SHIFT_RIGHT))
        {
            // Back up then for the next appropriate kind of parsing...        
            Lexer.Rewind();
            break;
        }

        // Retrieve the current operator...
        CurrentOperator = Lexer.GetCurrentOperator();
        
        // Parse adjacent factor...
        ParseFactor();
        
        // First operand popped off into register T1...
        InstructionIndex = 
            AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T1);

        // Second operand popped off into register T0...
        InstructionIndex = 
            AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);
        
        // Write appropriate instruction out for binary operator...
        switch(CurrentOperator)
        {
            // Binary multiply...
            case CLexer::OPERATOR_MULTIPLY:
            
                // Mul instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_MUL);
                break;
            
            // Binary division...
            case CLexer::OPERATOR_DIVIDE:
            
                // Div instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_DIV);
                break;
            
            // Binary modulus...
            case CLexer::OPERATOR_MODULUS:
            
                // Mod instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_MOD);
                break;
            
            // Binary exponentiation...
            case CLexer::OPERATOR_EXPONENT:
            
                // Exp instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_EXP);
                break;

            // Binary bitwise AND...
            case CLexer::OPERATOR_BITWISE_AND:
            
                // And instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_AND);
                break;

            // Binary bitwise OR...
            case CLexer::OPERATOR_BITWISE_OR:

                // Or instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_OR);
                break;

            // Binary bitwise XOR...
            case CLexer::OPERATOR_BITWISE_XOR:

                // XOr instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_XOR);
                break;

            // Binary bitwise shift left...
            case CLexer::OPERATOR_BITWISE_SHIFT_LEFT:

                // Shl instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_SHL);
                break;

            // Binary bitwise shift right...
            case CLexer::OPERATOR_BITWISE_SHIFT_RIGHT:

                // Shr instruction...
                InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                       INSTRUCTION_ICODE_SHR);
                break;

            // This should never happen...
            default:
                throw std::string("internal fault, unknown binary operator in"
                                  " term");
        }

        // Binary operator instruction also needs its two operands...
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T1);

        // Result in register T0 should be pushed onto the stack now...
        InstructionIndex = AddICodeInstruction(CurrentScope, 
                                               INSTRUCTION_ICODE_PUSH);
        AddRegisterICodeOperand(CurrentScope, InstructionIndex, 
                                REGISTER_ICODE_T0);      
    }
}

// Parse a variable / array declaration...
void CParser::ParseVariable() throw(std::string const)
{
    // Make sure we are hovering over an identifier now...
    ReadToken(CLexer::TOKEN_IDENTIFIER);
    
    // Remember the identifier...
    std::string sIdentifier = Lexer.GetCurrentLexeme();
    
    // For now, assume the variable size to be one...
    uint32 unSize = 1;
    
    // This variable is actually an array...
    if(Lexer.GetLookAheadCharacter() == '[')
    {
        // Ok, so consume the opening brace...
        ReadToken(CLexer::TOKEN_DELIMITER_OPEN_BRACE);
        
        // Make sure the array index is an integral value...
        ReadToken(CLexer::TOKEN_INTEGER);
        
        // Make sure the value is >= 1
        if(::atoi(Lexer.GetCurrentLexeme().c_str()) <= 1)
            throw std::string("invalid array size");

        // Store the size...
        unSize = ::atoi(Lexer.GetCurrentLexeme().c_str());
        
        // Consume the closing brace...
        ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_BRACE);
    }
    
    // Now create a variable name for the table...
    VariableName Name(sIdentifier, CurrentScope);
    
    // Add it to the table...
    AddVariable(Name, unSize, CVariable::Variable);
    
    // Eat the terminating semicolon...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
}

// Parse a while loop...
void CParser::ParseWhile() throw(std::string const)
{
    // Variables...
    InstructionListIndex    InstructionIndex    = 0;
    Loop                    CurrentLoop         = {0, 0};
    
    // Iterative logic only available outside of global scope...
    if(CurrentScope == Global)
        throw std::string("iterative logic unavailable in global scope");

    // Add our original line of source as an annotation in the listing...
    AddICodeAnnotation(CurrentScope, Lexer.GetCurrentSourceLine());
    
    // Generate beginning and end loop jump targets...
    InstructionListIndex const StartJumpTargetIndex = GetNextJumpTargetIndex();
    InstructionListIndex const EndJumpTargetIndex = GetNextJumpTargetIndex();
    
    // Initiate the loop with the start jump target preceding its code block...
    AddICodeJumpTarget(CurrentScope, StartJumpTargetIndex);
    
    // Chomp the opening parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_OPEN_PARENTHESIS);
    
    // Generate assembly logic for the conditional expression...
    ParseStatement();
    
    // Chomp the closing parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
    
    // POP result into first machine register...
    InstructionIndex = AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_POP);
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
    
    // JE _RegisterT0, 0 EndJumpTargetIndex...
    InstructionIndex = AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JE);
    AddRegisterICodeOperand(CurrentScope, InstructionIndex, REGISTER_ICODE_T0);
    AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
    AddJumpTargetICodeOperand(
        CurrentScope, InstructionIndex, EndJumpTargetIndex);

    // Remember new loop instance beginning and end for parser to recover from
    //  its body...
    CurrentLoop.StartTargetIndex    = StartJumpTargetIndex;
    CurrentLoop.EndTargetIndex      = EndJumpTargetIndex;
    LoopStack.push(CurrentLoop);

    // Generate assembly listing for the actual loop body...
    ParseStatement();

    // Done with the loop book keeping information for this level of loop...
    LoopStack.pop();

    // JMP StartJumpTargetIndex... (to re-evaluate conditional expression)
    InstructionIndex = AddICodeInstruction(CurrentScope, INSTRUCTION_ICODE_JMP);
    AddJumpTargetICodeOperand(
        CurrentScope, InstructionIndex, StartJumpTargetIndex);

    // The end jump target should get placed here now...
    AddICodeJumpTarget(CurrentScope, EndJumpTargetIndex);
}

// Read a token and verify it is what was expected...
void CParser::ReadToken(const CLexer::Token Expected) throw(std::string const)
{
    // Get the next token...
    CLexer::Token Received = Lexer.GetNextToken();

    // This is was not what was expected...
    if(Expected != Received)
        throw Lexer.TokenToString(Expected) + " expected, received " + 
              Lexer.TokenToString(Received);
}

