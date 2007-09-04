/*
  Name:         CLexer.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to breakup input stream into lexemes. I've combined the
                lexer and tokenizer together...
*/

// Includes...
#include "CLexer.h"

// Using the Agni namespace...
using namespace Agni;

// Initialize static arrays...

    /*  Notes: The second column is the offset into the succeeding state table
               that the operator's state might transition into. The third 
               column is the number of valid operator states in the succeeding 
               state table. If the second and third column members are both 
               zero, that entry represents a terminal state eg. +=, -=, et 
               cetera. DO NOT CHANGE THE ORDER OF EACH MEMBER IN EACH STATE 
               TABLE, as this compromises the state transition since they are 
               dependent on the substate ordinals... */

    // First operator state transition table...
    const CLexer::OperatorState CLexer::OperatorStates_0[MAXIMUM_OPERATOR_STATES]
        = {{'+', 0, 2,  OPERATOR_ADD},                              // +
            {'-', 2, 2,  OPERATOR_SUBTRACT},                         // -
            {'*', 4, 1,  OPERATOR_MULTIPLY},                         // *
            {'/', 5, 1,  OPERATOR_DIVIDE},                           // /
            {'%', 6, 1,  OPERATOR_MODULUS},                          // %
            {'^', 7, 1,  OPERATOR_EXPONENT},                         // ^
            {'&', 8, 2,  OPERATOR_BITWISE_AND},                      // &
            {'|', 10, 2, OPERATOR_BITWISE_OR},                       // |
            {'#', 12, 1, OPERATOR_BITWISE_XOR},                      // #
            {'~', 0, 0,  OPERATOR_BITWISE_NOT},                      // ~
            {'!', 13, 1, OPERATOR_LOGICAL_NOT},                      // !
            {'=', 14, 1, OPERATOR_ASSIGNMENT},                       // =
            {'<', 15, 2, OPERATOR_RELATIONAL_LESS},                  // <
            {'>', 17, 2, OPERATOR_RELATIONAL_GREATER},               // >
            {'$', 19, 1, OPERATOR_CONCATENATE}};                     // $

    // Second operator state transition table...
    const CLexer::OperatorState CLexer::OperatorStates_1[MAXIMUM_OPERATOR_STATES]
        = {{'=', 0, 0, OPERATOR_ASSIGNMENT_ADD},                    // +=
            {'+', 0, 0, OPERATOR_INCREMENT},                         // ++
            {'=', 0, 0, OPERATOR_ASSIGNMENT_SUBTRACT},               // -=
            {'-', 0, 0, OPERATOR_DECREMENT},                         // --
            {'=', 0, 0, OPERATOR_ASSIGNMENT_MULTIPLY},               // *=
            {'=', 0, 0, OPERATOR_ASSIGNMENT_DIVIDE},                 // /=
            {'=', 0, 0, OPERATOR_ASSIGNMENT_MODULUS},                // %=
            {'=', 0, 0, OPERATOR_ASSIGNMENT_EXPONENT},               // ^=
            {'=', 0, 0, OPERATOR_ASSIGNMENT_BITWISE_AND},            // &=
            {'&', 0, 0, OPERATOR_LOGICAL_AND},                       // &&
            {'=', 0, 0, OPERATOR_ASSIGNMENT_BITWISE_OR},             // |=
            {'|', 0, 0, OPERATOR_LOGICAL_OR},                        // ||
            {'=', 0, 0, OPERATOR_ASSIGNMENT_BITWISE_XOR},            // #=
            {'=', 0, 0, OPERATOR_RELATIONAL_NOT_EQUAL},              // !=
            {'=', 0, 0, OPERATOR_RELATIONAL_EQUAL},                  // ==
            {'=', 0, 0, OPERATOR_RELATIONAL_LESS_OR_EQUAL},          // <=
            {'<', 0, 1, OPERATOR_BITWISE_SHIFT_LEFT},                // <<
            {'=', 0, 0, OPERATOR_RELATIONAL_GREATER_OR_EQUAL},       // >=
            {'>', 1, 1, OPERATOR_BITWISE_SHIFT_RIGHT},               // >>
            {'=', 0, 0, OPERATOR_ASSIGNMENT_CONCATENATE}};           // $=

    // Third operator state transition table...
    const CLexer::OperatorState CLexer::OperatorStates_2[MAXIMUM_OPERATOR_STATES]
        = {{'=', 0, 0, OPERATOR_ASSIGNMENT_BITWISE_SHIFT_LEFT},     // <<=
            {'=', 0, 0, OPERATOR_ASSIGNMENT_BITWISE_SHIFT_RIGHT}};   // >>=

