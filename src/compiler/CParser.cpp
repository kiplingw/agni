/*
  Name:         CParser.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to take a source code vector and translate it into
                intermediate code. Provides the compiler front end...
*/

// Includes...
#include "CParser.h"

// Initialize static data...

    // Function indicies begin at one, since zero denotes global scope...
    CParser::IdentifierScope 
        CParser::CFunction::NextAvailableFunctionIndex = 1;

    // Variable indicies begin at one, since zero denotes global scope...
    CParser::VariableTableIndex 
        CParser::CVariable::NextAvailableVariableIndex = 1;

// Default constructor...
CParser::CParser(vector<string> &InputSourceCode)
    : SourceCode(InputSourceCode),
      Lexer(InputSourceCode),
      CurrentJumpTargetIndex(0)
{    
    // Clear out script header...
    memset(&MainHeader, 0, sizeof(Agni_MainHeader));
}

// Add float operand to i-code instruction...
void CParser::AddFloatICodeOperand(IdentifierScope FunctionIndex,
                                   InstructionListIndex InstructionIndex,
                                   float32 fFloatOperand) throw(const string)
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
    throw(const string)
{
    // Check to see if function is already in function table...
    if(FunctionTable_KeyByName.find(Name) != FunctionTable_KeyByName.end())
        throw "function " + Name + " redeclared";

    // Add new function to table...

        // Create a new function and assume no parameters for now...
        CFunction NewFunction(0, bIsHostFunction);

        // Now use the name and parameter count pair as hash key...
        FunctionTable_KeyByName.insert(make_pair(Name, NewFunction));

        // Also add this to our other function table using the index as key...
        FunctionTable_KeyByIndex.insert(make_pair(NewFunction.GetIndex(), 
                                                  &NewFunction));

    // Return the new function's index...
    return NewFunction.GetIndex();
}

// Add a line of source code or something else into the i-code for human...
void CParser::AddICodeAnnotation(IdentifierScope FunctionIndex, 
                                 const string sAnnotation)
    throw(const string)
{
    // Locate the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);
    
    // Create the appropriate i-node for the annotation...
    ICodeNode AnnotationNode;
    AnnotationNode.Type         = ICodeNode::ANNOTATION;
    AnnotationNode.sAnnotation  = sAnnotation;
    
    // Now add node to the requested function's i-code sequence...
    Function.ICodeList.push_back(AnnotationNode);
}

// Add i-code instruction to end of function, return index, or throw error...
CParser::InstructionListIndex CParser::AddICodeInstruction(
    IdentifierScope FunctionIndex, ICodeOperationCode OperationCode)
    throw(const string)
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
    throw(const string)
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
                              ICodeOperand Operand) throw(const string)
{
    // Find the node...
    ICodeNode &Node = GetICodeNodeByImplicitIndex(FunctionIndex,
                                                  InstructionIndex);

    // Verify that this is an instruction...
    if(Node.Type != ICodeNode::INSTRUCTION)
        throw "internal fault, cannot add operand to non-instructional node";

    // Now set the operand...
    Node.Instruction.OperandList.push_back(Operand);
}

// Add integer operand to i-code instruction...
void CParser::AddIntegerICodeOperand(IdentifierScope FunctionIndex,
                                     InstructionListIndex InstructionIndex,
                                     int32 nIntegerOperand) throw(const string)
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
    throw(const string)
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
    throw(const string)
{
    // Variables...
    ICodeOperand    Operand;
    
    // Initialize operand to register index...
    Operand.Type            = OT_ICODE_REGISTER;
    Operand.Register        = Register;
    
    // Add the operand to the instruction...
    AddICodeOperand(FunctionIndex, InstructionIndex, Operand);
}

// Add string operand to i-code instruction...
void CParser::AddStringICodeOperand(IdentifierScope FunctionIndex,
                                    InstructionListIndex InstructionIndex,
                                    StringTableIndex StringIndex) 
    throw(const string)
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
    throw(const string)
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
    throw(const string)
{
    // Check if the <identifier, scope> already exist in table...
    if(VariableTable_KeyByName.find(Name) != VariableTable_KeyByName.end())
        throw "variable '" + Name.first + "' already declared";

    // Add new variable to table...

        // Create new variable and set its attributes...
        CVariable NewVariable(unSize, Type);
    
        // Add it to the variable table using name as hash key...
        VariableTable_KeyByName.insert(make_pair(Name, NewVariable));
        
        // Also add this to our other variable table using index as key...
        VariableTable_KeyByIndex.insert(make_pair(NewVariable.GetIndex(),
                                                  &NewVariable));
    
    // Return the variable index to caller...
    return NewVariable.GetIndex();
}

