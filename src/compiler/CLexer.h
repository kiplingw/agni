/*
  Name:         CLexer.h (definition)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to breakup input stream into lexemes. I've combined the
                lexer and tokenizer together...
*/

// Multiple include protection...
#ifndef _CLEXER_H_
#define _CLEXER_H_

// Pre-processor directives...

    // Includes...

        // Data types...
        #include "../include/AgniPlatformSpecific.h"
        
        // String class...
        #include <string>
        
        // Vector...
        #include <vector>

// Within the Agni namespace...
namespace Agni
{
    // CLexer class definition...
    class CLexer
    {
        // Public stuff...
        public:

            // Data types, enums, and other constants...

                // Tokenizer's types...
                typedef enum
                {
                    // Error related tokens...
                    TOKEN_END_OF_STREAM = 0,
                    TOKEN_INVALID,
                    
                    // Basic tokens...
                    TOKEN_INTEGER,
                    TOKEN_FLOAT,
                    TOKEN_IDENTIFIER,
                    
                    // Reserved keywords...
                    TOKEN_RESERVED_VAR,
                    TOKEN_RESERVED_TRUE,
                    TOKEN_RESERVED_FALSE,
                    TOKEN_RESERVED_IF,
                    TOKEN_RESERVED_ELSE,
                    TOKEN_RESERVED_BREAK,
                    TOKEN_RESERVED_CONTINUE,
                    TOKEN_RESERVED_FOR,
                    TOKEN_RESERVED_WHILE,
                    TOKEN_RESERVED_FUNC,
                    TOKEN_RESERVED_RETURN,
                    TOKEN_RESERVED_HOST,
                    
                    // An operator...
                    TOKEN_OPERATOR,
                    
                    // Delimiters...
                    TOKEN_DELIMITER_COMMA,
                    TOKEN_DELIMITER_OPEN_PARENTHESIS,       /* ( */
                    TOKEN_DELIMITER_CLOSE_PARENTHESIS,      /* ) */

                    TOKEN_DELIMITER_OPEN_BRACE,             /* [ */
                    TOKEN_DELIMITER_CLOSE_BRACE,            /* ] */

                    TOKEN_DELIMITER_OPEN_CURLY_BRACE,       /* { */
                    TOKEN_DELIMITER_CLOSE_CURLY_BRACE,      /* } */
                    TOKEN_DELIMITER_SEMICOLON,
                    
                    // String...
                    TOKEN_STRING,
                    
                    // Pre-processor...
                        
                        // Macro definitions...
                        TOKEN_PREPROCESSOR_DEFINE,
                        TOKEN_PREPROCESSOR_UNDEF,
                        
                        // Conditional inclusion...
                        TOKEN_PREPROCESSOR_IFDEF,
                        TOKEN_PREPROCESSOR_IFNDEF,
                        TOKEN_PREPROCESSOR_IF,
                        TOKEN_PREPROCESSOR_ENDIF,
                        TOKEN_PREPROCESSOR_ELSE,
                        TOKEN_PREPROCESSOR_ELIF,
                        
                        // Error directive...
                        TOKEN_PREPROCESSOR_ERROR,
                        
                        // Source file inclusion...
                        TOKEN_PREPROCESSOR_INCLUDE
                }Token;

                // Operators...
                typedef enum
                {
                    // Invalid...
                 ___OPERATOR_INVALID___ = 0,

                    // Arithmetic...
                    OPERATOR_ADD,
                    OPERATOR_SUBTRACT,
                    OPERATOR_MULTIPLY,
                    OPERATOR_DIVIDE,
                    OPERATOR_MODULUS,       /* %  */
                    OPERATOR_EXPONENT,      /* ^  */
                    OPERATOR_CONCATENATE,   /* $  */
                    OPERATOR_INCREMENT,     /* ++ */
                    OPERATOR_DECREMENT,     /* -- */
                    
                    // Arithmetic combined with an assignment...
                    OPERATOR_ASSIGNMENT_ADD,
                    OPERATOR_ASSIGNMENT_SUBTRACT,
                    OPERATOR_ASSIGNMENT_MULTIPLY,
                    OPERATOR_ASSIGNMENT_DIVIDE,
                    OPERATOR_ASSIGNMENT_MODULUS,
                    OPERATOR_ASSIGNMENT_EXPONENT,
                    OPERATOR_ASSIGNMENT_CONCATENATE,
                    
                    // Bitwise...
                    OPERATOR_BITWISE_AND,
                    OPERATOR_BITWISE_OR,
                    OPERATOR_BITWISE_XOR,
                    OPERATOR_BITWISE_NOT,
                    OPERATOR_BITWISE_SHIFT_LEFT,
                    OPERATOR_BITWISE_SHIFT_RIGHT,
                    
                    // Bitwise combined with an assignment...
                    OPERATOR_ASSIGNMENT_BITWISE_AND,
                    OPERATOR_ASSIGNMENT_BITWISE_OR,
                    OPERATOR_ASSIGNMENT_BITWISE_XOR,
                    OPERATOR_ASSIGNMENT_BITWISE_SHIFT_LEFT,
                    OPERATOR_ASSIGNMENT_BITWISE_SHIFT_RIGHT,
                    
                    // Logical...
                  __OPERATOR_LOGICAL_FIRST__,
                    OPERATOR_LOGICAL_AND = __OPERATOR_LOGICAL_FIRST__,
                    OPERATOR_LOGICAL_OR,
                  __OPERATOR_LOGICAL_LAST__,
                    OPERATOR_LOGICAL_NOT = __OPERATOR_LOGICAL_LAST__,
                    
                    // Relational...
                  __OPERATOR_RELATIONAL_FIRST__, /* Not real operator */
                    OPERATOR_RELATIONAL_EQUAL = __OPERATOR_RELATIONAL_FIRST__,
                    OPERATOR_RELATIONAL_NOT_EQUAL,
                    OPERATOR_RELATIONAL_LESS,
                    OPERATOR_RELATIONAL_GREATER,
                    OPERATOR_RELATIONAL_LESS_OR_EQUAL,
                  __OPERATOR_RELATIONAL_LAST__, /* Not real operator */
                    OPERATOR_RELATIONAL_GREATER_OR_EQUAL = __OPERATOR_RELATIONAL_LAST__,
                    
                    // Assignment...
                    OPERATOR_ASSIGNMENT

                }Operator;