// Create a lexer from a source code vector...
CLexer::CLexer(const vector<string> &UserSourceCode)
    : unCurrentLine(0), 
      SourceCode(UserSourceCode),
      CurrentToken(TOKEN_INVALID),
      unCurrentLexemeStart(0),
      unCurrentLexemeEnd(0),
      CurrentOperator(___OPERATOR_INVALID___)
{
    // Initialize delimeters table for O(1) lookups...
    memset(Delimiters, '\x0', sizeof(Delimiters));
    Delimiters[(unsigned int) ','] =    ',';
    Delimiters[(unsigned int) '('] =    '(';
    Delimiters[(unsigned int) ')'] =    ')';
    Delimiters[(unsigned int) '['] =    '[';
    Delimiters[(unsigned int) ']'] =    ']';
    Delimiters[(unsigned int) '{'] =    '{';
    Delimiters[(unsigned int) '}'] =    '}';
    Delimiters[(unsigned int) ';'] =    ';';
}

// Backup lexer state...
void CLexer::Backup()
{
    // Backup state...
    Backup_unCurrentLine        = unCurrentLine;
    Backup_CurrentToken         = CurrentToken;
    Backup_sCurrentLexeme       = sCurrentLexeme;
    Backup_unCurrentLexemeStart = unCurrentLexemeStart;
    Backup_unCurrentLexemeEnd   = unCurrentLexemeEnd;
    Backup_CurrentOperator      = CurrentOperator;
}

// Get the current line starting from one...
uint32 CLexer::GetCurrentHumanLineIndex() const
{
    // Humans count line numbers starting from one...
    return GetCurrentLineIndex() + 1;
}

// Get the current lexeme...
const string &CLexer::GetCurrentLexeme()
{
    // Return it...
    return sCurrentLexeme;
}

// Copy the current lexeme...
void CLexer::GetCurrentLexeme(string &sLexeme) const
{
    // Store lexeme string for caller...
    sLexeme = sCurrentLexeme;
}

// Get the current token...
CLexer::Token CLexer::GetCurrentToken() const
{
    // Return it...
    return CurrentToken;
}

// Get the current token as a string...
const string CLexer::GetCurrentTokenAsString() const
{
    // Return it as a string...
    return TokenToString(CurrentToken);
}

// Get the current line starting from zero...
uint32 CLexer::GetCurrentLineIndex() const
{
    // Return position...
    return unCurrentLine;
}

// Get the current operator, if applicable...
CLexer::Operator CLexer::GetCurrentOperator() const
{
    // Return it...
    return CurrentOperator;
}

// Get the current source line...
const string &CLexer::GetCurrentSourceLine() const
{
    // Return it...
    return SourceCode.at(unCurrentLine);
}

// Get the next character...
char CLexer::GetNextCharacter()
{
    // Variables...
    string  sCurrentLine;

    // Have we reached the end of the source code? Alert lexer...
    if(unCurrentLine == SourceCode.size())
        return '\x0';

    // Fetch the current line...
    sCurrentLine = SourceCode[unCurrentLine];

    // We're at the end of this line...
    if(unCurrentLexemeEnd >= sCurrentLine.length())
    {
        // Go to the next line...
      ++unCurrentLine;
        
        // But this is now the end of the source code, alert lexer...
        if(unCurrentLine == SourceCode.size())
            return '\x0';

        // We are at a new line...
        else
        {
            // Grab it...
            sCurrentLine = SourceCode[unCurrentLine];
            
            // Prepare for a new lexeme...
            unCurrentLexemeStart    = 0;
            unCurrentLexemeEnd      = 0;
        }
    }
    
    // Return character and seek to the next one...
    return sCurrentLine.at(unCurrentLexemeEnd++);
}