// Get function via index, or throw an error...
CParser::CFunction &CParser::GetFunctionByIndex(IdentifierScope Index) 
    const throw(const string)
{
    // Variables...
    map<IdentifierScope, CFunction *>::const_iterator   Location;

    // Find the function...
    Location = FunctionTable_KeyByIndex.find(Index);

        // Not found...
        if(Location == FunctionTable_KeyByIndex.end())
            throw "internal fault, function index not found";
          
    // Return the function object...
    return *(Location)->second;
}

// Get function via name, or throw an error...
CParser::CFunction &CParser::GetFunctionByName(FunctionName Name) 
    throw(const string)
{
    // Variables...
    map<FunctionName, CFunction>::iterator    Location;

    // Find the function...
    Location = FunctionTable_KeyByName.find(Name);

        // Not found...
        if(Location == FunctionTable_KeyByName.end())
            throw "internal fault, function name not found";

    // Return the function object...
    return Location->second;
}

// Get an i-code node from within a function at the specified instruction...
CParser::ICodeNode &CParser::GetICodeNodeByImplicitIndex(
    IdentifierScope FunctionIndex, InstructionListIndex InstructionIndex) 
    const throw(const string)
{
    // Variables...
    InstructionListIndex    Index    = 0;

    // First find the function...
    CFunction &Function = GetFunctionByIndex(FunctionIndex);
    
    // Seek to index...
    list<ICodeNode>::iterator Location = Function.ICodeList.begin();
    while(Index != InstructionIndex)
    {
        // Out of bounds...
        if(Location == Function.ICodeList.end())
            throw "internal fault, instruction index out of bounds";

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

// Get variable via name, or throw an error...
CParser::CVariable &CParser::GetVariableByName(VariableName Name) 
    throw(const string)
{
    // Variables...
    map<VariableName, CVariable>::iterator  Location;

    // Find the variable...
    Location = VariableTable_KeyByName.find(Name);

        // Not found...
        if(Location == VariableTable_KeyByName.end())
            throw Name.first + "was not declared";

    // Return the variable object...
    return Location->second;
}
            
// Get variable by index, or throw an error...
CParser::CVariable &CParser::GetVariableByIndex(VariableTableIndex Index)
    const throw(const string)
{
    // Variables...
    map<VariableTableIndex, CVariable *>::const_iterator    Location;

    // Find the variable...
    Location = VariableTable_KeyByIndex.find(Index);

        // Not found...
        if(Location == VariableTable_KeyByIndex.end())
            throw "internal fault, variable index not found";
          
    // Return the variable object...
    return *(Location)->second;
}

// Get size of variable using <identifier, scope> as key, or throw an error...
uint32 CParser::GetVariableSize(VariableName Name) throw(const string)
{
    // Find the variable...
    CVariable &Variable = GetVariableByName(Name);
    
    // Return the size to caller...
    return Variable.unSize;
}

// Is this a function in the function table?
boolean CParser::IsFunctionInTable(FunctionName Name) const
{
    // Variables...
    map<FunctionName, CFunction>::const_iterator    Location;

    // Find the function...
    Location = FunctionTable_KeyByName.find(Name);

        // Not found...
        if(Location == FunctionTable_KeyByName.end())
            return false;

    // Found...
    return true;
}

// Is this a variable in the variable table?
boolean CParser::IsVariableInTable(VariableName Name) const
{
    // Variables...
    map<VariableName, CVariable>::const_iterator    Location;

    // Find the variable...
    Location = VariableTable_KeyByName.find(Name);

        // Not found...
        if(Location == VariableTable_KeyByName.end())
            return false;

    // Found...
    return true;
}

// Is operator logical?
boolean CParser::IsOperatorLogical(const CLexer::Operator CandidateOperator) 
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
boolean CParser::IsOperatorRelational(const CLexer::Operator CandidateOperator) 
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
void CParser::Parse() throw(const string)
{
    // Reset the lexer...
    Lexer.Reset();
    
    // Clear the loop stack...
    while(!LoopStack.empty())
        LoopStack.pop();
    
    // Commence parsing in the global scope...
    CurrentScope = Global;
    
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

// Parse an assignment...
void CParser::ParseAssignment() throw(const string)
{

}

// Parse a break...
void CParser::ParseBreak() throw(const string)
{

}

// Parse a code block...
void CParser::ParseCodeBlock() throw(const string)
{
    // Code blocks cannot exist in the global scope...
    if(CurrentScope == Global)
        throw "global code locks illegal";

    // Parse every statement within this block until we read its end...
    while(Lexer.GetLookAheadCharacter() != '}')
        ParseStatement();

    // Consume the closing curly brace at the end of the code block...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_CURLY_BRACE);
}

// Parse continue...
void CParser::ParseContinue() throw(const string)
{

}

// Parse an expression...
void CParser::ParseExpression() throw(const string)
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

        // The first operand goes into the T0 register...
        
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

                // This should never happen...
                default:
                    throw "internal fault, unknown relational operator";
            }
            
            // Jump operands are TO, T1, TrueTarget
            AddRegisterICodeOperand(CurrentScope, InstructionIndex,
                                    REGISTER_ICODE_T0);
            AddRegisterICodeOperand(CurrentScope, InstructionIndex,
                                    REGISTER_ICODE_T1);
            AddJumpTargetICodeOperand(CurrentScope, InstructionIndex,
                                      TrueJumpTargetIndex);

            // The outcome for false expressions is a zero on the stack...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_PUSH);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 0);
            
            // Generate a jump past the true condition...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_JMP);
            AddJumpTargetICodeOperand(CurrentScope, InstructionIndex, 
                                      ExitJumpTargetIndex);
            
            // Create a target for truth to begin...
            AddICodeJumpTarget(CurrentScope, TrueJumpTargetIndex);
            
            // The outcome for true expressions is a one on the stack...
            InstructionIndex = AddICodeInstruction(CurrentScope, 
                                                   INSTRUCTION_ICODE_PUSH);
            AddIntegerICodeOperand(CurrentScope, InstructionIndex, 1);
            
            // In order to seek past true code, we need the exit target...
            AddICodeJumpTarget(CurrentScope, ExitJumpTargetIndex);
        }
        
        // Evaluate logical operator...
        else
        {
            /* 
                todo: finish this method 
            */
        }
    }
}

