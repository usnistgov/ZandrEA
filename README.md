# ZandrEA 

## What is it?

ZandrEA is a software framework supporting research into the automated, real-time detection and diagnostics of operational faults in the heating, ventilating, and air-conditioning (HVAC) systems of large commercial buildings.

It consists of a system of four Docker containers orchestrated with Docker Compose. Running in the Docker environment vastly reduces the requirements to build and run this software. See the [INSTALLATION](./INSTALLATION.md) document for instructions for how to build, run, and test ZandrEA.

There are two types of environments supported by the build process:
1. Development: Intended for software evaluation and development purposes. This is the default build environment.
2. Production: although ZandrEA should NOT be used in any production capacity (see [Current Status](#current-status) below), a more streamlined, better-performing "production" build is available. This build pre-generates all the web pages so they can be directly and immediately served instead, reducing overhead. Most or all debugging capabilities have been stripped out. You might use this on a longer-running test installation, for example, where you want more stability and less overhead.

## System Requirements

The use of Docker allows for very minimal system requirements and a programatic, scripted build process that is portable across any type of system that can run Docker.

Any host system hardware that can run Docker or Docker Desktop should should suffice for development and testing, as long as:
- At least 4GB of RAM should be available for the Docker containers
- At least 2 CPU cores should be available for the Docker containers
- At least 200GB of disk space should be available for build/development
- At least 50GB of space should be available for a production-only runtime environment

Development has been done primarily on Intel CPU systems. Builds have also been successful on Mac M1 and M2 CPUs (modern Mac systems) but be aware that containers built on one CPU type won't (or may not) run on another CPU type. If you need to use different CPU types, investigate the Docker platform environment variables.

### Hardware/Operating System

Any host that can run Docker (or Docker Desktop) should be able to build and run this project, regardless of the host hardware or operating system, as long as it meets the minimal hardware resource requirements listed above.

### Software Requirements on the host system

- Docker or Docker Desktop
- The `make` utility (preferably a modern GNU version) is used create shortcuts and manage dependencies in the build process. Technically you could get by without having it on the host, and just issue the same `docker compose` commands manually, but it does simplify things.

## Overview about the structure of this implementation

This build consists of four Docker containers orchestrated with docker-compose:
- `ea-rest` is the core REST server component that houses all of the EA functionality, and wraps it with a REST server.
- `ea-webapp` is a React single page (web) application to serve as the user interface and front end.
- `ea-bacnet` is the Python bacnet code to collect data from bacnet devices and push it to the REST server
- `ea-proxy` is a Traefik server running as a reverse proxy for the `ea-rest` and `ea-webapp` containers.

The reverse proxy consolidates all HTTP services (both web app and REST API) - everything runs on the standard HTTP port now. The proxy moves the REST API to a /api URI prefix to avoid conflicts with the web app, and presents all HTTP services as normal port 80 HTTP service.

Not yet implemented: a configuration component and a device discovery component.

The code will build and run on Mac or Linux and can be built outside of Docker for development/testing if need be,
but Docker is the target production environment and provides a standardized build environment cross-platform - in this way it can be built and run on a Windows server with Docker installed, if docker-compose is also installed.

## Contact Information

### Principal Investigator
Daniel Veronica<br>NIST Engineering Laboratory<br>Building Energy and Environment Division<br>Mechanical Systems and Controls Group (732.02)<br>daniel.veronica@nist.gov

### Project Home Page:
<https://www.nist.gov/programs-projects/automated-fault-detection-and-diagnostics-mechanical-services-commercial-buildings>

### External Project Documentation:
TBD.

## How to reference this work in citations

TBD.

## Links to referenced software dependencies

| Name | URL | Notes |
| --- | --- | --- |
| Docker | <https://docs.docker.com/engine/install/> | Most Linux distros have a `docker-ce` package (a free "community edition") that will suffice |
| Docker Compose plugin | <https://docs.docker.com/compose/install/> | May already be included with your Docker installation |
| GNU make | <https://www.gnu.org/software/make/> | Install from Linux distro, or as part of software development framework, or sometimes standalone |
| HDF5 | <https://www.hdfgroup.org/solutions/hdf5/> | Source code is included in this repo |

## Current Status

Note that as the purpose of this project is to facilitate research, it has intentionally NOT been developed into a complete production-ready product. It is NOT intended for use as-is in a production environment. **It currently lacks any form of authentication or access control, so although it is read-only as far as your BACnet devices go, any client with network access can push data to it, possibly misleading (at best) or corrupting (at worst) any fault detection or knowledge bases you develop during use. We recommend that you only run it on a private network without Internet access - which you should be doing with your building BACnet network also. DO NOT expose this application to the open Internet. _USE AT YOUR OWN RISK!_**

As of October 1, 2024:

- The `ea-rest` REST daemon is functional. The [REST API documentation](./EAd/REST-API-v3.md) is up to date.
- The `ea-webapp` container is feature-complete (as far as the capabilities available through the REST API)
- The `ea-bacnet` docker container has been reworked, and with proper configuration MIGHT work now. It is currently untested. See the [EAbacnet/README](./EAbacnet/README.md) file for configuration information.