// Get the next token...
CLexer::Token CLexer::GetNextToken()
{
    // Variables...
    uint32          CurrentLexerState               = STATE_START;
    uint8           CurrentOperatorCharacterIndex   = 0;
    uint8           CurrentOperatorStateIndex       = 0;
    OperatorState   CurrentOperatorState            = {0, 0, 0, 
                                                       ___OPERATOR_INVALID___};
    boolean         bLexemeDone                     = false;
    char            cCurrentCharacter               = '\x0';
    boolean         bAddCurrentCharacter            = false;
    Token           TempToken                       = TOKEN_INVALID;
    
    // Store our state so we can restore later...
    Backup();
    
    // Clear current lexeme out...
    sCurrentLexeme.clear();

    // The new lexeme begins at the end of the last one...
    unCurrentLexemeStart = unCurrentLexemeEnd;
    
    // Begin lexical analysis...
    while(true)
    {
        // Extract the next character in the source code...
        cCurrentCharacter = GetNextCharacter();
        
            // We've reached the end of the source, nothing more to do...
            if(cCurrentCharacter == '\x0')
                break;
        
        // Assume, for now, that the character will be added to the lexeme...
        bAddCurrentCharacter = true;
        
        // Depending on which state we are in, handle the new character...
        switch(CurrentLexerState)
        {
            // Token is invalid...
            case STATE_UNKNOWN:
            
                // Done...
                bLexemeDone = true;
                break;
            
            // State machine in initial state...
            case STATE_START:
            
                // White space...
                if(IsCharacterWhiteSpace(cCurrentCharacter))
                {
                    // Ignore it...
                  ++unCurrentLexemeStart;                
                    bAddCurrentCharacter = false;
                }
                
                // Integer starting...
                else if(IsCharacterNumeric(cCurrentCharacter))
                    CurrentLexerState = STATE_INTEGER;
                
                // Float starting...
                else if(cCurrentCharacter == '.')
                    CurrentLexerState = STATE_FLOAT;
                
                // Identifier starting...
                else if(IsCharacterIdentifier(cCurrentCharacter))
                    CurrentLexerState = STATE_IDENTIFIER;
                
                // Delimeter starting...
                else if(IsCharacterDelimiter(cCurrentCharacter))
                    CurrentLexerState = STATE_DELIMITER;
                
                // Operator starting...
                else if(IsCharacterOperator(cCurrentCharacter, 0))
                {
                    // Get the initial operator state index...
                    CurrentOperatorStateIndex = 
                            GetOperatorStateIndex(cCurrentCharacter, 0, 0, 0);

                        // Bad character...
                        if(CurrentOperatorStateIndex == (uint8) -1)
                            return TOKEN_INVALID;

                    // Now get the state...
                    CurrentOperatorState = 
                                GetOperatorState(0, CurrentOperatorStateIndex);

                    // Remember that first operator character has been read...
                    CurrentOperatorCharacterIndex = 1;
                    
                    // Assume, for now, that this the entire operator...
                    CurrentOperator = CurrentOperatorState.Index;
                    
                    // We are now lexing an operator...
                    CurrentLexerState = STATE_OPERATOR;
                }
                
                // String starting...
                else if(cCurrentCharacter == '\"')
                {
                    // We don't care about the quote itself...
                    bAddCurrentCharacter = false;
                    
                    // Jump to next state...
                    CurrentLexerState = STATE_STRING;
                }
                
                // Invalid character...
                else
                    CurrentLexerState = STATE_UNKNOWN;

                // Ready to jump to next state...
                break;

            // Lexing an integer...
            case STATE_INTEGER:
                
                // Integer, so assume lexeme is still an integer...
                if(IsCharacterNumeric(cCurrentCharacter))
                    CurrentLexerState = STATE_INTEGER;
                
                // Turns out this integer is actually a float...
                else if(cCurrentCharacter == '.')
                    CurrentLexerState = STATE_FLOAT;

                // If white space or delimeter, then integer is done...
                else if(IsCharacterWhiteSpace(cCurrentCharacter) ||
                        IsCharacterDelimiter(cCurrentCharacter))
                {
                    // Integer lexeme complete...
                    bAddCurrentCharacter    = false;
                    bLexemeDone             = true;                 
                }
                
                // Anything else is not a valid float or integer...
                else
                    CurrentLexerState = STATE_UNKNOWN;
                
                // Carry on eating the next character...
                break;

            // Lexing a float...
            case STATE_FLOAT:

                // Integer, so assume lexeme is still a float...
                if(IsCharacterNumeric(cCurrentCharacter))
                    CurrentLexerState = STATE_FLOAT;

                // Hit white space or delimeter...
                else if(IsCharacterWhiteSpace(cCurrentCharacter) ||
                   IsCharacterDelimiter(cCurrentCharacter))
                {
                    // Float lexeme complete...
                    bLexemeDone             = true;
                    bAddCurrentCharacter    = false;
                }
                
                // Anything else is invalid...
                else
                    CurrentLexerState = STATE_UNKNOWN;
                
                // Carry on with next character...
                break;

            // Lexing an identifier...
            case STATE_IDENTIFIER:
            
                // Character, so assume lexeme is still an identifier...
                if(IsCharacterIdentifier(cCurrentCharacter))
                    CurrentLexerState = STATE_IDENTIFIER;

                // Hit white space or delimeter...
                else if(IsCharacterWhiteSpace(cCurrentCharacter) ||
                   IsCharacterDelimiter(cCurrentCharacter))
                {
                    // Identifier lexeme complete...
                    bLexemeDone             = true;
                    bAddCurrentCharacter    = false;
                }
                
                // Anything else is invalid...
                else
                    CurrentLexerState = STATE_UNKNOWN;
            
                // Carry on with next character...
                break;

            // Lexing an operator...
            case STATE_OPERATOR:
                
                // Based on the character and its position, if it has no
                //  substates, then we know it is complete...
                if(CurrentOperatorState.SubStateCount == 0)
                {
                    // Operator lexeme complete...
                    bLexemeDone             = true;
                    bAddCurrentCharacter    = false;
                }
                
                // Is the new character part of the operator?
                if(IsCharacterOperator(cCurrentCharacter, 
                                       CurrentOperatorCharacterIndex))
                {
                    // Get the operator state index of the next substate...
                    CurrentOperatorStateIndex = 
                        GetOperatorStateIndex(cCurrentCharacter, 
                                              CurrentOperatorCharacterIndex,
                                              CurrentOperatorState.SubStateIndex,
                                              CurrentOperatorState.SubStateCount);

                    // Who knows...
                    if(CurrentOperatorStateIndex == (uint8) -1)
                        CurrentLexerState = STATE_UNKNOWN;

                    // Fetch the next operator structure...
                    else
                    {
                        // Extract...
                        CurrentOperatorState = 
                            GetOperatorState(CurrentOperatorCharacterIndex,
                                             CurrentOperatorStateIndex);

                        // We are ready for the next operator character...
                      ++CurrentOperatorCharacterIndex;
                        
                        // We are more certain of an operator now...
                        CurrentOperator = CurrentOperatorState.Index;
                    }
                }
                
                // New character is not part of this operator...
                else
                {
                    // All done then...
                    bLexemeDone             = true;
                    bAddCurrentCharacter    = false;                
                }
                
                // Carry on with next character...
                break;

            // Lexing a delimiter...
            case STATE_DELIMITER:
            
                // We are starting on something new now...
                bAddCurrentCharacter    = false;
                bLexemeDone             = true;
                
                // Carry on with next character...
                break;

            // Lexing a string...
            case STATE_STRING:
            
                // End of string reached...
                if(cCurrentCharacter == '\"')
                {
                    // We don't care about the quotation mark...
                    bAddCurrentCharacter    = false;
                    
                    // Remember that we are done with the string...
                    CurrentLexerState       = STATE_STRING_CLOSE_QUOTE;
                }

                // We can't start a new line without closing present one...
                else if(cCurrentCharacter == '\n')
                {
                    // Prepare to abort...
                    bAddCurrentCharacter    = false;
                    CurrentLexerState       = STATE_UNKNOWN;
                }
                
                // String escape...
                else if(cCurrentCharacter == '\\')
                {
                    // Skip escape character and remember we are escaping...
                    bAddCurrentCharacter    = false;
                    CurrentLexerState       = STATE_STRING_ESCAPE;
                }

                // Carry on with next character...
                break;

            // Lexing string escape sequence...
            case STATE_STRING_ESCAPE:

                // Revert back to string state...
                CurrentLexerState = STATE_STRING;

                // Carry on with next character...
                break;

            // String has just ended...
            case STATE_STRING_CLOSE_QUOTE:
            
                // Ignore the quotation character...
                bAddCurrentCharacter    = false;
                bLexemeDone             = true;

                // Carry on with next character...
                break;
        }
        
        // Finally add the character to the back of the lexeme...
        if(bAddCurrentCharacter)
            sCurrentLexeme += cCurrentCharacter;

        // Lexeme all done, stop grabbing characters...
        if(bLexemeDone)
            break;
    }

    // Seek the end of the lexeme back to prepare for next one...
  --unCurrentLexemeEnd;

    // Now that we've terminated the lexer state machine and have a complete
    //  lexeme, use lexer state to determine the token...
    switch(CurrentLexerState)
    {
        // Unknown token...
        case STATE_UNKNOWN:

            // Remember token...
            TempToken = TOKEN_INVALID;
            break;

        // Integer token...
        case STATE_INTEGER:
        
            // Remember token...
            TempToken = TOKEN_INTEGER;
            break;

        // Float token...
        case STATE_FLOAT:

            // Remember token...
            TempToken = TOKEN_FLOAT;
            break;

        // Identifier or a reserved word token...
        case STATE_IDENTIFIER:
        
            // Assume it is an identifier until proven that this is a reserved 
            //  word, if that is the case...
            TempToken = TOKEN_IDENTIFIER;
            
            // var or var []...
            if(sCurrentLexeme == "var")
                TempToken = TOKEN_RESERVED_VAR;

            // true...
            if(sCurrentLexeme == "true") 
                TempToken = TOKEN_RESERVED_TRUE;

            // false...
            if(sCurrentLexeme == "false")
                TempToken = TOKEN_RESERVED_FALSE;

            // if...
            if(sCurrentLexeme == "if")
                TempToken = TOKEN_RESERVED_IF;

            // else...
            if(sCurrentLexeme == "else")
                TempToken = TOKEN_RESERVED_ELSE;

            // break...
            if(sCurrentLexeme == "break")
                TempToken = TOKEN_RESERVED_BREAK;
                
            // continue...
            if(sCurrentLexeme == "continue")
                TempToken = TOKEN_RESERVED_CONTINUE;
            
            // for...
            if(sCurrentLexeme == "for")
                TempToken = TOKEN_RESERVED_FOR;
            
            // while...
            if(sCurrentLexeme == "while")
                TempToken = TOKEN_RESERVED_WHILE;
            
            // func...
            if(sCurrentLexeme == "func")
                TempToken = TOKEN_RESERVED_FUNC;
            
            // return...
            if(sCurrentLexeme == "return")
                TempToken = TOKEN_RESERVED_RETURN;
            
            // host...
            if(sCurrentLexeme == "host")
                TempToken = TOKEN_RESERVED_HOST;

            // Done...
            break;

        // Delimiter...
        case STATE_DELIMITER:
        
            // Which delimeter was found?
            switch(sCurrentLexeme.at(0))
            {
                // Comma...
                case ',': TempToken = TOKEN_DELIMITER_COMMA; break;            
                
                // Open parenthesis...
                case '(': TempToken = TOKEN_DELIMITER_OPEN_PARENTHESIS; break;
                
                // Close parenthesis...
                case ')': TempToken = TOKEN_DELIMITER_CLOSE_PARENTHESIS; break;
                
                // Open bracket...
                case '[': TempToken = TOKEN_DELIMITER_OPEN_BRACE; break;
                
                // Close bracket...
                case ']': TempToken = TOKEN_DELIMITER_CLOSE_BRACE; break;

                // Open curly brace...
                case '{': TempToken = TOKEN_DELIMITER_OPEN_CURLY_BRACE; break;

                // Close curly brace...
                case '}': TempToken = TOKEN_DELIMITER_CLOSE_CURLY_BRACE; break;

                // Semi colon...
                case ';': TempToken = TOKEN_DELIMITER_SEMICOLON; break;
            }

            // Done...
            break;

        // Operator...
        case STATE_OPERATOR:

            // Assume for now that it is an operator...
            TempToken = TOKEN_OPERATOR;

            // Since # doubles as beginning of a directive, check to make sure
            //  if it is being used as a pre-processor directive here...
            if(sCurrentLexeme == "#")
            {
                // #include detected...
                if(SourceCode[unCurrentLine].compare(unCurrentLexemeEnd, 
                   strlen("include"), "include") == 0)
                {
                    // Remember token and finish building lexeme...
                    TempToken = TOKEN_PREPROCESSOR_INCLUDE;
                    sCurrentLexeme += "include";

                    // The new lexeme begins at the end of the last one...
                    unCurrentLexemeEnd += strlen("include");
                    unCurrentLexemeStart = unCurrentLexemeEnd;
                }
            }

            // Done...
            break;

        // String...
        case STATE_STRING_CLOSE_QUOTE:
        
            // Remember token...
            TempToken = TOKEN_STRING;
            break;

        // All that remains is white space...
        default:
            TempToken = TOKEN_END_OF_STREAM;
    }

    // Remember the current token now...
    CurrentToken = TempToken;
    
    // Return the token finally to the caller...
    return CurrentToken;
}

