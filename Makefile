# These should come from environment:
# CXX
# CXXFLAGS

# TMV_DIR
# FFTW_DIR
# YAML_DIR
# MKL_DIR (optionally)

INCLUDES := 

LIBS := -lm

# Here are (most of) the object libraries we can compile for external use:
OBJ := BinomFact.o StringStuff.o Poisson.o Table.o Pset.o odeint.o \
	Interpolant.o Expressions.o Shear.o 

# Collect the includes and libraries we need
ifdef FFTW_DIR
INCLUDES += -I $(FFTW_DIR)/include
LIBS += -L $(FFTW_DIR)/lib -lfftw3
OBJ += fft.o 
else
$(info WARNING: No FFTW_DIR in environment, skipping fft.cpp compilation)
endif

ifdef YAML_DIR
INCLUDES += -I $(YAML_DIR)/include
LIBS += -L $(YAML_DIR)/lib -lyaml-cpp
OBJ += Poly2d.o Lookup1d.o
else
$(info WARNING: No YAML_DIR in environment, skipping Poly2d.cpp, Lookup1d.cpp compilation)
endif

ifdef TMV_DIR
INCLUDES += -I $(TMV_DIR)/include -D USE_TMV
LIBS += $(shell cat $(TMV_DIR)/share/tmv/tmv-link) -ltmv_symband 
endif

ifdef EIGEN_DIR
INCLUDES += -I $(EIGEN_DIR) -D USE_EIGEN
endif

# Check that either TMV or EIGEN are available (ok to have both)
$(if $(or $(TMV_DIR),$(EIGEN_DIR)), , $(error Need either TMV_DIR or EIGEN_DIR))

ifdef MKL_DIR
INCLUDES += -I $(MKL_DIR)/include -D USE_MKL
endif

# Rule for compilation:
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# External directories where we'll need to clean/build dependents
EXTDIRS = 

SRC = $(OBJ:%.o=%.cpp)

all: $(OBJ)

test_lookup: test_lookup.o $(OBJ) 
	$(CXX) $(CXXFLAGS) $^  $(LIBS) -o $@

tests/testMarquardt.o: tests/testMarquardt.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I . -c $< -o $@

tests/testMarquardt: tests/testMarquardt.o 
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^  $(LIBS) -o $@

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
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $(SRC) > .depend

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
