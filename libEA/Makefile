#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# The C++ src code in [root]/libEA/ is built into a static library interacting at runtime with another
# part of the "ead" executable built from C++ src code in [root]/EAd/ by the Makefile in [root]/ea/EAd/.
# So, this Makefile (in [root]/libEA/) is called at its default target by the Makefile in [root]/EAd/.
# Contexts: In the "appbuilder" stage of Dockerfile, "[root]" is "/ea/". On host, it is your repo root.
# CURDIR is a GNU Make built-in set to the pwd existing in the context from which a Makefile is called.
# Given that call is only in the Dockerfile stage, context here is in that stage, not on your host.
#==================================================================================================C====5

HDF5_ROOT := $(CURDIR)/../HDF5
CXX := h5c++
CXXPATH := $(HDF5_ROOT)/bin/h5c++
# ** DAV - unsure why SWB did paths as relative using PREFIX (vs. absolutes)
PREFIX := ..
DEFS := -DPOSIX -DSTANDALONE

export PATH := $(HDF5_ROOT)/bin:$(PATH)

# You probably will need to adjust these definitions
LIBNAME=ea
STATICLIB=lib$(LIBNAME).a
#DYNAMICLIB=lib$(LIBNAME).so
CXXOPTS=-Wno-comments

# Provide variable listng all paths to files called by #include in /libEA/ src files
INCLUDES = -I$(HDF5_ROOT)/include -I/opt/homebrew/include

# Syntax to add many subdirs relative to pwd: $(addprefix -I, $(shell find [rel path] -type d))
# INCLUDES for major libraries typically path down ONLY to libraries top-level "/include/" directories.
# A "major" library has symlinks in "/include/" as necessary to obviate INCLUDE vars of deeper pathing.
# Use app's src file to define any deeper path needed (e.g., a header with #include <grpcpp/grpcpp.h>).
# Else, INCLUDE of deeper library dirs can shadow calls app makes to system libraries (e.g., <ctime>).

INCLUDES += -I$(PREFIX)/grpc/include -I$(PREFIX)/protobuf
#bad way was += $(addprefix -I,$(shell find $(PREFIX)/grpc/include -type d)) -I$(PREFIX)/protobuf

LD=		$(CXX)
LIBS=		#-L../dep/lib -L/opt/homebrew/lib -lhdf5 -lcpprest
CXXFLAGS +=	-g $(CXXOPTS) $(DEFS) $(INCLUDES)
#LDFLAGS=	-g -Wl,-rpath=$$HOME/Git/ea_ConApp/dep/lib64 $(LIBS)
LDFLAGS +=	-g #$(LIBS)
AR=ar
ARFLAGS=rcs
#DYNAMICLDFLAGS=-Wl,soname=$(DYNAMICLIB)

# NPROC= no. of cores on dev cpu building images (vs. target cpu running them) -> no. jobs by Make -j
ifeq ($(HDF5PLATFORM),Darwin)
export NPROC := $(shell sysctl -n hw.logicalcpu)
else
export NPROC := $(shell nproc)
endif
export JOBS := $(shell expr $(NPROC) + 2) 
#MAKEFLAGS += " -j$(JOBS) "

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# These are the source and the corresponding dependency and object files we need to build the static lib

EXCLPAT = #libmain.%	# Pattern to exclude files that are NOT libEA source files
# Scan pwd for files except those filtered and assign all to variable
SRCS =	$(filter-out $(EXCLPAT), $(wildcard *.cpp))
# Generate object .o filenames from src filenames. This is necessary for Make to build a dependency
# graph AHEAD of running the compilation build rule. Otherwise, every src file found gets recompiled
# Substi ref below is an alternative to doing it by OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OBJS =	$(SRCS:.cpp=.o)
DEPS =	$(SRCS:.cpp=.d)

# These define the "EA Runtime API" between the front and back ends of the "EA" part of ZandrEA
PUBLIC_HEADERS := exportTypes.hpp exportCalls.hpp

# We don't need to clean up when we're making these targets
NODEPS:=clean tags svn

.PHONY:	all objs clean build rebuild compile recompile install reinstall

.NOTPARALLEL:	build rebuild install reinstall

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# This Makefile (at /ea/libEA/ in "appbuilder" stage) is run when Makefile at /ea/EAd/ calls this target:
all:	build

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPS)
endif

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# THREE STEPS in building a static library from src code in /ea/libEA so it later can link into "ead" 

#==================================================================================================C====5
# (1) Generation of dependency (".d") files in order to individualize the deps of each src file.
# Each .d file accompanys its src file in the compiler rule's dep list so it recompiles only if revised.
# This intervening layer of .d target is effected by -M flag upon compiler (e.g., 2nd line of recipe). 
# Rule for creating dependecy (.d) files from source code in the ea/libEA directory:

%.d:	%.cpp $(CXXPATH)
	@set -e; rm -f $@; \
		$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
#	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@
#==================================================================================================C====5
# (2) Compile object .o files as needed based upon any changes among .hpp/.cpp or dependencies (.d)

%.o:	%.cpp %.d Makefile $(CXXPATH) $(PUBLIC_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

#==================================================================================================C====5
# (3) Run Archiver utility ("ar") to package EA's object .o files into a single static library (.a) file.
# Flag "rcs" invokes create/replace action and indexing of each .o within archive so compiler finds them.

$(STATICLIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $^

#==================================================================================================C====5
# These targets are in the chain invoking the primary purpose; building a static library from /libEA/  
objs:	$(OBJS)

# The call of default "all" target by the Makefile at /ea/EAd/ lands here:
build:
	$(MAKE) -j$(JOBS) objs

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# Further (collateral-purpose) targets in this ( /ea/libEA/ ) Makefile :

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