// Get the next token as a string...
const string CLexer::GetNextTokenAsString()
{
    // Get the next token...
    return TokenToString(GetNextToken());
    

}

// Get the offset of the beginning of the current lexeme...
uint32 CLexer::GetLexemeStartIndex() const
{
    // Return it...
    return unCurrentLexemeStart;
}

// Get the first character of the next token...
char CLexer::GetLookAheadCharacter()
{
    // Variables...
    char    cCurrentCharacter   = '\x0';

    // Backup the lexer state before we go peeking around...
    Backup();
    
    // Skip over white space...
    do
    {
        // Grab a character...
        cCurrentCharacter = GetNextCharacter();    
    }
    while(IsCharacterWhiteSpace(cCurrentCharacter));
    
    // Restore the lexer back to how it was before...
    Rewind();
    
    // Return the next character to the caller...
    return cCurrentCharacter;
}

// Get an operator state...
CLexer::OperatorState CLexer::GetOperatorState(uint8 CharacterIndex, 
                                               uint8 StateIndex) const
{
    // Return operator state depending on the character and state index...
    switch(CharacterIndex)
    {
        // First character uses the first operator state...
        case 0:
            return OperatorStates_0[StateIndex];
        
        // Second character uses the second operator state...
        case 1:
            return OperatorStates_1[StateIndex];
            
        // Third character uses the third operator state...
        case 2:
        default:
            return OperatorStates_2[StateIndex];
    }
}

