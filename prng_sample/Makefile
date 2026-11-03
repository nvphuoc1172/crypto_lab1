ifeq ($(ROOT),)
ROOT:=.
endif

CPP:=g++
CPP_FLAGS:=-O3 -Wall -Wextra -Wpedantic -Wvla -Wredundant-decls 

SHA3_SRC:=$(ROOT)/lib/fips202/fips202.c
SHA3_INCLUDE:=-I $(ROOT)/lib/fips202

SRC:=$(ROOT)/src
INCLUDE:=-I $(ROOT)/src -lntl -lgf2x -lgmp -pthread
LIB:=$(SHA3_INCLUDE)

MAIN_PRNG:=$(ROOT)/src/main_prng.cpp
PRNG_SRC:=$(ROOT)/lib/fips202/prng.cpp

LIB_OBJS:=fips202.o

BIN:=bin
BUILD:=bin/build

folders:
	@echo -e "\n### Creating folders\n"
	mkdir -p $(BUILD) $(BIN)

fips202.o: | folders
	@/bin/echo -e "\n### Compiling fips202"
	$(CPP) $(CFLAGS) -c $(SHA3_SRC) $(SHA3_INCLUDE) -o $(BUILD)/$@

%.o: $(SRC)/%.cpp | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CPP) $(CPP_FLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

prng: | folders
	@echo -e "\n### Compiling prng\n"
	$(CPP) $(CPP_FLAGS) $(MAIN_PRNG) $(PRNG_SRC) $(SHA3_SRC) -I $(SRC) $(LIB) -o $(BIN)/$@

run-prng: prng
	./$(BIN)/prng

clean:
	rm -rf ./bin
