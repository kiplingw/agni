# SCons declarative input file that controls how Agni is built...

# Imports...
import os

# Grab environment object and prepare prettier build messages......
env = Environment(CXXCOMSTR     = "Compiling $SOURCE ...",
                  SHCXXCOMSTR   = "Compiling shared object $TARGET ...",
                  LINKCOMSTR    = "Linking $TARGET ...",
                  ASCOMSTR      = "Archiving $TARGET ...",
                  RANLIBCOMSTR  = "Indexing $TARGET ...",
                  SHLINKCOMSTR  = "Linking shared library $TARGET ...")

# SCons internal settings...
env.SourceSignatures('MD5')

# Versioning...
env.VERSION_MAJOR   = 0
env.VERSION_MINOR   = 95
env.VERSION_SVN     = os.popen('svnversion .').read()[:-1]
env.VERSION_SVN     = env.VERSION_SVN.split(':')[-1]

# Common flags...
env.Append(CPPFLAGS = " -Wall -Werror ")
env.Append(CPPDEFINES=[('AGNI_VERSION_MAJOR', env.VERSION_MAJOR),
                       ('AGNI_VERSION_MINOR', env.VERSION_MINOR),
                       ('AGNI_VERSION_SVN', "'\"" + env.VERSION_SVN + "\"'")])

# Debugging enabled?
debug = ARGUMENTS.get('debug', 1)
if int(debug):
    env.Append(CPPFLAGS = '-g3')
else:
    env.Append(CPPFLAGS = '-O3')

# Build assembler...
assembler = env.Program('aga', ['src/assembler/Main.cpp', 
                        'src/assembler/Assembler.cpp'])
env.Alias('assembler', assembler)

# Build compiler...
compiler = env.Program('agc', ['src/compiler/Main.cpp',
                       'src/compiler/CAgniMachineTarget.cpp',
                       'src/compiler/Compiler.cpp',
                       'src/compiler/CLexer.cpp',
                       'src/compiler/CLoader.cpp',
                       'src/compiler/CMachineTarget_Base.cpp',
                       'src/compiler/CParser.cpp',
                       'src/compiler/CPreProcessor.cpp'])
env.Alias('compiler', compiler)

# Build virtual machine...
avm = env.SharedLibrary('agni', ['src/virtualmachine/VirtualMachine.cpp'])
env.Alias('vm', avm)

# Build virtual machine test...
avmtest = env.Program('avmtest', 
            ['src/virtualmachine/testing/VirtualMachineTest.cpp'], 
            LIBS = ['agni'],
            LIBPATH = '.',
            CPPPATH = "src/include")
env.Depends(avmtest, avm)

