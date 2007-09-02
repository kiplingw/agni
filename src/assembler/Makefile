CPP  = g++
OBJ  = AgniAssemblerMain.o CAgniAssembler.o
CXXINCS = -I"../../dist/include"
BIN  = ../../dist/bin/aa
CXXFLAGS = $(CXXINCS) -Wall -Werror -g3
RM = rm -f

all: $(BIN)

test: all
	echo Testing the AgniAssembler we just built...
	$(BIN) -a ../../dist/examples/scripts/Test.agl -o ../../dist/examples/scripts/Test -V

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN)

AgniAssemblerMain.o: AgniAssemblerMain.cpp
	$(CPP) -c AgniAssemblerMain.cpp -o AgniAssemblerMain.o $(CXXFLAGS)

CAgniAssembler.o: CAgniAssembler.cpp CAgniAssembler.h
	$(CPP) -c CAgniAssembler.cpp -o CAgniAssembler.o $(CXXFLAGS)
