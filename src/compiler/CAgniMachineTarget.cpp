/*
  Name:         CAgniMachineTarget.cpp (implementation)
  Author:       Kip Warner (Kip@TheVertigo.com)
  Description:  The AVM target machine backend receives i-code from the parser
                and translates to assembly listing for the Agni assembler...
*/

// Includes...
#include "CAgniMachineTarget.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <ctime>
#include <list>

// Using the Agni namepace...
using namespace Agni;

// Default constructor...
CAgniMachineTarget::CAgniMachineTarget(
    Agni::CParser const &InputParser, std::string const &_sOutputAssemblyListing)
    : CMachineTarget_Base(InputParser, _sOutputAssemblyListing),
      TabStopWidth(4)
{

}

// Get the output listing file extension for this target...
std::string const CAgniMachineTarget::GetListingFileExtension() const
{
    // Return it...
    return std::string("agl");
}

// Emit assembly listing for target or throw string...
void CAgniMachineTarget::EmitAssemblyListing() throw(std::string const)
{
    // Objects, variables, and structures...
    std::ofstream           Output;
    time_t                  RawTime     = 0;
    struct tm              *pTimeInfo   = NULL;
    Agni_MainHeader const  &MainHeader  = Parser.GetMainHeader();
    
    // Try to write out listing...
    try
    {
        // Open the file...
        Output.open(GetOutputFile().c_str(), std::ios::out);

            // Failed...
            if(!Output.is_open())
                throw std::string("access denied to write assembly listing");

        // Write header...
        
            // File name...
            Output << "; Name: " << GetOutputFile() << std::endl;
            
            // The Agni version...
            Output << "; Agni Version: " << AGNI_VERSION_MAJOR << "." 
                   << AGNI_VERSION_MINOR << std::endl;

            // Compile time...
                
                // Get raw time and convert to local time...
                time(&RawTime);
                pTimeInfo = localtime(&RawTime);

                // Output...
                Output << "; Compiled: " << asctime(pTimeInfo) << std::endl;

        // Write directives...
        Output << "; Preprocessor directives..." << std::endl << std::endl;
                
                // Stack size...
                Output << "\t; Stack size..." << std::endl
                       << "\tSetStackSize " << MainHeader.unStackSize
                       << std::endl << std::endl;

                // Thread priority...
                Output << "\t; Thread priority..." << std::endl
                       << "\tSetThreadPriority Low" << std::endl << std::endl;

        // Write global variables...
        
            // Create iterator...
            std::map<CParser::VariableName, CParser::CVariable>::const_iterator 
                GlobalVariableIterator;

            // We only want a header if there are any globals...
            bool bHasEmitGlobal = false;

            // Emit all globals...
            for(GlobalVariableIterator = Parser.GetVariableTable().begin();
                GlobalVariableIterator != Parser.GetVariableTable().end();
              ++GlobalVariableIterator)
            {
                // Get variable name...
                CParser::VariableName const &Name = 
                    GlobalVariableIterator->first;
                
                // Get the variable object...
                CParser::CVariable const &Variable = 
                    GlobalVariableIterator->second;
                
                // Not a global...
                if(Name.second != CParser::Global)
                    continue;

                // Emit header, if not already...
                if(!bHasEmitGlobal)
                    Output << "; Global variables..." << std::endl;

                // Emit global variable...
                if(Variable.unSize == 1)
                    Output << "var " << Name.first << ";" << std::endl;
                
                // Emit global array...
                else
                    Output << "var " << Name.first << "[" << Variable.unSize
                           << "];" << std::endl;

                // Remember that we have found at least one global...
                bHasEmitGlobal = true;
            }
            
            // Globals were emitted, shove in some white space for more stuff...
            if(bHasEmitGlobal)
                Output << std::endl;

        // Emit each function...
            
            // Create iterator...
            std::map<CParser::FunctionName, CParser::CFunction>::const_iterator 
                FunctionIterator;

            // Emit each function, except Main... (we do that last)
            for(FunctionIterator = Parser.GetFunctionTable().begin();
                FunctionIterator != Parser.GetFunctionTable().end();
              ++FunctionIterator)
            {
                // Get the function name...
                CParser::FunctionName const &Name = FunctionIterator->first;
                
                // Get the function object...
                CParser::CFunction const &Function = FunctionIterator->second;
                
                // This is the main function, skip it...
                if(Name == CParser::FunctionName("Main"))
                    continue;
                
                // This is a host function, skip it...
                if(Function.bIsHostFunction)
                    continue;

                // Emit the function...
                EmitFunction(Output, Name, Function);
            }
            
            // Emit Main function, if any...
                
                // Try to find Main...
                FunctionIterator = Parser.GetFunctionTable().find(
                    CParser::FunctionName("Main"));

                // Found...
                if(FunctionIterator != Parser.GetFunctionTable().end())
                    EmitFunction(Output, FunctionIterator->first, 
                                 FunctionIterator->second);

            // Emit tail header...
            Output << "; eof..." << std::endl << std::endl;
    }

        // Failed to write listing...
        catch(std::string const sReason)
        {
            // Format...
            std::string const sBetterReason = 
                std::string(": error: ") + sReason;

            // Pass up the error chain...
            throw sBetterReason;
        }
}