// Get an operator state index...
uint8 CLexer::GetOperatorStateIndex(char cCharacter, 
                                    uint8 CharacterIndex,
                                    uint8 SubStateIndex, 
                                    uint8 SubStateCount) const
{
    // Variables...
    uint8   StartStateIndex     = 0;
    uint8   EndStateIndex       = 0;
    uint8   CurrentStateIndex   = 0;
    char    cTempCharacter      = '\x0';

    /* Check to make sure character index is ok...
    assert(CharacterIndex <= 2);*/

    // This is the first character in an operator (sequence?)
    if(CharacterIndex == 0)
    {
        // Just about everything in the first state table can transition...
        StartStateIndex = 0;
        EndStateIndex   = MAXIMUM_OPERATOR_STATES;
    }
    
    // Character is passed the first state table...
    else
    {
        // So use the caller's knowledge of substate beginning and length...
        StartStateIndex = SubStateIndex;
        EndStateIndex   = SubStateIndex + SubStateCount;
    }
    
    // Start at the first valid substate until we find a match...
    for(CurrentStateIndex = StartStateIndex; CurrentStateIndex < EndStateIndex;
        CurrentStateIndex++)
    {
        // Find the current state at the specified index...
        switch(CharacterIndex)
        {
            // First character in sequence...
            case 0:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_0[CurrentStateIndex].cCharacter;
                break;
            
            // Second character in sequence...
            case 1:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_1[CurrentStateIndex].cCharacter;
                break;
           
           // Third character in sequence...
            case 2:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_2[CurrentStateIndex].cCharacter;
                break;
        }
        
        // Found index into state table, return to caller...
        if(cTempCharacter == cCharacter)
            return CurrentStateIndex;
    }
    
    // No match found... (this shouldn't ever happen)
    return (uint8) -1;
}

