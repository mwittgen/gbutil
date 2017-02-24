# These should come from environment:
# CXX
# OPTFLAGS

# TMV_DIR
# FFTW_DIR
# YAML_DIR
# MKL_DIR (optionally)

export CXX
export OPTFLAGS

ABS_INCLUDES = -I $(FFTW_DIR)/include -I $(TMV_DIR)/include -I $(YAML_DIR)/include -I $(EIGEN_DIR)

ifdef MKL_DIR
ABS_INCLUDES += -I $(MKL_DIR)/include
endif

SUBDIRS = 

INCLUDES = 

CXXFLAGS = $(OPTFLAGS) $(ABS_INCLUDES) $(INCLUDES) 

SRC = $(shell ls *.cpp)

OBJ = BinomFact.o fft.o StringStuff.o Poisson.o Table.o Pset.o odeint.o Poly2d.o \
	Interpolant.o Expressions.o Shear.o Lookup1d.o

all: $(OBJ)

# For building test programs:
LIB_DIRS = -L $(CFITSIO_DIR)/lib -L $(TMV_DIR)/lib -L $(FFTW_DIR)/lib \
	-L $(YAML_DIR)/lib
TMV_LINK := $(shell cat $(TMV_DIR)/share/tmv/tmv-link)
LIBS = -lm $(LIB_DIRS) -lyaml-cpp -lfftw3 -lcfitsio -ltmv_symband $(TMV_LINK)

test_lookup: test_lookup.o $(OBJ) 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

testLinalg: testLinalg.o 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

testLinalg2: testLinalg2.o 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

###############################################################
## Standard stuff:
###############################################################

subs:
	for dir in $(SUBDIRS); do (cd $$dir && $(MAKE)); done

depend:
	for dir in $(SUBDIRS); do (cd $$dir && $(MAKE) depend); done
	$(CXX) $(CXXFLAGS) -MM $(SRC) > .$@

clean:
	for dir in $(SUBDIRS); do (cd $$dir && $(MAKE) clean); done
	rm -f *.o *~ core .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

export

.PHONY: all install dist depend clean 
