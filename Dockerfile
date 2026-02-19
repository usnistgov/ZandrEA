FROM ubuntu:24.04 AS baseos
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
# Bare minimum of libraries needed to run the above executable
# Put build-essential into baseos when want to debug EA inside its dev container - DAV
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

# DAV - BEGIN - Install latest version CMake from Kitware repo (v. 4.2.3 on 260218)
# [Steps as copied from apt.kitware.com on 260218 for Ubuntu 24.04 (Noble Numbat)]
RUN apt-get update && apt-get install -y ca-certificates gpg wget lsb-release
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
# Use lsb-release to confess the OS release present instead of hardcoding ".../ubuntu/ noble main"
RUN echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || rm /usr/share/keyrings/kitware-archive-keyring.gpg
RUN apt-get install -y kitware-archive-keyring
RUN apt-get install -y cmake && rm -rf /var/lib/apt/lists/*
RUN cmake --version 
# DAV - END - install of CMake

FROM buildbase AS appbuilder
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
ENV PATH=$PKGROOT/HDF5/bin:$PATH
WORKDIR $PKGROOT
#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# Since gRPC is a new EA dependency (like HDF5), build and install it (via CMake) before building EA
# BEGIN - gRPC C++ install, mostly per site: https://grpc.io/docs/languages/cpp/quickstart/ on 260218
# Install gRPC dependencies while at $PKGROOT
RUN apt install -y autoconf libtool pkg-config libsystemd-dev
WORKDIR $PKGROOT/grpc/grpc-src/
# Clone gRPC src into dedicated dir 
RUN git clone --recurse-submodules -b v1.78.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc.git .
# Create and switch to a directory dedicated to an out-of-source build
WORKDIR $PKGROOT/grpc/build/
# Call cmake w/o the build flag to have it read gRPC's CMakeLists.txt file and config a build env
# then call Make to build and install (using the targets/instructions CMake wrote up for it)
RUN cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_INSTALL_PREFIX=$PKGROOT/grpc \
      ../grpc-src && \
      make -j 4 && \
      make install
ENV PATH=$PATH:$PKGROOT/grpc/bin:$PKGROOT/grpc/include:$PKGROOT/grpc/lib
# END - gRPC (C++ side) install
#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
COPY libEA ./libEA/
COPY EAd ./EAd/
RUN make build-ead

FROM baseos
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
RUN mkdir -p /data /ea/bin /ea/include /ea/lib /ea/EAd /ea/libEA /ea/.vscode \
  && rm -rf /var/lib/apt/lists/*
COPY .gdbinit ./ea/.gdbinit
COPY --from=appbuilder /ea/include/ /ea/include/
COPY --from=appbuilder /ea/lib/ /ea/lib/
COPY --from=appbuilder /ea/bin/ /ea/bin/
COPY .vscode ./.vscode/
COPY --from=appbuilder /ea/grpc/bin/ /ea/grpc/bin/
COPY --from=appbuilder /ea/grpc/include/ /ea/grpc/include/
COPY --from=appbuilder /ea/grpc/lib/ /ea/grpc/lib/
WORKDIR /ea
RUN ln -s bin/ead /ea/
WORKDIR /data
ENTRYPOINT ["/ea/bin/ead"]
CMD ["-b", "http://0.0.0.0", "-p", "9876"]
VOLUME ["/data"]
EXPOSE 9876/tcp
