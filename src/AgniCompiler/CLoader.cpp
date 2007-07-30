/*
  Name:         CLoader.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  Code to load user script from disk or stream into memory...
*/

// Includes...
#include "CLoader.h"

// Default constructor...
CLoader::CLoader(const string sInputFileName)
    : sFileName(sInputFileName)
{

}

// Load source code or throw a const string...
list<string> CLoader::Load() const
{
    // Variables...
    FILE           *hFile                   = NULL;
    char            cCharacterRead          = '\x0';
    string          sCurrentLine;
    list<string>    SourceCodeLinkedList;
    
    // Open the input file...
    hFile = fopen(sFileName.c_str(), "r");

        // Failed to open file...
        if(!hFile)
            throw string("cannot read from \"" + sFileName + "\"");

    // Keep reading characters until we hit the end of the file...
    while(EOF != (cCharacterRead = fgetc(hFile)))
    {
        // Read a character...
        switch(cCharacterRead)
        {
            // Win32 carriage returns should be ignored...
            case '\r':
                break;
            
            // New line signals the end of current one...
            case '\n':
            
                // Store new line character cince lexer needs it. Store line...
                sCurrentLine += '\n';
                SourceCodeLinkedList.push_back(sCurrentLine);
                
                // Clear line for next one...
                sCurrentLine.clear();
                
                // Done...
                break;
            
            // Anything else should be appended to the line...
            default:
            
                // Add character to line...
                sCurrentLine += cCharacterRead;
                
                // Done...
                break;
        }
    }

    // Add the last line and ensure the file ends with a new line character...
    SourceCodeLinkedList.push_back(sCurrentLine + '\n');
    
    // Done with file...
    fclose(hFile);
    
    // Return the linked list script...
    return SourceCodeLinkedList;
    
}