// Is character a delimiter?
boolean CLexer::IsCharacterDelimiter(char cCharacter) const
{
    // Check in constant time...
    return (Delimiters[(unsigned int) cCharacter] == cCharacter);
    //return Delimiters.is_in(cCharacter);
}

// Is character an identifier?
boolean CLexer::IsCharacterIdentifier(char cCharacter) const
{
    // Check...
    return ((cCharacter >= '0' && cCharacter <= '9') ||
            (cCharacter >= 'a' && cCharacter <= 'z') ||
            (cCharacter >= 'A' && cCharacter <= 'Z'));
}

// Is character numeric?
boolean CLexer::IsCharacterNumeric(char cCharacter) const
{
    // Check...
    return (cCharacter >= '0' && cCharacter <= '9');
}

// Is character an operator or at least a part of one?
boolean CLexer::IsCharacterOperator(char cCharacter, uint8 CharacterIndex) const
{
    // Variables...
    uint8   CurrentStateIndex   = 0;
    char    cTempCharacter      = '\x0';

    // Find a match in the operator state, based on its CharacterIndex offset...
    for(CurrentStateIndex = 0; CurrentStateIndex < MAXIMUM_OPERATOR_STATES;
        CurrentStateIndex++)
    {
        // Find the current state at the specified index...
        switch(CharacterIndex)
        {
            // First character in sequence...
            case 0:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_0[CurrentStateIndex].cCharacter;
                break;
            
            // Second character in sequence...
            case 1:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_1[CurrentStateIndex].cCharacter;
                break;
                
            // Third character in sequence...
            case 2:
                
                // Extract character to check for match after...
                cTempCharacter = OperatorStates_2[CurrentStateIndex].cCharacter;
                break;
        }
        
        // Found it, so the character is or is part of a valid operator...
        if(cCharacter == cTempCharacter)
            return true;
    }

    // No match found...
    return false;
}

