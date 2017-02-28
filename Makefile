# These should come from environment:
# CXX
# OPTFLAGS

# TMV_DIR
# FFTW_DIR
# YAML_DIR
# MKL_DIR (optionally)

export CXX
export OPTFLAGS

CXXFLAGS := $(OPTFLAGS)

LIBS := -lm

# Collect the includes and libraries we need
ifdef FFTW_DIR
CXXFLAGS += -I $(FFTW_DIR)/include
LIBS += -L $(FFTW_DIR)/lib -lfftw3
else
$(error Require FFTW_DIR in environment)
endif

ifdef YAML_DIR
CXXFLAGS += -I $(YAML_DIR)/include
LIBS += -L $(YAML_DIR)/lib -lyaml-cpp
else
$(error Require YAML_DIR in environment)
endif

ifdef TMV_DIR
CXXFLAGS += -I $(TMV_DIR)/include -D USE_TMV
LIBS += $(shell cat $(TMV_DIR)/share/tmv/tmv-link) -ltmv_symband 
endif

ifdef EIGEN_DIR
CXXFLAGS += -I $(EIGEN_DIR)/include -D USE_EIGEN
endif

# Check that either TMV or EIGEN are available (ok to have both)
$(if $(or $(TMV_DIR),$(EIGEN_DIR)), , $(error Need either TMV_DIR or EIGEN_DIR))

ifdef MKL_DIR
CXXFLAGS += -I $(MKL_DIR)/include -D USE_MKL
endif

# External directories where we'll need to clean/build dependents
EXTDIRS = 

OBJ = BinomFact.o fft.o StringStuff.o Poisson.o Table.o Pset.o odeint.o Poly2d.o \
	Interpolant.o Expressions.o Shear.o Lookup1d.o

all: $(OBJ)

test_lookup: test_lookup.o $(OBJ) 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

tests/testMarquardt.o: tests/testMarquardt.cpp
	$(CXX) $(CXXFLAGS) -I . -c $^ -o $@

tests/testMarquardt: tests/testMarquardt.o 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

testLinalg: testLinalg.o 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

testLinalg2: testLinalg2.o 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

###############################################################
## Standard stuff:
###############################################################

exts:
	for dir in $(EXTDIRS); do (cd $$dir && $(MAKE)); done

local-depend:
	$(CXX) $(CXXFLAGS) -MM $(SRC) > .$@

depend: local-depend
	for dir in $(EXTDIRS); do (cd $$dir && $(MAKE) depend); done

local-clean:
	rm -f *.o *~ core .depend tests/*.o

clean: local-clean
	for dir in $(EXTDIRS); do (cd $$dir && $(MAKE) clean); done

ifeq (.depend, $(wildcard .depend))
include .depend
endif

export

.PHONY: all install dist depend clean 
