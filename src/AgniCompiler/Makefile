CPP  = g++
OBJ  = AgniCompilerMain.o CAgniCompiler.o CLexer.o CLoader.o CParser.o CPreProcessor.o
CXXINCS = -I"../../dist/include"
BIN  = ../../dist/bin/ac
AGE  = ../../dist/examples/scripts/Test.age ../../dist/examples/scripts/Random.age ../../dist/examples/scripts/Example.age
CXXFLAGS = $(CXXINCS) -Wall -Werror -g3
RM = rm -f

all: $(BIN)

test: all
	$(BIN) --version
	$(BIN) -c ../../dist/examples/scripts/Thing.ags -o ../../dist/examples/scripts/Thing -V

test_tokenizer: all
	$(BIN) --version
	$(BIN) -c ../../dist/examples/scripts/Thing.ags -o ../../dist/examples/scripts/Thing -V --dump-token-stream

test_preprocessor: all
	$(BIN) --version
	$(BIN) -c ../../dist/examples/scripts/Thing.ags -o ../../dist/examples/scripts/Thing -V --preprocess

test_emitter: all
	$(BIN) --version
	$(BIN) -c ../../dist/examples/scripts/Thing.ags -o ../../dist/examples/scripts/Thing -V --stop

debug:
	ddd -foreground black -background grey --args $(BIN) -c ../../dist/examples/scripts/Thing.ags -o ../../dist/examples/scripts/Thing -V

clean:
	${RM} $(OBJ) $(BIN) $(AGE)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN)

AgniCompilerMain.o: AgniCompilerMain.cpp
	$(CPP) -c AgniCompilerMain.cpp -o AgniCompilerMain.o $(CXXFLAGS)

CAgniCompiler.o: CAgniCompiler.cpp CAgniCompiler.h
	$(CPP) -c CAgniCompiler.cpp -o CAgniCompiler.o $(CXXFLAGS)

CLexer.o: CLexer.cpp CLexer.h
	$(CPP) -c CLexer.cpp -o CLexer.o $(CXXFLAGS)

CLoader.o: CLoader.cpp CLoader.h
	$(CPP) -c CLoader.cpp -o CLoader.o $(CXXFLAGS)

CParser.o: CParser.cpp CParser.h
	$(CPP) -c CParser.cpp -o CParser.o $(CXXFLAGS)

CPreProcessor.o: CPreProcessor.cpp CPreProcessor.h
	$(CPP) -c CPreProcessor.cpp -o CPreProcessor.o $(CXXFLAGS)
