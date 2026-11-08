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
KEY_SRC:=$(SRC)/private_key.cpp
EXCHANGE_SRC:=$(SRC)/dh_exchange.cpp
PRIME_SRC:=$(SRC)/prime_sample.cpp
PRIME_TEST_SRC:=$(SRC)/test_prime.cpp
KEY_TEST_SRC:=$(SRC)/test_private_key.cpp

ifeq ($(OS),Windows_NT)
PRNG_LIBS:=-ladvapi32
SHELL:=cmd.exe
EXE:=.exe
RUN_BIN:=bin\$(empty)
else
RUN_BIN:=./bin/
endif

BIN:=bin
BUILD:=bin/build

folders:
ifeq ($(OS),Windows_NT)
	@if not exist "bin\build" mkdir "bin\build"
else
	@mkdir -p $(BUILD) $(BIN)
endif

# Demo PRNG
prng: | folders
	@echo "### Compiling prng"
	$(CPP) $(CPP_FLAGS) $(SRC)/main_prng.cpp $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

run-prng: prng
	$(RUN_BIN)prng$(EXE)

# Complete Diffie-Hellman Key Exchange Demo
dh: | folders
	@echo "### Compiling Diffie-Hellman demo"
	$(CPP) $(CPP_FLAGS) $(SRC)/main_dh.cpp $(EXCHANGE_SRC) $(DH_SRC) $(KEY_SRC) $(PRIME_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

run-dh: dh
	$(RUN_BIN)dh$(EXE)

# Unit test for BigInt 512/1024 and modular exponentiation
test-u512: | folders
	@echo "### Compiling test_u512"
	$(CPP) $(CPP_FLAGS) $(SRC)/test_u512.cpp $(U512_SRC) $(DH_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

run-test-u512: test-u512
	$(RUN_BIN)test-u512$(EXE)

# Unit test for Diffie-Hellman private key generation
test-private-key: | folders
	@echo "### Compiling private key generation tests"
	$(CPP) $(CPP_FLAGS) $(KEY_TEST_SRC) $(KEY_SRC) $(DH_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

run-test-private-key: test-private-key
	$(RUN_BIN)test-private-key$(EXE)

# Compile the primality and prime-generation module for integration.
prime-module: | folders
	@echo "### Compiling prime generation module"
	$(CPP) $(CPP_FLAGS) $(INCLUDE) -c $(PRIME_SRC) -o $(BUILD)/prime_sample.o

test-prime: | folders
	@echo "### Compiling prime generation tests"
	$(CPP) $(CPP_FLAGS) $(PRIME_TEST_SRC) $(PRIME_SRC) $(DH_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

run-test-prime: test-prime
	$(RUN_BIN)test-prime$(EXE)

test-regressions: | folders
	$(CPP) $(CPP_FLAGS) $(SRC)/test_regressions.cpp $(U512_SRC) $(DH_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

test-private-key-extended: | folders
	$(CPP) $(CPP_FLAGS) $(SRC)/test_private_key_extended.cpp $(KEY_SRC) $(DH_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

test-prng: | folders
	$(CPP) $(CPP_FLAGS) $(ROOT)/tests/prng_test.cpp $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

test-dh: dh
	$(RUN_BIN)dh$(EXE) --test

test-dh-exchange: | folders
	$(CPP) $(CPP_FLAGS) $(SRC)/test_dh_exchange.cpp $(EXCHANGE_SRC) $(KEY_SRC) $(DH_SRC) $(U512_SRC) $(PRNG_SRC) $(SHA3_SRC) $(INCLUDE) $(PRNG_LIBS) -o $(BIN)/$@$(EXE)

all: test-dh-exchange prng dh test-u512 test-private-key test-private-key-extended test-prime test-regressions test-prng

test: all
	$(RUN_BIN)test-dh-exchange$(EXE)
	$(RUN_BIN)prng$(EXE)
	$(RUN_BIN)test-u512$(EXE)
	$(RUN_BIN)test-private-key$(EXE)
	$(RUN_BIN)test-private-key-extended$(EXE)
	$(RUN_BIN)test-prng$(EXE)
	$(RUN_BIN)dh$(EXE) --test
	$(RUN_BIN)test-prime$(EXE)
	$(RUN_BIN)test-regressions$(EXE)

clean:
ifeq ($(OS),Windows_NT)
	@if exist bin\*.exe del /q bin\*.exe
	@if exist bin\build\*.o del /q bin\build\*.o
else
	rm -rf ./bin
endif

.PHONY: folders prng run-prng test-u512 run-test-u512 test-private-key run-test-private-key prime-module test-prime run-test-prime clean

.PHONY: all test dh run-dh test-regressions

.PHONY: test-dh test-prng test-private-key-extended

.PHONY: test-dh-exchange
