/*
  Name:         CPreProcessor.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to take a source code linked list and convert it into 
                compiler ready source code...
*/

// Includes...
#include "CPreProcessor.h"
#include <sstream>

// Use the Agni namespace...
using namespace Agni;

// Default constructor...
CPreProcessor::CPreProcessor(
    std::string const sInputRootPath, 
    std::list<std::string> const InputSourceCodeLinkedList)
    : sRootPath(sInputRootPath),
      SourceCodeLinkedList(InputSourceCodeLinkedList)
{

}

// Process source code linked list and produce final vector form...
std::vector<std::string> CPreProcessor::Process()
{
    // Variables and objects...
    std::list<std::string>::iterator    ListIterator;
    unsigned int                        unLine                      = 0;
    bool                                bWithinBlockComment         = false;
    bool                                bWithinString               = false;
    std::string                         sFileName;
    std::list<std::string>              NewSourceCodeLinkedList;
    std::vector<std::string>            NewSourceCode;

    // Try to pre-process our script...
    try
    {
        // Strip comments out of source code...
        for(ListIterator = SourceCodeLinkedList.begin(), unLine = 1;
            ListIterator != SourceCodeLinkedList.end();
          ++ListIterator, ++unLine)
        {
            // Extract line...
            std::string &sCurrentLine = *ListIterator;

            // Scan through each character...
            for(unsigned int unCharacterIndex = 0;
                unCharacterIndex < sCurrentLine.length();
                unCharacterIndex++)
            {
                // Found a quotation mark, toggle within string flag...
                if(sCurrentLine.at(unCharacterIndex) == '\"')
                    bWithinString = bWithinString ? false : true;
                
                // Found a single line comment...
                if((sCurrentLine.compare(unCharacterIndex, 2, "//") == 0) && 
                   !bWithinBlockComment && !bWithinString)
                {
                    // Kill it and the rest of the line with a new line character...
                    sCurrentLine.at(unCharacterIndex) = '\n';
                    sCurrentLine.erase(unCharacterIndex + 1);
                }
                
                // Found a block comment...
                if((sCurrentLine.compare(unCharacterIndex, 2, "/*") == 0) && 
                   !bWithinString)
                {
                    // *Gasp*, we're already within a block comment...
                    if(bWithinBlockComment)
                        throw "nested block comment detected";
                    
                    // Toggle block comment state...
                    bWithinBlockComment = true;
                }

                // Found end of a block comment...
                if((sCurrentLine.compare(unCharacterIndex, 2, "*/") == 0) && 
                   bWithinBlockComment)
                {
                    // Write over characters with white space...
                    sCurrentLine.replace(unCharacterIndex, 2, "  ");

                    // Untoggle block comment state...
                    bWithinBlockComment = false;
                }
                
                // We're reading stuff from within a block comment...
                if(bWithinBlockComment)
                {
                    // Unless this is a new line, wipe it out...
                    if(sCurrentLine.at(unCharacterIndex) != '\n')
                        sCurrentLine.at(unCharacterIndex) = ' ';
                }
            }
        }
        
        // Check for unterminated block comment...
        if(bWithinBlockComment)
            throw "unterminated block comment";
            
        // Handle directives...
        for(ListIterator = SourceCodeLinkedList.begin(), unLine = 1;
            ListIterator != SourceCodeLinkedList.end();
          ++ListIterator, ++unLine)
        {
            // Extract line and create a vector from it for lexer...
            std::string &sCurrentLine = *ListIterator;
            std::vector<std::string> SingleLineVector;
            SingleLineVector.push_back(sCurrentLine);

            // Create a lexer to process the single line...
            CLexer PreProcessorLexer(SingleLineVector);
   
            // Keep looking for directives until we reach the end...
            while(CLexer::TOKEN_END_OF_STREAM != 
                  PreProcessorLexer.GetNextToken())
            {
                // #include detected...
                if(PreProcessorLexer.GetCurrentToken() 
                    == CLexer::TOKEN_PREPROCESSOR_INCLUDE)
                {
                    // Load from disk...

                        // Verify the next token is indeed the path...                
                        if(PreProcessorLexer.GetNextToken() != CLexer::TOKEN_STRING)
                            throw std::string("bad #include usage");

                        // Grab the filename...
                        sFileName = PreProcessorLexer.GetCurrentLexeme();

                        // Create loader to retrieve structured source code...
                        CLoader Loader(sRootPath + sFileName);
                        
                        // Load the source code and save it in a linked list...
                        NewSourceCodeLinkedList = Loader.Load();
                    
                    // Now pre-process it and turn it into a vector...

                        // Create a new pre-processor...
                        CPreProcessor PreProcessor(sRootPath, 
                                                   NewSourceCodeLinkedList);
                        
                        // Recurse for new pre-processed included file...
                        NewSourceCode = PreProcessor.Process();

                    // Clear the line with the #include directive...
                    sCurrentLine = "\n";

                    // Insert the included source where the directive was found...
                    SourceCodeLinkedList.insert(ListIterator, 
                                                NewSourceCode.begin(), 
                                                NewSourceCode.end());
                }
            }
        }
    }

        // Pre-processing failed for some reason...
        catch(std::string const sReason)
        {
            // Generate partial, though useful, error message...
            std::ostringstream StringStream;
            StringStream << ":" << unLine << ": error: " << sReason;

            // Pass message to compiler's higher level exception handler...
            throw StringStream.str();
        }

    // Convert linked list to finalized vector...
    std::vector<std::string> SourceCodeVector(SourceCodeLinkedList.begin(),
                                              SourceCodeLinkedList.end());

    // Return the fully processed source code, ready for parsing...
    return SourceCodeVector;
}

