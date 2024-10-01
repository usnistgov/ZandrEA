# These should all be inherited from the top level Makefile, but here are defaults just in case
# (Remember that := can be overridden by variables passed in)
HDF5_ROOT := $(CURDIR)/../HDF5
CXX := h5c++
CXXPATH := $(HDF5_ROOT)/bin/h5c++
PREFIX := ..
DEFS := -DPOSIX -DSTANDALONE

export PATH := $(HDF5_ROOT)/bin:$(PATH)

# You probably will need to adjust these definitions
DEFS=-DPOSIX -DSTANDALONE
#LIBNAME=ea
#LIBDIR=$(PREFIX)/lib
#EALIBFILE=$(LIBDIR)/lib$(LIBNAME).a
CXXOPTS=-Wno-comments

# Boost libraries needed
# (The actual link flags are generated below based on platform)
BASE_BOOST_LIBS := system chrono thread program_options filesystem iostreams atomic
BOOSTLIBPREFIX := boost_

# You may need to adjust these for your porting target
INCLUDES=-I$(PREFIX)/libEA -I$(PREFIX)/include -I$(HDF5_ROOT)/include -I/opt/homebrew/opt/openssl@1.1/include -I/opt/homebrew/include
LD=$(CXX)
ifeq ($(OS),Windows_NT)
  HOST_OS := Windows
  DEFS += -DWINDOWS
else
  HOST_OS := $(shell uname -s)
endif
ifeq ($(HOST_OS),Darwin)
  BOOSTLIBSUFFIX := -mt
  DEFS += -DOSX
endif
ifeq ($(HOST_OS),Linux)
  BOOSTLIBSUFFIX := 
  DEFS += -DLINUX
endif

BOOSTLIBS := $(patsubst %,-l$(BOOSTLIBPREFIX)%$(BOOSTLIBSUFFIX),$(BASE_BOOST_LIBS))
LIBEXCLPAT = #libmain.%
LIBSRCS =  $(filter-out $(LIBEXCLPAT), $(wildcard ../libEA/*.cpp))
LIBOBJS =  $(LIBSRCS:.cpp=.o)
LIBS += $(LIBOBJS) -L$(HDF5_ROOT)/lib -L$(PREFIX)/lib -L/opt/homebrew/opt/openssl@1.1/lib -L/opt/homebrew/lib -lcpprest -lssl -lcrypto $(BOOSTLIBS) -lstdc++ -lhdf5
CXXFLAGS += -g $(CXXOPTS) $(DEFS) $(INCLUDES)
#LDFLAGS=-g -Wl,-rpath=$$HOME/Git/ea_ConApp/dep/lib64 $(LIBS)
LDFLAGS += -g $(LIBS)

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

# These are the source and corresponding dep and object files we need to build
SRCS =	apiver.cpp handler.cpp main.cpp #session.cpp
OBJS =	$(SRCS:.cpp=.o)
DEPS =	$(SRCS:.cpp=.d)

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn

.PHONY:	all clean install dist-clean build compile recompile rebuild

TARGETS := ead

all:	install

compile:	$(CXXPATH) $(TARGETS)

build:	compile

install:	$(TARGETS)
	@mkdir $(PREFIX)/bin && chmod 0755 $(PREFIX)/bin
	@for f in $(TARGETS) ; do if test ! -f $(PREFIX)/bin/$$f -o $(PREFIX)/bin/$$f -ot $$f ; then echo "Installing $$f"; install -m 0555 -C $$f $(PREFIX)/bin/ ; fi ; done

reinstall:	clean install

recompile:	reinstall

rebuild:	reinstall

ead:	$(LIBOBJS) $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

$(LIBOBJS):
	$(MAKE) -C ../libEA

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPS)
endif

#This is the rule for creating the dependency files
%.d:	%.cpp
	@set -e; rm -f $@; \
		$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
#	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@

# How to build a .o file from a .cpp file
%.o:	%.cpp %.d Makefile
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) *.a *.so *.o *.d *.d.* *.h5 "60s Rule Kit.xml" "10s Rule Kit.xml"
	rm -rf *.dSYM
	$(MAKE) -C tests clean

dist-clean:	clean

test:	ead
	$(MAKE) -C tests