// Is character a piece of white space?
boolean CLexer::IsCharacterWhiteSpace(char cCharacter) const
{
    // Check...
    return (cCharacter == ' ' || cCharacter == '\t' || cCharacter == '\n');
}

// Initialize lexer state from another's...
CLexer &CLexer::operator=(const CLexer &SourceLexer)
{
    // Copy lexer state from source into us...
    unCurrentLine           = SourceLexer.unCurrentLine;
    /* SourceCode initialized in constructor */
    CurrentToken            = SourceLexer.CurrentToken;
    sCurrentLexeme          = SourceLexer.sCurrentLexeme;
    unCurrentLexemeStart    = SourceLexer.unCurrentLexemeStart;
    unCurrentLexemeEnd      = SourceLexer.unCurrentLexemeEnd;
    CurrentOperator         = SourceLexer.CurrentOperator;
    
    // Expression evaluates to true...
    return *this;
}

// Reset the lexer...
void CLexer::Reset()
{
    // Reset back to the start of the script...
    unCurrentLine           = 0;
    
    // Reset the start and end of the current lexeme...
    unCurrentLexemeStart    = 0;
    unCurrentLexemeEnd      = 0;
    
    // Reset the current operator...
    CurrentOperator         = ___OPERATOR_INVALID___;
}

