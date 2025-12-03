EXEC_TX_RX = TX_RX_energy_test
EXEC_TX = TX_gen_cw_noisy
# EXEC = RX_energy_test
EXEC_RX = RX_energy_test


AFF3CT = /home/jrosseel/aff3ct
STREAMPU = $(AFF3CT)/lib/streampu

# AFF3CT IS ASSUMED TO BE STATICALLY COMPILED AND IT INTEGRATES STREAMPU (-DAFF3CT_COMPILE_STATIC_LIB=ON -DAFF3CT_INCLUDE_SPU_LIB=ON)

DEF = -DAFF3CT_POLAR_BIT_PACKING
DEF += -DAFF3CT_EXT_STRINGS
# DEF += -DAFF3CT_8BIT_PREC
# DEF += -DAFF3CT_16BIT_PREC
# DEF += -DAFF3CT_32BIT_PREC
# DEF += -DAFF3CT_64BIT_PREC
DEF += -DAFF3CT_MULTI_PREC
# DEF += -DAFF3CT_CHANNEL_GSL
# DEF += -DAFF3CT_CHANNEL_MKL
# DEF += -DAFF3CT_MPI

DEF += -DSPU_COLORS
DEF += -DSPU_STACKTRACE
DEF += -DSPU_TESTS
# DEF += -DSPU_STACKTRACE_SEGFAULT
# DEF += -DSPU_FAST
# DEF += -DSPU_HWLOC


SRC_TX = main_TX.cpp LoggerModule.cpp 
SRC_RX = main_RX_energy_test.cpp LoggerModule.cpp 
SRC_TX_RX = main_TX_RX.cpp LoggerModule.cpp

INC = -I$(AFF3CT)/include \
	-I$(AFF3CT)/src \
    -I$(AFF3CT)/lib/MIPP/src \
    -I$(AFF3CT)/lib/date/include/date \
    -I$(AFF3CT)/lib/cli/src \
	-I$(STREAMPU)/include \
    -I$(STREAMPU)/src \
	-I$(STREAMPU)/lib/rang/include \
	-I$(STREAMPU)/lib/json/include \
    -I/usr/include 

LIBS = $(AFF3CT)/build/lib/libaff3ct-4.0.0.a

ifeq ($(findstring -DSPU_STACKTRACE, $(DEF)), -DSPU_STACKTRACE)
    $(info Link with cpptrace)
    INC += -I$(STREAMPU)/lib/cpptrace/include
	LIBS += $(AFF3CT)/build/lib/streampu/lib/cpptrace/lib/libcpptrace.a
endif
ifeq ($(findstring -DSPU_HWLOC, $(DEF)), -DSPU_HWLOC)
    $(info Link with hwloc not supported (yet))
	exit(0)
endif
ifeq ($(findstring -DAFF3CT_CHANNEL_GSL, $(DEF)), -DAFF3CT_CHANNEL_GSL)
    $(info Link with gsl not supported (yet))
	exit(0)
endif
ifeq ($(findstring -DAFF3CT_CHANNEL_MKL, $(DEF)), -DAFF3CT_CHANNEL_MKL)
    $(info Link with mkl not supported (yet))
	exit(0)
endif
ifeq ($(findstring -DAFF3CT_MPI, $(DEF)), -DAFF3CT_MPI)
    $(info Link with mpi not supported (yet))
	exit(0)
endif

LIB_DIR = $(AFF3CT)/build/lib

# $(STREAMPU) cpptrace
# $(STREAMPU) pthread
# $(STREAMPU) hwloc
# $(STREAMPU) self

# $(AFF3CT) gsl
# $(AFF3CT) mkl
# $(AFF3CT) mpi
# $(AFF3CT) pthread ?

CXX = g++
CXXFLAGS = -std=c++17 $(DEF) $(INC) -O3 -funroll-loops -pg -g -lCPP_Joules -lstdc++fs
LDFLAGS = $(foreach dir,$(LIB_DIR),-L$(dir)) $(LIBS) $(DEF) -g -lCPP_Joules

all: $(EXEC_TX) $(EXEC_RX) $(EXEC_TX_RX)

$(EXEC_TX): $(SRC_TX)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(EXEC_RX): $(SRC_RX)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)	

$(EXEC_TX_RX): $(SRC_TX_RX)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

valgrind: $(EXEC_TX) $(EXEC_RX) $(EXEC_TX_RX)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC_TX) ./$(EXEC_RX) ./$(EXEC_TX_RX)

clean:
	rm -f $(EXEC_TX) $(EXEC_RX) $(EXEC_TX_RX)

.PHONY: all clean