// Parse for loop...
void CParser::ParseFor() throw(const string)
{

}

// Parse a function definition...
void CParser::ParseFunction() throw(const string)
{
    // No nested functions permitted...
    if(CurrentScope != Global)
        throw "nested functions are not permitted";

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
            throw "entry point cannot accept parameters (yet)";

        // Create a stack to store the parameters, reading them left to right...
        stack<string>   ParametersLeftToRight;
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
            AddVariable(NewParameter, 1, CVariable::PARAMETER);
            
            // Done with it...
            ParametersLeftToRight.pop();
        }
    }

    // Consume the closing parenthesis...
    ReadToken(CLexer::TOKEN_DELIMITER_CLOSE_PARENTHESIS);
}

// Parse a function invokation...
void CParser::ParseFunctionCall() throw(const string)
{

}

// Parse a host function import...
void CParser::ParseHost() throw(const string)
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
void CParser::ParseIf() throw(const string)
{

}

// Parse a function return...
void CParser::ParseReturn() throw(const string)
{

}

// Parse a statement...
void CParser::ParseStatement() throw(const string)
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
            throw "unexpected end of file";

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
                throw "invalid identifier";

            // Done...
            break;
        }
        
        // Nothing else is considered a valid statement...
        default:
            throw "unexpected input";
    }
}

// Parse a sub expression...
void CParser::ParseSubExpression() throw(const string)
{

}

// Parse a variable / array declaration...
void CParser::ParseVariable() throw(const string)
{
    // Make sure we are hovering over an identifier now...
    ReadToken(CLexer::TOKEN_IDENTIFIER);
    
    // Remember the identifier...
    string sIdentifier = Lexer.GetCurrentLexeme();
    
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
        if(::atoi(Lexer.GetCurrentLexeme().c_str()) >= 1)
            throw "invalid array size";

        // Store the size...
        unSize = ::atoi(Lexer.GetCurrentLexeme().c_str());
    }
    
    // Now create a variable name for the table...
    VariableName Name(sIdentifier, CurrentScope);
    
    // Add it to the table...
    AddVariable(Name, unSize, CVariable::VARIABLE);
    
    // Eat the terminating semicolon...
    ReadToken(CLexer::TOKEN_DELIMITER_SEMICOLON);
}

void CParser::ParseWhile() throw(const string)
{

}

// Read a token and verify it is what was expected...
void CParser::ReadToken(const CLexer::Token Expected) throw(const string)
{
    // Get the next token...
    CLexer::Token Received = Lexer.GetNextToken();

    // This is was not what was expected...
    if(Expected != Received)
        throw Lexer.TokenToString(Expected) + " expected, received " + 
              Lexer.TokenToString(Received);
}
