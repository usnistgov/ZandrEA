# Dependencies

These are the dependencies needed to build the standalone version. If you use Docker these are all taken care of automatically by docker-compose and Dockerfiles.

Supported platforms for non-Docker build:
- Linux (Ubuntu 20.04+)
- (deprecated) MacOS with XCode Command Line tools and Homebrew installed

Windows was supported in the past but support has been removed to simplify the build process.

If installing from packages e.g. HDF5, download the sources into `{repo}/dep/src` and extract and build them there with a prefix of `{repo}/dep` .  Then install them (so libraries go into `{repo}/dep/lib` etc.).

If you put them elsewhere be sure to update the Makefile.

You should install them in order as shown.

## Docker

Docker is the preferred build and deployment platform, because it provides a standard build across multiple types of hardware and operating systems. If you are using Docker, you just need to have Docker Workstation installed on your desktop for development/testing (on desktop workstations this may require a paid license), or docker packages installed on your Linux server. You will need the Docker `compose` plugin. You will also need a version of the `make` utility, preferably the GNU version. If you intend to use any of the test scripts provided then you will also need Python 3.x installed. 

NOTE: one potential issue with Docker is the target platform type. If your target server has a different CPU architecture than the machine you're using to build the images then'll want to look in to setting a platform type in the compose files and/or in environment variables. This has been a problem when using a Mac with Apple silicon (M-seriels processors) to build for an Intel x86-64 target, but ARM targets are also becoming popular. In general the easiest solution is to just build on the same CPU type as your target platform.

On a Linux system (desktop or server) on Ubuntu for example, it suffices to install the distro Docker packages:
```
apt install -y docker-ce docker-compose-plugin
```

## Ubuntu 24.04

This is the best-supported build environment because the Docker build uses Ubuntu base images. Looking at the root Dockerfile shows exactly what needs to be installed to create a build system.

Recent Ubuntu distros have new-enough packages that use of Homebrew for Linux is no longer recommended.

## MacOS X - Intel CPUs
## MacOS X - Apple M1 and M2 CPUs

If you will be developing for Docker then it is sufficient to download and install Docker Desktop from docker.com. Note that you may need to purchase a license. Once you install docker then you can simply open a Terminal window and use the docker-(target) Make targets to build, start, and stop the docker containers. Internally the containers run Linux and will build following the recipe above with no other host system dependencies.

If you want to develop any of the code natively for ease of testing and debugging then you will need to install a few things.

The XCode Command Line Tools includes the CLang compiler which is essentially compatible with GCC/G++ and is sufficient for development.
```
xcode-select --install
```

Then install Homebrew (see https://brew.sh/). Once it's installed you can install some tools and libraries you'll need beyond the compiler:
```
brew install cpprestsdk cereal
```
...these will bring in other dependencies as needed.

*NOTE:* On the Apple Silicon machines (M1/M2/M3/....) this is a completely different CPU type which is not well tested yet, so running native builds or native Docker builds may be problematic. You can work around this in Docker by telling Docker to run emulated-CPU Intel containers. Do this by setting an environment varible anywhere you run Docker commands:
```
export DOCKER_DEFAULT_PLATFORM=linux/amd64
```

## Generic build instructions

(The notes below may or may not be up to date. I've left them here in case it helps, but don't count on them working exactly as written anymore. I haven't done any standalone builds (outside of Docker) for a year or two. Use at your own risk.)

### GCC

On Mac, I found that using the Xcode clang compiler with `-std=c++14` (with a few extra patches) was sufficient.

When building on Linux, you need a gcc version 7.x compiler.  On 14.04 I had to buld from source in the dep directory.  Then build, test, install gcc/g++:
```
mkdir build && cd build && ../configure --prefix=$HOME/Git/ea_ConApp/dep --enable-languages=c,c++ --disable-multilib CC=/usr/bin/gcc CXX=/usr/bin/g++ && make -j && make test && make install
```

### HDF5

Note that the other dependencies below and the EA project are built with the HDF5-provided "h5c++" command, so you need to install a properly configured HDF5 library before building EA.

Download and build the HDF5 library.  I was not able to use the default Homebrew install on Mac because it lacked the C++ interface, at least.

Download from:  https://www.hdfgroup.org/downloads/hdf5/source-code/

(Note: a copy of the hdf5-1.14.1 source is included in the repo - you shouldn't need to download it.)

On Mac build with:
```
./configure --enable-cxx --disable-shared --enable-static --enable-symbols=yes --prefix=$HOME/Git/ea_ConApp/dep CC=gcc CXX=g++ CXXFLAGS="-std=c++14"
```

On Linux build with:
```
./configure --enable-cxx --with-gnu-ld --disable-shared --enable-static --enable-symbols=yes --prefix=$HOME/Git/ea_ConApp/dep CC=gcc-7 CXX=g++-7
```

## cereal - C++ Object Serialization library

On Mac: `brew install cereal`

On Ubuntu 16.04: Either use linuxbrew as above, or: `apt-get install libcereal-dev`

## REST Libraries and Frameworks

### C++ REST SDK

On Mac: `brew install cpprestsdk`

On Linux: Use linuxbrew as above, or: `apt-get install libcpprest-dev`

CPPREST depends on Boost and OpenSSL.  I was able to get a build working with SSL enabled, but I had several issues with it:
* Boost::Asio (where SSL resides) is header-only, BUT is not yet compatible with OpenSSL 1.1 (as of Boost 1.67) (segfaults when a connection is opened)
* It looked like the overhead per request was fairly high (18ms http NOOP --> 64ms https NOOP)
* Tuning OpenSSL for secure config underneath CPPREST and Boost looked difficult
* Doing HTTPS at this level would make more sense if http/2 and/or persistent connections were supported.
* Long term plan is to put this behind an nginx proxy to do the https management and static resource service.

