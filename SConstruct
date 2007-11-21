# SCons declarative input file that controls how Agni is built...

# Imports...
import os

# Create environment...
env = Environment()

# SCons internal settings...
env.SourceSignatures('MD5')

# Versioning...
VERSION_MAJOR   = 0
VERSION_MINOR   = 94
VERSION_SVN     = os.popen('echo -n `LANG=C svn info | grep ^Revision | cut -d\  -f2`').read()

# Common flags...
env.Append(CPPFLAGS = '-DAGNI_VERSION_MAJOR=' + str(VERSION_MAJOR) + " " \
                      '-DAGNI_VERSION_MINOR=' + str(VERSION_MINOR) + " " \
                      '-DAGNI_VERSION_SVN=' + str(VERSION_SVN) + 
                      " -Wall -Werror ")

# Debugging enabled?
debug = ARGUMENTS.get('debug', 1)
if int(debug):
    env.Append(CPPFLAGS = '-g3')
else:
    env.Append(CPPFLAGS = '-O3')

# Build assembler...
env.Program('assembler', ['src/assembler/Main.cpp', 
                          'src/assembler/Assembler.cpp'])

# Build compiler...
env.Program('compiler', ['src/compiler/Main.cpp',
                         'src/compiler/Compiler.cpp',
                         'src/compiler/CLexer.cpp',
                         'src/compiler/CLoader.cpp',
                         'src/compiler/CParser.cpp',
                         'src/compiler/CPreProcessor.cpp'])

# Build virtual machine...
avm = env.SharedLibrary('agni', ['src/virtualmachine/VirtualMachine.cpp'])

# Build virtual machine test...
avmtest = env.Program('avmtest', 
            ['src/testing/VirtualMachineTest.cpp'], 
            LIBS = ['agni'],
            LIBPATH = '.')
env.Depends(avmtest, avm)

