#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# Typical ZandrEA dev-phase builds/startups begin with "make docker-up" at CLI with this file in the pwd.
# Per GNU Make behavior, that results first in a parse of this file followed by a jump to the rule at its
# target "docker-up" (below). That rule sends execution out of this file to Docker Compose, which finds
# docker-compose.yml directing "rest" service builds to the project root Dockerfile on host computer.
# That Dockefile copies this file from host to a "buildbase" stage to make a call using that copy.
# The parse of this file on the host was superfluous, but now its parse in the Dockerfile copy matters.
# The target called is "compiler", which builds the HDF5 compiler "wrapper" script "h5c++" into "/ea/".
# Subsequent calls are made on the stage's copy of this file, but using targets other than "docker-up".
# So, strictly speaking, this process is not recursive, because a completely separate copy is now open.
# However, the initiating "docker-up" target call in the instance of this file on the host computer
# cannot return the host process and its CLI to a user prompt until the recipe of "docker-up" returns
# from "docker compose --build --detach", meaning calls to Makefiles copied to Docker stages are exited.

# While the host process call to Docker Compose is still "out", it 

#==================================================================================================C====5
# CLI call to this file presumes HDF5 tarball of ver. specified and its h5c++.tmpl file are in the pwd.
# (SWB) Must define the following variables:
# DOCKER_REGISTRY
# DOCKER_PROJECT
# DOCKER_IMAGE_PREFIX
# DOCKER_IMAGE_SUFFIX
ENVFILE ?= production.env
-include ${ENVFILE}
DOCKER_REGISTRY ?= 
DOCKER_PROJECT ?= zandrea
DOCKER_IMAGE_PREFIX ?= 
DOCKER_IMAGE_SUFFIX ?= _prod


.PHONY:	all _all compile build build-ead rebuild recompile clean test docker-build docker-rerun docker-up docker-down docker-status docker-prune docker-rm-kb docker-retest docker-production-build docker-production-up docker-production-down docker-production-retest docker-production-save docker-production-push jscli pushtestdata install reinstall compiler dist-clean

