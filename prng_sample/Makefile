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
PRIME_SRC:=$(SRC)/prime_sample.cpp
PRIME_TEST_SRC:=$(SRC)/test_prime.cpp

ifeq ($(OS),Windows_NT)
PRNG_LIBS:=-ladvapi32
endif

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

# Compile the primality and prime-generation module for integration.
prime-module: | folders
	@echo "### Compiling prime generation module"
	$(CPP) $(CPP_FLAGS) $(INCLUDE) -c $(PRIME_SRC) -o $(BUILD)/prime_sample.o

test-prime: | folders
	@echo "### Compiling prime generation tests"
	$(CPP) $(CPP_FLAGS) $(PRIME_TEST_SRC) $(PRIME_SRC) $(DH_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@

run-test-prime: test-prime
	./$(BIN)/test-prime

clean:
	rm -rf ./bin

.PHONY: folders prng run-prng test-u512 run-test-u512 prime-module test-prime run-test-prime clean
