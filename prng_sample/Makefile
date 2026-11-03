ifeq ($(ROOT),)
ROOT:=.
endif

CPP:=g++
CPP_FLAGS:=-std=c++14 -O3 -Wall -Wextra -Wpedantic -Wvla -Wredundant-decls

SHA3_SRC:=$(ROOT)/lib/fips202/fips202.c
SHA3_INCLUDE:=-I $(ROOT)/lib/fips202

SRC:=$(ROOT)/src
INCLUDE:=-I $(ROOT)/src $(SHA3_INCLUDE)

PRNG_SRC:=$(ROOT)/lib/fips202/prng.cpp
U512_SRC:=$(SRC)/u512_sample.cpp
DH_SRC:=$(SRC)/dh_sample.cpp

BIN:=bin
BUILD:=bin/build

folders:
	@mkdir -p $(BUILD) $(BIN)

# Demo PRNG
prng: | folders
	@echo "### Compiling prng"
	$(CPP) $(CPP_FLAGS) $(SRC)/main_prng.cpp $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) -o $(BIN)/$@

run-prng: prng
	./$(BIN)/prng

# Unit test for BigInt 512/1024 and modular exponentiation
test-u512: | folders
	@echo "### Compiling test_u512"
	$(CPP) $(CPP_FLAGS) $(SRC)/test_u512.cpp $(U512_SRC) $(DH_SRC) $(INCLUDE) -o $(BIN)/$@

run-test-u512: test-u512
	./$(BIN)/test-u512

clean:
	rm -rf ./bin
