#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# ZandrEA repo root directory multi-stage Dockerfile: builds image that runs as the "ea-rest" container.
# Each FROM starts a "stage"; each COPY, ADD, and RUN caches a "layer" credited to rebuilds if unedited.
# "Heavy" libraries are built and layer-cached by two "builder" stages, but any edit to this file or to
# any Makefile (since calls by both builders lead to them) invokes rebuild of ALL libraries (~20 mins).
# Edits made only to src code in /libEA/ or /EAd/ will skip to faster rebuilds ( ~seconds)
#==================================================================================================C====5

FROM ubuntu:24.04 AS baseos
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
# Bare minimum of libraries needed to run the ZandrEA executable ("ead")
# build-essential in baseos if want to debug C and C++ src via VS Code remoting to REST (dev) container
# Ubuntu 24 "apt" defaults build-essential g++ to C++17, which is the minimum C++ for gRPC while still
# okay for (the relativley old) HDF5 v. 1.14 (vv. check HDF5 tarball copied to "buildbase" stage below)
# Default "user" upon any "RUN" etc. is "root" unless another is specified by a "USER" command
RUN apt update && DEBIAN_FRONTEND="noninteractive" apt upgrade -y && DEBIAN_FRONTEND="noninteractive" apt install -y \
    libboost-chrono-dev \
    libboost-date-time-dev \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-iostreams-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libcereal-dev \
    libcpprest-dev \
    libstdc++-13-dev \
    libsz2 \
    build-essential \
    gdb
#  && rm -rf /var/lib/apt/lists/*

FROM baseos AS buildbase
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
# "ARG var" creates build-time argument "var" sent to "docker build" by flag:"--build-arg <var>=<value>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
 
RUN apt update && DEBIAN_FRONTEND="noninteractive" apt install -y \
    libaec-dev \
    nodejs \
  && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /ea/include/ea /ea/lib/ea /ea/bin /data
ENV PATH=$PKGROOT/HDF5/bin:$PATH
WORKDIR $PKGROOT
COPY hdf5-1.14.4-2.tar.gz h5c++.tmpl Makefile ./
RUN make compiler

#==================================================================================================C====5
# DAV - Install latest CMake from Kitware repo (v. 4.2.3 on 260218); apt default CMake too old for gRPC.
# [Steps as copied from apt.kitware.com on 260218 for Ubuntu 24.04 (Noble Numbat)]
RUN apt-get update && apt-get install -y ca-certificates gpg wget lsb-release
# If FALSE on kitware-archive-keyring package, manually obtain copy of signing key
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || \
      wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
      gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
# Add Kitware repo to sources list of apt and update
# [Use lsb-release to confess the OS release present instead of hardcoding ".../ubuntu/ noble main"]
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | \
      tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
      apt-get update
# If FALSE on kitware-archive-keyring pkg, remove manually obtained signed key to make room for package
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || \
      rm /usr/share/keyrings/kitware-archive-keyring.gpg
# Install kitware-archive-keyring pkg to ensure local keyring stays up to date as Kitware rotates them
RUN apt-get install -y kitware-archive-keyring
# Install cmake (apt deflects to Kitware), do not remove pkg lists (no "rm -rf /var/lib/apt/lists/*") 
RUN apt-get install -y cmake
# DAV - END - install of CMake

FROM buildbase AS appbuilder
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
ENV PATH=$PKGROOT/HDF5/bin:$PATH
WORKDIR $PKGROOT
#==================================================================================================C====5
# DAV - BEGIN - gRPC C++ install, per site: https://grpc.io/docs/languages/cpp/quickstart/ on 260218
# Install gRPC dependencies while at $PKGROOT
RUN apt install -y autoconf libtool pkg-config libsystemd-dev
RUN apt-get update && apt-get install -y git
# Create directory for gRPC src code and switch pwd to it
WORKDIR $PKGROOT/grpc/grpc-src/
# Clone gRPC src code into current directory ("." at end)
RUN git clone --recurse-submodules -b v1.78.0 --depth 1 --shallow-submodules \
      https://github.com/grpc/grpc .
# Create and switch to a directory dedicated to an out-of-source build
WORKDIR $PKGROOT/grpc/build/
# Call cmake w/o the build flag to have it read gRPC's CMakeLists.txt file and config a happy build env.
# Note flagging of C++ version, 17 is minimum yet needs to be binary compatible w/ any other library
# Note flag to build shared (dynamic) libs (.so files) versus default (vv. CMake) static (.a files)
RUN cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX=$PKGROOT/grpc \
      ../grpc-src
# Call the focused Makefile that CMake generated to build/install gRPC per the happy build environment
RUN make -j 4 && make install
# Copy dir from host holding Protobuf .proto file and C++ and Python stub codes Protoc compiled off-line
# [Location on host (first argument of COPY) is relative to directory holding this Dockerfile]
WORKDIR $PKGROOT
COPY ./protobuf/  $PKGROOT/protobuf/
# PATH holds only paths to executables; paths to includes must be flagged (i.e., -I) within Make recipes
ENV PATH=$PATH:$PKGROOT/grpc/bin:$PKGROOT/protobuf
# DAV - END - gRPC (C++ side) install
#==================================================================================================C====5
# Build "ead" executable
WORKDIR $PKGROOT
COPY libEA ./libEA/
COPY EAd ./EAd/
# Call stage's copy of root Makefile; "build-ead" is phony label of rule building executable /ea/bin/ead
RUN make build-ead

FROM baseos
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
RUN mkdir -p /data /ea/bin /ea/include /ea/lib /ea/EAd /ea/libEA /ea/ \
  /ea/grpc/bin /ea/grpc/include /ea/grpc/lib .vscode \
  && rm -rf /var/lib/apt/lists/*
COPY --from=appbuilder /ea/grpc/bin/ /ea/grpc/bin/
COPY --from=appbuilder /ea/grpc/include/ /ea/grpc/include/
COPY --from=appbuilder /ea/grpc/lib/ /ea/grpc/lib/
COPY --from=appbuilder /ea/protobuf/ /ea/protobuf/
COPY .gdbinit ./ea/.gdbinit
COPY --from=appbuilder /ea/include/ /ea/include/
COPY --from=appbuilder /ea/lib/ /ea/lib/
COPY --from=appbuilder /ea/bin/ /ea/bin/
COPY .vscode ./.vscode/

WORKDIR /ea
RUN ln -s bin/ead /ea/
WORKDIR /data
ENTRYPOINT ["/ea/bin/ead"]
CMD ["-b", "http://0.0.0.0", "-p", "9876"]
VOLUME ["/data"]
EXPOSE 9876/tcp