            // Methods...

                // Create a lexer from a source code vector...
                CLexer(const std::vector<std::string> &UserSourceCode);

                // Get the current line starting from one...
                uint32                  GetCurrentHumanLineIndex() const;
                
                // Get the current line starting from one...
                std::string const       GetCurrentHumanLineString() const;

                // Get the current line starting from zero...
                uint32                  GetCurrentLineIndex() const;
                
                // Get the current operator, if applicable...
                Operator                GetCurrentOperator() const;
                
                // Get the current lexeme...
                std::string const      &GetCurrentLexeme();
                
                // Copy the current lexeme...
                void                    GetCurrentLexeme(std::string &sLexeme) 
                    const;

                // Get the current source line...
                const std::string      &GetCurrentSourceLine() const;

                // Get the first character of the next token...
                char                    GetLookAheadCharacter();
                
                // Get the next token...
                Token                   GetNextToken();
                
                // Get the next token as a string...
                std::string const       GetNextTokenAsString();
                
                // Get the current token...
                Token                   GetCurrentToken() const;
                
                // Get the current token as a string...
                std::string const       GetCurrentTokenAsString() const;

                // Initialize lexer state from another's...
                CLexer                 &operator=(const CLexer &SourceLexer);
                
                // Reset the lexer...
                void                    Reset();

                // Rewind / Shift back lexer one token...
                void                    Rewind();

                // Converts a token into a human readable string...
                const std::string       TokenToString(const Token token) const;

                // Deconstructor...
               ~CLexer();
        
        // Protected stuff...
        protected:

            // Data types...
                
                // Operator state...
                typedef struct _OperatorState
                {
                    // State character...
                    char        cCharacter;
                    
                    // Index into substate...
                    uint8       SubStateIndex;
                    
                    // Number of candidate substates...
                    uint8       SubStateCount;
                    
                    // Operator represented if no further substates are
                    //  transitioned to or are are none to begin with...
                    Operator    Index;

                }OperatorState;

            // Constants...
                
                // Maximum number of operator states...
                #define MAXIMUM_OPERATOR_STATES 32
                
                // Maximum number of delimIters...
                #define MAXIMUM_DELIMITER_COUNT 24
                
                // Lexer finite state machine states...
                enum STATE
                {
                    STATE_UNKNOWN   = 0,
                    STATE_START,
                    STATE_INTEGER,
                    STATE_FLOAT,
                    STATE_IDENTIFIER,
                    STATE_OPERATOR,
                    STATE_DELIMITER,
                    STATE_STRING,
                    STATE_STRING_ESCAPE,    /* \ */
                    STATE_STRING_CLOSE_QUOTE,
                };

                // Delimiters...
                char Delimiters[256];

                // Operator state tables...
                static const OperatorState OperatorStates_0[MAXIMUM_OPERATOR_STATES];
                static const OperatorState OperatorStates_1[MAXIMUM_OPERATOR_STATES];
                static const OperatorState OperatorStates_2[MAXIMUM_OPERATOR_STATES];

            // Variables...

                // Current line...
                uint32                          unCurrentLine;

                // Source code...
                std::vector<std::string> const &SourceCode;

                // Current token...
                Token                           CurrentToken;

                // Current lexeme...
                std::string                     sCurrentLexeme;

                // Current lexeme start and end indices...
                uint32                          unCurrentLexemeStart;
                uint32                          unCurrentLexemeEnd;

                // Current operator...
                Operator                        CurrentOperator;

                // Previous lexer state...
                uint32                          Backup_unCurrentLine;
                Token                           Backup_CurrentToken;
                std::string                     Backup_sCurrentLexeme;
                uint32                          Backup_unCurrentLexemeStart;
                uint32                          Backup_unCurrentLexemeEnd;
                Operator                        Backup_CurrentOperator;

            // Methods...

                // Backup lexer state...
                void            Backup();

                // Get the offset of the beginning of the current lexeme...
                uint32          GetLexemeStartIndex() const;

                // Get the next character...
                char            GetNextCharacter();

                // Get an operator state...
                OperatorState   GetOperatorState(uint8 CharacterIndex, 
                                                 uint8 StateIndex) const;

                // Get an operator state index...
                uint8 GetOperatorStateIndex(char cCharacter, uint8 CharacterIndex,
                                            uint8 SubStateIndex, 
                                            uint8 SubStateCount) const;

                // Character identification...
                boolean IsCharacterDelimiter(char cCharacter) const;
                boolean IsCharacterIdentifier(char cCharacter) const;
                boolean IsCharacterNumeric(char cCharacter) const;
                boolean IsCharacterOperator(char cCharacter, 
                                            uint8 CharacterIndex) const;
                boolean IsCharacterWhiteSpace(char cCharacter) const;
                
    };
}

#endif

