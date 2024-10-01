# Require this file to be present. It must define the following variables:
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

# NOTE: I commented out .NOTPARALLEL because I discovered the .WAIT special target. (May be
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

export PREFIX := $(CURDIR)

# This is because Dan's Windows WSL environment keeps getting timeouts
export COMPOSE_HTTP_TIMEOUT = 1000

JSCLI := eajscli	# or EAjsClient

SUBDIRS := libEA EAd eajscli		# NOTE: these MUST be in dependency order
LIBEA_SUBDIRS := libEA				# NOTE: these MUST be in dependency order

##############################################################################
# FEATURE FLAGS - enable at your own risk
# Set to 1 to enable, 0 to disable
##############################################################################
USE_SSL := 0

FEATURE_FLAGS := USE_SSL

# Make syntax makes this difficult, but this just builds a list of compiler
# define flags (-Dflag=1) for all the feature flags above that have a value of 1.
CXX_FEATURE_FLAGS :=
define FFtemplate = 
ifeq ($$($(1)),1)
CXX_FEATURE_FLAGS += -D$(1)=1
endif
endef
$(foreach ff,$(FEATURE_FLAGS),$(eval $(call FFtemplate,$(flag))))

##############################################################################
HDF5PLATFORM := $(shell uname -s)
HDF5VER := 1.14.4-2
HDF5SRCDIR := hdf5-$(HDF5VER)
HDF5SRCTGZ := $(HDF5SRCDIR).tar.gz
HDF5INSTALLDIR := HDF5
export HDF5CXX := $(HDF5INSTALLDIR)/bin/h5c++
export ABSHDF5CXX := $(CURDIR)/$(HDF5INSTALLDIR)/bin/h5c++
export CXX := $(ABSHDF5CXX)
##############################################################################

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

all:	docker-build

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

compiler:	$(HDF5CXX)

$(HDF5SRCDIR):	$(HDF5SRCTGZ)
	rm -rf $@ && tar xzf $<

$(HDF5INSTALLDIR):
	mkdir -p $@/bin $@/lib $@/include

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

ifeq ($(HDF5PLATFORM),Darwin)
HDF5CONFIGURE := ./configure --enable-threadsafe --disable-hl --disable-shared --enable-static --enable-symbols=yes --prefix=$(CURDIR)/$(HDF5INSTALLDIR) CC=$(NATIVE_CC) CXX=$(NATIVE_CXX) CXXFLAGS="-std=c++14"
export MAKE := gmake
else
HDF5CONFIGURE := ./configure --enable-threadsafe --disable-hl --with-gnu-ld --enable-shared --enable-static --enable-symbols=yes --prefix=$(CURDIR)/$(HDF5INSTALLDIR) CC=$(NATIVE_CC) CXX=$(NATIVE_CXX) CXXFLAGS="-std=c++14 -pthread" #LIBS=-lpthread
endif

$(HDF5CXX):	$(HDF5INSTALLDIR) $(HDF5SRCDIR) $(HDF5SRCTGZ) h5c++.tmpl
	(cd $(HDF5SRCDIR) && $(HDF5CONFIGURE) && $(MAKE) -j$(JOBS) install)
	rm -f $@ && sed -e s/BASEDIR/$(HDF5INSTALLDIR)/g < h5c++.tmpl > $@ && chmod 0555 $@


##################################################################################################

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
INCLUDES=-I$(PREFIX)/libEA -I$(PREFIX)/include -I$(HDF5INSTALLDIR)/include $(HB_INCLUDES)

BOOSTLIBS := $(patsubst %,-l$(BOOSTLIBPREFIX)%$(BOOSTLIBSUFFIX),$(BASE_BOOST_LIBS))

##################################################################################################

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
$(PUBLIC_HEADERS):	include/ea $(MASTER_PUBLIC_HEADERS)
	cp $(MASTER_PUBLIC_HEADERS) include/ea
	(cd include && ln -sf ea/* ./)

include/ea:
	-mkdir -p include/ea

bin:
	-mkdir bin

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

#This is the rule for creating the dependency files
%.d:    %.cpp $(HDF5CXX)
	@set -e; rm -f $@; \
		$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
#       $(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@

# How to build a .o file from a .cpp file
%.o:    %.cpp %.d Makefile $(HDF5CXX) $(PUBLIC_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<



##################################################################################################

$(EAD_EXES):	Makefile bin include/ea $(HDF5CXX) $(PUBLIC_HEADERS) $(LIBEA_OBJS) $(EAD_OBJS)
	$(CXX) $(LDFLAGS) $(LIBEA_OBJS) $(EAD_OBJS) -o $@ $(EAD_LIBS)
	-chmod 755 $@

##################################################################################################

all:	; @$(MAKE) _all

_all:	compiler $(EXES)

build-ead:	$(EAD_EXES)

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

jscli:
	(cd eajscli && npm start)

pushtestdata:
	(cd EAd/tests && sleep 5 && $(MAKE) pushtestdata)

rebuild:	reinstall 

dist-clean:	clean
	rm -rf $(HDF5INSTALLDIR) $(HDF5SRCDIR) bin lib include
