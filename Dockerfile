FROM ubuntu:24.04 AS baseos
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
# Bare minimum of libraries needed to run the above executable
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
#    build-essential \  [moved to baseos for debugging dev ctnr - DAV]
    libaec-dev \
    nodejs \
  && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /ea/include/ea /ea/lib/ea /ea/bin /data
ENV PATH $PKGROOT/HDF5/bin:$PATH
WORKDIR $PKGROOT
COPY hdf5-1.14.4-2.tar.gz h5c++.tmpl Makefile ./
RUN make compiler

FROM buildbase AS appbuilder
LABEL maintainer="Steve Barber <steve.barber@nist.gov>"
ARG home=/ea
ENV HOME=${home}
ENV PKGROOT=${home}
ENV PATH $PKGROOT/HDF5/bin:$PATH
WORKDIR $PKGROOT
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
WORKDIR /ea
RUN ln -s bin/ead /ea/
WORKDIR /data
ENTRYPOINT ["/ea/bin/ead"]
CMD ["-b", "http://0.0.0.0", "-p", "9876"]
VOLUME ["/data"]
EXPOSE 9876/tcp
