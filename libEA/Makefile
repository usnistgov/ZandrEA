# These should all be inherited from the top level Makefile, but here are defaults just in case
# (Remember that := can be overridden by variables passed in)
HDF5_ROOT := $(CURDIR)/../HDF5
CXX := h5c++
CXXPATH := $(HDF5_ROOT)/bin/h5c++
PREFIX := ..
DEFS := -DPOSIX -DSTANDALONE

export PATH := $(HDF5_ROOT)/bin:$(PATH)

# You probably will need to adjust these definitions
LIBNAME=ea
STATICLIB=lib$(LIBNAME).a
#DYNAMICLIB=lib$(LIBNAME).so
CXXOPTS=-Wno-comments

# You may need to adjust these for your porting target
INCLUDES=-I$(HDF5_ROOT)/include -I/opt/homebrew/include
LD=		$(CXX)
LIBS=		#-L../dep/lib -L/opt/homebrew/lib -lhdf5 -lcpprest
CXXFLAGS +=	-g $(CXXOPTS) $(DEFS) $(INCLUDES)
#LDFLAGS=	-g -Wl,-rpath=$$HOME/Git/ea_ConApp/dep/lib64 $(LIBS)
LDFLAGS +=	-g #$(LIBS)
AR=ar
ARFLAGS=rcs
#DYNAMICLDFLAGS=-Wl,soname=$(DYNAMICLIB)

# How many jobs to run in parallel...
ifeq ($(HDF5PLATFORM),Darwin)
export NPROC := $(shell sysctl -n hw.logicalcpu)
else
export NPROC := $(shell nproc)
endif
export JOBS := $(shell expr $(NPROC) + 2) 
#MAKEFLAGS += " -j$(JOBS) "

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

# These are the source and corresponding dep and object files we need to build
EXCLPAT = #libmain.%	# Pattern to exclude files that are NOT libEA source files
SRCS =	$(filter-out $(EXCLPAT), $(wildcard *.cpp))	# match all *.cpp files except files that match EXCLPAT
OBJS =	$(SRCS:.cpp=.o)
DEPS =	$(SRCS:.cpp=.d)

PUBLIC_HEADERS := exportTypes.hpp exportCalls.hpp

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn

.PHONY:	all objs clean build rebuild compile recompile install reinstall

.NOTPARALLEL:	build rebuild install reinstall

all:	build

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPS)
endif

#This is the rule for creating the dependency files
%.d:	%.cpp $(CXXPATH)
	@set -e; rm -f $@; \
		$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
#	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@

# How to build a .o file from a .cpp file
%.o:	%.cpp %.d Makefile $(CXXPATH) $(PUBLIC_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(STATICLIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# This is our executable
#ea:	$(OBJS)
#	$(CXX) $(LDFLAGS) $(OBJS) -o $@

objs:	$(OBJS)

build:
	$(MAKE) -j$(JOBS) objs

compile:	build

rebuild:	clean build

recompile:	rebuild

reinstall:	clean install

clean:
	rm -f ea *.a *.so *.o *.d *.d.* *.h5 "60s Rule Kit.xml" "10s Rule Kit.xml"
	rm -rf ea.dSYM

$(CXXPATH):
	@echo "ERROR: HDF5 compiler wrapper not found ($(CXXPATH))"
	@exit 1

install:	build $(STATICLIB)
	@if test ! -f $(PREFIX)/lib/ea/$(STATICLIB) -o $(STATICLIB) -nt $(PREFIX)/lib/ea/$(STATICLIB) ; then \
		echo "Installing into prefix $(PREFIX)" ; \
		install -m 0444 -C $(PUBLIC_HEADERS) $(PREFIX)/include/ea/ ; \
		install -m 0444 -C $(STATICLIB) $(PREFIX)/lib/ea/ ; \
		(cd $(PREFIX)/include && for f in $(PUBLIC_HEADERS); do test -L $$f || ln -sf ea/$$f ./; done) ; \
		(cd $(PREFIX)/lib && for f in $(STATICLIB); do test -L $$f || ln -sf ea/$$f ./; done) ; \
	fi

test:
	@echo "No tests implmented yet"
