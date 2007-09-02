CPP  = g++
OBJ  = CAgni.o
CXXINCS = -I"../../dist/include"
BIN  = ../../dist/lib/libAgni.a
CXXFLAGS = $(CXXINCS) -Wall -Werror -g3
RM = rm -f

all: $(BIN)

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	ar rcsv $(BIN) $(OBJ)
	ranlib $(BIN)

CAgni.o: CAgni.cpp ../../dist/include/CAgni.h ../../dist/include/AgniPlatformSpecific.h ../../dist/include/AgniCommonDefinitions.h
	$(CPP) -c CAgni.cpp -o CAgni.o $(CXXFLAGS)