# (SWB) I commented out .NOTPARALLEL because I discovered the .WAIT special target. (May be
# specific only to GNU make...?)  This gives better control over dependency processing than
# the .NOTPARALLEL target.
# NOTE2: the .WAIT special target was added in GNU make 4.4 which is too new to be in most
# distros! However I think WAIT is only being used in the top-level convenience functions
# (mostly to perform docker operations serially as needed) so as long as GNU make 4.4+ is
# the verison of make in your PATH on your build host, you should be ok.
# NOTE3: I'm going to make this behavior conditional, so if make is new enough it will take
# advantage of it to speed up the build.
NULL :=
SPACE := $(NULL) $(NULL)
MAKE_VERSION_SPLIT := $(patsubst .,$(SPACE),$(MAKE_VERSION))
MAKE_VERSION_MAJOR := $(word 1,$(MAKE_VERSION_SPLIT))
MAKE_VERSION_MINOR := $(word 2,$(MAKE_VERSION_SPLIT))
ifeq ($(filter shell-export,$(value .FEATURES)),)
$(info ####### $(MAKE) $(MAKE_VERSION_MAJOR).$(MAKE_VERSION_MINOR) does not support the .WAIT target - applying workaround)
$(info ####### Upgrade to GNU make 4.4+ to get better parallel build performance)
.NOTPARALLEL:	docker-clean docker-up docker-down docker-retest docker-rerun docker-restart docker-production-save docker-production-push test
.PHONY:		.WAIT
.WAIT: ;
endif

#MAKEFLAGS += --no-print-directory

#==================================================================================================C====5
# This (root) Makefile enables export on assignments also needed by sub-Makefile at ./libEA/
export PREFIX := $(CURDIR)

# This is because Dan's Windows WSL environment keeps getting timeouts
export COMPOSE_HTTP_TIMEOUT = 1000

JSCLI := eajscli	# or EAjsClient

# The use of a variable defining a list might expect a "dependency order".
# Convention is that dependency grows going to the right in the list.
SUBDIRS := libEA EAd eajscli		# NOTE: these MUST be in dependency order
LIBEA_SUBDIRS := libEA				# NOTE: these MUST be in dependency order

##############################################################################
# FEATURE FLAGS - enable at your own risk
# Set to 1 to enable, 0 to disable
##############################################################################
USE_SSL := 0

FEATURE_FLAGS := USE_SSL

# (SWB) Make syntax makes this difficult, but this just builds a list of compiler
# define flags (-Dflag=1) for all the feature flags above that have a value of 1.
CXX_FEATURE_FLAGS :=
define FFtemplate = 
ifeq ($$($(1)),1)
CXX_FEATURE_FLAGS += -D$(1)=1
endif
endef
$(foreach ff,$(FEATURE_FLAGS),$(eval $(call FFtemplate,$(flag))))

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# Define variables to configure the GNU Autotools implementation HDF5 uses to build its compiler:
# Call "shell" to run "uname -s" so it returns "Linux" or "Darwin" (MacOS)
HDF5PLATFORM := $(shell uname -s)
# Explicitly define version
HDF5VER := 1.14.4-2
HDF5SRCDIR := hdf5-$(HDF5VER)
HDF5SRCTGZ := $(HDF5SRCDIR).tar.gz
HDF5INSTALLDIR := HDF5
#==================================================================================================C====5
# IMPORTANT: general C++ compiler CXX set to be the HDF5CXX compiler built from the HDF5 Autotools pkg.
# Project files that #include HDF5 files require HDF5CXX, so for simplicity make its use universal. 
export HDF5CXX := $(HDF5INSTALLDIR)/bin/h5c++
export ABSHDF5CXX := $(CURDIR)/$(HDF5INSTALLDIR)/bin/h5c++
export CXX := $(ABSHDF5CXX)
#==================================================================================================C====5

ifeq ($(HDF5PLATFORM),Darwin)
export NPROC := $(shell sysctl -n hw.logicalcpu)
else
export NPROC := $(shell nproc)
endif
export JOBS := $(shell expr $(NPROC) + 2) 
MAKEFLAGS += --jobs=$(JOBS)

export PATH := $(CURDIR)/HDF5/bin:$(PATH)

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

# Conventional GNU name for the topmost target (default goal):
all: docker-build

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# Each call to "docker compose [etc.]" in recipes below cause a jump (but not exit) from this file to
# docker-compose.yml, which in turn may call a separate copy of this Makefile using a different target
# (e.g., build-ead). "In turn" means per the "services" dependencies defined in the docker-compose.yml.

docker-build:
	docker compose build

docker-up:
	docker compose up --build --detach

docker-down:
	-docker compose down

docker-rerun:	docker-down .WAIT docker-up

docker-restart:	docker-down .WAIT docker-up

docker-status:
	docker compose ps

docker-prune:
	-docker system prune -f
	-docker volume prune -f

docker-rm-kb:
	-/bin/rm -f data/*.h5
	-docker volume rm $(DOCKER_PROJECT)_rest-kb

docker-retest:	docker-down .WAIT docker-rm-kb .WAIT docker-up .WAIT pushtestdata

docker-clean:	docker-down .WAIT docker-rm-kb
	-/bin/rm -f production-images.tar.gz
	@echo "\nTo remove all unused docker resources run:\n\n\tmake docker-prune\n"
	@echo "WARNING: pruning will remove ALL docker resources, not just this project!"

docker-production-build docker-production-save production-images.tar.gz:	Dockerfile.production docker-compose.production.yml Makefile $(EAD_SRCS) $(LIBEA_SRCS) eajscli/Dockerfile.production eajscli/nginx/default.conf $(JSCLI_DEPS)
	docker compose --env-file=${ENVFILE} -f docker-compose.production.yml build #--no-cache --pull
	docker save ${DOCKER_REGISTRY}${DOCKER_PROJECT}/${DOCKER_IMAGE_PREFIX}ead${DOCKER_IMAGE_SUFFIX} ${DOCKER_REGISTRY}${DOCKER_PROJECT}/${DOCKER_IMAGE_PREFIX}webapp${DOCKER_IMAGE_SUFFIX} ${DOCKER_REGISTRY}${DOCKER_PROJECT}/${DOCKER_IMAGE_PREFIX}bacnet${DOCKER_IMAGE_SUFFIX} traefik | gzip -v > production-images.tar.gz

docker-production-push:	production-images.tar.gz
	docker compose --env-file=${ENVFILE} -f docker-compose.production.yml push

docker-production-up:	production-images.tar.gz
	docker compose --env-file=${ENVFILE} -f docker-compose.production.yml up --detach

docker-production-down:
	docker compose --env-file=${ENVFILE} -f docker-compose.production.yml down

docker-production-retest:	docker-production-down .WAIT docker-production-up .WAIT pushtestdata

#==================================================================================================C====5
# Phony target expressing dependency HDF5 compilation has upon HDF5CXX being present;
# Given the HDF5CXX assignment above, that means "Is /HDF5/bin/h5C++ built from template and available?"
compiler: $(HDF5CXX)

# Is the target (a directory) absent, OR is the HDF5 tarball newer than it? [error if tarball missing]
# If yes, whack any dir present and extract HDF5 src code from tarball (which creates a new target dir)  
$(HDF5SRCDIR): $(HDF5SRCTGZ)
	rm -rf $@ && tar xzf $<
# HDF5 tarball extract auto-creates a new HDF5SRCDIR (src dir) but not the final "installation" dirs
$(HDF5INSTALLDIR):
	mkdir -p $@/bin $@/lib $@/include
#==================================================================================================C====5

# On Mac build with: `./configure --enable-cxx --disable-shared --enable-static --enable-symbols=yes --prefix=$HOME/Git/ea_ConApp/dep CC=gcc CXX=g++ CXXFLAGS="-std=c++14"`
# On Linux build with: `./configure --enable-cxx --with-gnu-ld --disable-shared --enable-static --enable-symbols=yes --prefix=/Users/steveb/Git/ea_ConApp/dep CC=gcc CXX=g++`
#ifneq ($(HOMEBREW_PREFIX),)
#  # Homebrew is installed so we'll use the newest GCC there
#  NATIVE_CC := $(lastword $(sort $(wildcard $(HOMEBREW_PREFIX)/bin/gcc-[0-9] $(HOMEBREW_PREFIX)/bin/gcc-[0-9][0-9])))
#  NATIVE_CXX := $(lastword $(sort $(wildcard $(HOMEBREW_PREFIX)/bin/g++-[0-9] $(HOMEBREW_PREFIX)/bin/g++-[0-9][0-9])))
#  $(info Found newest homebrew CC = $(NATIVE_CC)   CXX = $(NATIVE_CXX))
#else
  NATIVE_CC := gcc
  NATIVE_CXX := g++
  $(info Fell back to system CC = $(NATIVE_CC)   CXX = $(NATIVE_CXX))
#endif

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# FIRST: Overview of what is ahead:
# Upon jump to root Dockerfile "builder" stage: system (apt) install of dependencies (incl. some Boost).
# Dockerfile: COPY to builder the HDF5 .tar w/ its embedded kit of GNU Autotools and h5c++.tmpl template.
# Dockerfile: COPY this file and call "compile" target => HDF5 compiled per a h5c++ "wrapper" (script).
# The h5c++ script is generated locally from the h5c++.tmpl downloaded as part of the HDF5 package

#==================================================================================================C====5
# HDF5CONFIGURE scripts the Autotools method "configure" and flags so it works next to a call to "h5c++"

ifeq ($(HDF5PLATFORM),Darwin)
HDF5CONFIGURE := ./configure --enable-threadsafe --disable-hl --disable-shared --enable-static --enable-symbols=yes --prefix=$(CURDIR)/$(HDF5INSTALLDIR) CC=$(NATIVE_CC) CXX=$(NATIVE_CXX) CXXFLAGS="-std=c++14"
export MAKE := gmake
else
HDF5CONFIGURE := ./configure --enable-threadsafe --disable-hl --with-gnu-ld --enable-shared --enable-static --enable-symbols=yes --prefix=$(CURDIR)/$(HDF5INSTALLDIR) CC=$(NATIVE_CC) CXX=$(NATIVE_CXX) CXXFLAGS="-std=c++14 -pthread" #LIBS=-lpthread
endif


# Before compile/link of any HDF5-included src file, need a rule to build HDF5CXX, an executable "h5c++"
# compiler ("wrapper" script) based on ".tmpl" template packaged in the HDF5 Autotools mplementation.
# $(MAKE) is a GNU Make built-in for calling "make" by the exact path as the current call, without
# Autotools would recur to the current Makefile as it is most local. Here, HDF5CONFIGURE instead
# causes an Autotools-generated Makefile in the HDF5INSTALLDIR to be called to build the h5c++ compiler:

$(HDF5CXX):	$(HDF5INSTALLDIR) $(HDF5SRCDIR) $(HDF5SRCTGZ) h5c++.tmpl
	(cd $(HDF5SRCDIR) && $(HDF5CONFIGURE) && $(MAKE) -j$(JOBS) install)
	rm -f $@ && sed -e s/BASEDIR/$(HDF5INSTALLDIR)/g < h5c++.tmpl > $@ && chmod 0555 $@

#==================================================================================================C====5
# NEXT:
# Dockerfile: gRPC built from src files by install and call of separate build system (CMake)
# Object ".o" files from both are linked to objs from /libEA/ src code using Make rules below.
#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5

DEFS	:= -DPOSIX -DSTANDALONE
CXXOPTS	=  -Wno-comments

# Boost libraries needed
# (The actual link flags are generated below based on platform)
BASE_BOOST_LIBS = system chrono thread program_options filesystem iostreams atomic
BOOSTLIBPREFIX = boost_

# We now only use Homebrew on Mac targets
HB_INCLUDES := 
HB_LDFLAGS :=

# You may need to adjust these for your porting target
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
  ifneq ($(HOMEBREW_PREFIX),)
  HB_INCLUDES := -I$(HOMEBREW_PREFIX)/opt/openssl@1.1/include -I$(HOMEBREW_PREFIX)/include
  #HB_INCLUDES := -I$(HOMEBREW_PREFIX)/include
  HB_LDFLAGS := -L$(HOMEBREW_PREFIX)/opt/openssl@1.1/lib -L$(HOMEBREW_PREFIX)/lib
  endif
endif
# First (given a Linux OS), clear BOOSTLIBSUFFIX
ifeq ($(HOST_OS),Linux)
  BOOSTLIBSUFFIX := 
  DEFS += -DLINUX
  ifneq ($(HOMEBREW_PREFIX),)
    exppath := $(subst :, ,$(PATH))
    ifneq (0,$(words $(filter $(HOMEBREW_PREFIX)%,$(exppath))))
      $(warning Warning: You have Homebrew in your PATH on $(HOST_OS) - suggest removing before continuing)
      #noop :=
      #space := $(noop) $(noop)
      #PATH := $(subst $(space),:,$(filter-out $(HOMEBREW_PREFIX)%,$(exppath)))
      #$(warning Updated PATH to $(PATH))
      # This fails because if you were running a homebrew make command, it drops it from the path
    endif
  endif
endif

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# Variables to organize information needed in the rules generating the executable [ $(EAD_EXES) target ]:
#==================================================================================================C====5
# Paths to the directories holding "#include" files. These are needed by the dependency rule**
# [ ** or the compilation rule had it not been preceded by a dependency rule]
# Up to the linker rule it is irrelevant whether these will be statically or dynamically linked. 

INCLUDES=-I$(PREFIX)/libEA -I$(PREFIX)/include -I$(HDF5INSTALLDIR)/include $(HB_INCLUDES)
# This is also 
INCLUDES += $(addprefix -I,$(shell find $(PREFIX)/grpc/include -type d))

#==================================================================================================C====5
# Passing this in linker rule tells it to dynamically (-l) link Boost library
BOOSTLIBS := $(patsubst %,-l$(BOOSTLIBPREFIX)%$(BOOSTLIBSUFFIX),$(BASE_BOOST_LIBS))

#==================================================================================================C====5
# Paths and filenames are assigned here on the basis that the root Dockerfile and the calls its
# stages make on their copy of this file and its sub-Makefiles will together create and populate
# these paths and filenames before the $(EAD_EXES) target needing them is hit and resolved.
# [That is, none of these variables are exported out to be populated elsewhere]:

LIBEA_EXCLPAT = libEA/EAwin32DLL.% # libEA/dllmain.%
LIBEA_SRCS := $(filter-out $(LIBEA_EXCLPAT), $(wildcard libEA/*.cpp))
LIBEA_OBJS := $(LIBEA_SRCS:.cpp=.o)
LIBEA_DEPS := $(LIBEA_SRCS:.cpp=.d)
LIBEA_EXES :=
LIBEA_PUBLIC_HEADERS := libEA/exportTypes.hpp libEA/exportCalls.hpp

EAD_EXCLPAT = EAd/session.%
EAD_SRCS := $(filter-out $(EAD_EXCLPAT), $(wildcard EAd/*.cpp))
EAD_OBJS := $(EAD_SRCS:.cpp=.o)
EAD_DEPS := $(EAD_SRCS:.cpp=.d)



#EAD_LIBS := -L$(HDF5INSTALLDIR)/lib -L./lib -L$(HOMEBREW_PREFIX)/opt/openssl@1.1/lib -L$(HOMEBREW_PREFIX)/lib -lcpprest $(BOOSTLIBS) -lhdf5 -lssl -lcrypto -lstdc++
EAD_LIBS := -L$(HDF5INSTALLDIR)/lib -L./lib $(HB_LDFLAGS) -lcpprest $(BOOSTLIBS) -lhdf5 -lsz -lssl -lcrypto -lstdc++
EAD_EXES := bin/ead
EAD_PUBLIC_HEADERS := 

#==================================================================================================C====5
# [DAV] Integration of gRPC



#==================================================================================================C====5
# $$$$ DAV Find out what these are for

SRCS = $(LIBEA_SRCS) $(EAD_SRCS)
OBJS = $(LIBEA_OBJS) $(EAD_OBJS)
DEPS = $(LIBEA_DEPS) $(EAD_DEPS)
EXES = $(LIBEA_OBJS) $(EAD_EXES)

##################################################################################################

JSCLI_SRCS := $(wildcard eajscli/src/*.js eajscli/src/*.css eajscli/public/* eajscli/src/Components/*.js)
JSCLI_DEPS := $(JSCLI_SRCS) eajscli/package.json

##################################################################################################

MASTER_PUBLIC_HEADERS := $(LIBEA_PUBLIC_HEADERS) $(EAD_PUBLIC_HEADERS)
PUBLIC_HEADERS := $(addprefix include/ea/,$(notdir $(MASTER_PUBLIC_HEADERS)))
$(PUBLIC_HEADERS): include/ea $(MASTER_PUBLIC_HEADERS)
	cp $(MASTER_PUBLIC_HEADERS) include/ea
	(cd include && ln -sf ea/* ./)

#==================================================================================================C====5
# NOT Make "include" directive, but instead rules creating ./include/ea and ./bin if not present
include/ea:
	-mkdir -p include/ea

bin:
	-mkdir bin

#==================================================================================================C====5
# Extending these flags

CXXFLAGS +=     -g -O $(CXX_FEATURE_FLAGS) $(CXXOPTS) $(DEFS) $(INCLUDES)
LDFLAGS +=      -g

##################################################################################################

#Don't create dependencies when we're cleaning, for instance
NODEPS=docker-down docker-up docker-run docker-rerun docker-build docker-restart docker-status docker-prune docker-rm-kb docker-retest docker-clean docker-production-build docker-production-up docker-production-down docker-production-retest docker-production-save docker-production-push clean dist-clean tags svn pushtestdata
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPS)
endif

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# THREE STEPS in building "ead" (the executable running container "ea-rest") from its C++ src code:
# [Since these are run by a copy of this file located in a Dockerfile stage, the cluttering up of its pwd
# with intermediate results (e.g., %.d pattern generations) doesn't matter once that stage is superceded]

#==================================================================================================C====5
# (1) Generation of dependency (".d") files in order to individualize the deps of each src file.
# Each .d file accompanys its src file in the compiler rule's dep list so it recompiles only if revised.
# This intervening layer of .d target is effected by -M flag upon compiler (e.g., 2nd line of recipe). 
# Rule for creating dependecy (.d) files from source code in the root directory (i.e., HDF5 src files):

%.d: %.cpp $(HDF5CXX)
	@set -e; rm -f $@; \
		$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
#       $(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@

#==================================================================================================C====5
# (2) Call compiler again to generate obj files from C++ src files after checks upon dependencies.
# [Note: Step (1) rolled the includes .cpp have on project headers (.hpp) into the dependencies (%.d)]

%.o: %.cpp %.d Makefile $(HDF5CXX) $(PUBLIC_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

#==================================================================================================C====5
# (3) Link rule; since $(EAD_EXES)="/ea/bin/ead", result goes where root Dockerfile starts up "ea-rest": 
 
$(EAD_EXES): Makefile bin include/ea $(HDF5CXX) $(PUBLIC_HEADERS) $(LIBEA_OBJS) $(EAD_OBJS)
	$(CXX) $(LDFLAGS) $(LIBEA_OBJS) $(EAD_OBJS) -o $@ $(EAD_LIBS)
	-chmod 755 $@


#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
# General targets not pointing to Docker Compose nor other Docker componentry:

all: ; @$(MAKE) _all

_all:	compiler $(EXES)

build-ead: $(EAD_EXES)

test:	$(EXES)
	for c in bin/desktopTestTheDll_IowaVAV_Interact_FeaturesAndMore ; do /bin/rm -f *.h5; $$c || exit 1; /bin/rm -f *.h5; done
	$(MAKE) -C EAd/tests test

clean:
	/bin/rm -f *.h5 data/*.h5 "60s Rule Kit.xml" "10s Rule Kit.xml" ; \
	/bin/rm -rf build ; \
	/bin/rm -f $(OBJS) $(DEPS) $(EXES); \
	for d in $(SUBDIRS); do \
	  test -f $$d/Makefile && $(MAKE) -C $$d clean ; \
	done

compile:	$(EXES)

install:	$(EXES)

reinstall:	clean .WAIT install

recompile:	reinstall

#==================================================================================================C====5
# This target switches to a local directory (so "./" prefix not req'd as it is when directly pathing an
# executable) in order to start a Node.js app (app.js ?)
jscli:
	(cd eajscli && npm start)

pushtestdata:
	(cd EAd/tests && sleep 5 && $(MAKE) pushtestdata)

rebuild:	reinstall 

dist-clean:	clean
	rm -rf $(HDF5INSTALLDIR) $(HDF5SRCDIR) bin lib include