// Emit a function declaration and body...
void CAgniMachineTarget::EmitFunction(std::ofstream &Output, 
    CParser::FunctionName FunctionName, CParser::CFunction const &Function) 
    throw(std::string const)
{
    // Variables...
    std::string sParametersBlock;
    std::string sLocalVariablesBlock;

    // Emit header...

        // For main...
        if(FunctionName == CParser::FunctionName("Main"))
            Output << "; Main function follows... (entry point)" << std::endl;
        
        // Otherwise emit ordinary header...
        else
            Output << "; " << FunctionName << " function follows..." 
                   << std::endl;

    // Emit declaration and brace for opening code block...
    Output << "func " << FunctionName << std::endl
           << "{" << std::endl;

    // Create iterator for scanning through variable table...
    std::map<CParser::VariableName, CParser::CVariable>::const_iterator 
        VariableIterator;

    // Prepare emissions for all of function's parameters and local variables...
    for(VariableIterator = Parser.GetVariableTable().begin();
        VariableIterator != Parser.GetVariableTable().end();
      ++VariableIterator)
    {
        // Get variable name...
        CParser::VariableName const &VariableName = VariableIterator->first;
        
        // Get variable object...
        CParser::CVariable const &Variable = VariableIterator->second;
        
        // Scope is not for our function...
        if(Function.GetIndex() != VariableName.second)
            continue;
        
        // This is a parameter...
        if(Variable.Type == CParser::CVariable::Parameter)
        {
            // Append to parameter emission block...
            sParametersBlock += "\tparam " + VariableName.first + ";\n";
            
            // This should not be an array...
            assert(Variable.unSize == 1);
        }
        
        // This is a local variable...
        else
        {
            // Prepare emission for a variable...
            if(Variable.unSize == 1)
                sLocalVariablesBlock += "\tvar " + VariableName.first + ";\n";

            // Prepare emission for an array...
            else
            {
                // Append to parameter emission block...
                sLocalVariablesBlock += "\tvar " + VariableName.first + "[";
                sLocalVariablesBlock += Variable.unSize;
                sLocalVariablesBlock += "]\n";
            }
        }
    }

    // Emit parameters, if any...
    if(Function.Parameters > 0)
    {
        // Header...
        Output << "\t; Parameters..." << std::endl;
        
        // Emit parameters block...
        Output << sParametersBlock << std::endl;
    }

    // Emit local variables, if any...
    if(!sLocalVariablesBlock.empty())
    {
        // Header...
        Output << "\t; Local variables..." << std::endl;
        
        // Emit local variables block...
        Output << sLocalVariablesBlock << std::endl;
    }

    // Create iterator for i-code...
    std::list<CParser::ICodeNode>::const_iterator ICodeIterator;

    // Emit code until none left...
    for(ICodeIterator = Function.ICodeList.begin();
        ICodeIterator != Function.ICodeList.end();
      ++ICodeIterator)
    {
        // Get the current i-code node...
        CParser::ICodeNode const &ICodeNode = *ICodeIterator;
        
        // Emit appropriately based on node type...
        switch(ICodeNode.Type)
        {
            // Annotation...
            case CParser::ICodeNode::ANNOTATION:
            {
                // Emit...
                Output << "\t; " << ICodeNode.sAnnotation << std::endl;
                
                /*
                    TODO: Finish this.
                */
                
                // Done...
                break;            
            }
            
            // Instruction...
            case CParser::ICodeNode::INSTRUCTION:
            {
                // Done...
                break;            
            }
            
            // Jump target...
            case CParser::ICodeNode::JUMP_TARGET:
            {
                // Done...
                break;
            }
            
            // Unknown...
            default:
                throw std::string("internal fault, unknown i-code node type");
        }
    }

    
    // If there was no code, leave a comment saying so...
    if(Function.ICodeList.empty())
        Output << "\t; No code..." << std::endl;
    
    // Emit closing brace...
    Output << "}" << std::endl << std::endl;
}