// Shift back lexer one token...
void CLexer::Rewind()
{
    // Restore previous state...
    unCurrentLine           = Backup_unCurrentLine;
    CurrentToken            = Backup_CurrentToken;
    sCurrentLexeme          = Backup_sCurrentLexeme;
    unCurrentLexemeStart    = Backup_unCurrentLexemeStart;
    unCurrentLexemeEnd      = Backup_unCurrentLexemeEnd;
    CurrentOperator         = Backup_CurrentOperator;
}

// Converts a token into a human readable string...
const string CLexer::TokenToString(const Token token) const
{
    // Convert to string...
    switch(token)
    {
        // Basic tokens...
        case TOKEN_END_OF_STREAM: return string("___end_of_stream___");
        case TOKEN_INVALID: return string("___invalid___");
        case TOKEN_INTEGER: return string("integer");
        case TOKEN_FLOAT: return string("float");
        case TOKEN_IDENTIFIER: return string("identifier");
        
        // Reserved keywords...
        case TOKEN_RESERVED_VAR: return string("var");
        case TOKEN_RESERVED_TRUE: return string("true");
        case TOKEN_RESERVED_FALSE: return string("false");
        case TOKEN_RESERVED_IF: return string("if");
        case TOKEN_RESERVED_ELSE: return string("else");
        case TOKEN_RESERVED_BREAK: return string("break");
        case TOKEN_RESERVED_CONTINUE: return string("continue");
        case TOKEN_RESERVED_FOR: return string("for");
        case TOKEN_RESERVED_WHILE: return string("while");
        case TOKEN_RESERVED_FUNC: return string("func");
        case TOKEN_RESERVED_RETURN: return string("return");
        case TOKEN_RESERVED_HOST: return string("host");
        
        // An operator...
        case TOKEN_OPERATOR: return string("operator");
        
        // Delimiters...
        case TOKEN_DELIMITER_COMMA: return string(",");
        case TOKEN_DELIMITER_OPEN_PARENTHESIS: return string("(");
        case TOKEN_DELIMITER_CLOSE_PARENTHESIS: return string(")");

        case TOKEN_DELIMITER_OPEN_BRACE: return string("[");
        case TOKEN_DELIMITER_CLOSE_BRACE: return string("]");

        case TOKEN_DELIMITER_OPEN_CURLY_BRACE: return string("{");
        case TOKEN_DELIMITER_CLOSE_CURLY_BRACE: return string("}");
        case TOKEN_DELIMITER_SEMICOLON: return string(";");
        
        // String...
        case TOKEN_STRING: return string("string");

        // Macro definitions...
        case TOKEN_PREPROCESSOR_DEFINE: return string("#define");
        case TOKEN_PREPROCESSOR_UNDEF: return string("#undef");
        
        // Conditional inclusion...
        case TOKEN_PREPROCESSOR_IFDEF: return string("#ifdef");
        case TOKEN_PREPROCESSOR_IFNDEF: return string("#ifndef");
        case TOKEN_PREPROCESSOR_IF: return string("#if");
        case TOKEN_PREPROCESSOR_ENDIF: return string("#endif");
        case TOKEN_PREPROCESSOR_ELSE: return string("#else");
        case TOKEN_PREPROCESSOR_ELIF: return string("#elif");
        
        // Error directive...
        case TOKEN_PREPROCESSOR_ERROR: return string("#error");
        
        // Source file inclusion...
        case TOKEN_PREPROCESSOR_INCLUDE: return string("#include");

        // Unhandled token...
        default: return string("___unknown_token___");
    }
}

// Deconstructor...
CLexer::~CLexer()
{

}
