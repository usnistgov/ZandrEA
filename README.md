# ZandrEA;

## What is it?

ZandrEA&trade; is an ongoing, collaborative, open-source software development project.  It supports research into automated, real-time detection and diagnostics of "faults" (unwanted, wasteful conditions) in the heating, ventilating, and air-conditioning (HVAC) systems of large commercial buildings.  The topic is known throughout the HVAC industry as automated fault detection and diagnostics (AFDD). The ZandrEA project was founded by federal employees at the U.S. National Institute of Standards and Technology (NIST). The project shares in NIST's overall mission through research advancing AFDD technology in ways to benefit U.S. enterprises marketing AFDD products and services. The project is "collaborative" because research proposals, development contributions, and technical skills are welcomed from beyond NIST, particularly from other laboratories, universities, private-sector businesses, and individuals engaged in AFDD.

Anyone wanting to learn more about ZandrEA and its application to AFDD research is strongly encouraged to first read the Section 1, Introduction in its "primer" document, NIST Technical Note (TN) 2337, available as a free PDF download at: https://doi.org/10.6028/NIST.TN.2337 

The moniker "EA" preceded the final name "ZandrEA" given to the project, being previously an abbreviation of "Expert Assistant". That association to an abbreviation is now deprecated. However, as a convenient short tag, "EA" is still used extensively throughout the ZandrEA codebase and TN-2337 to encompass the parts of it in the founding NIST contribution. EA should be understood to refer to an evolving subset within a future, potentially larger and more broadly capable, codebase for ZandrEA.

## Implementation Overview

ZandrEA consists of a system of four Docker containers orchestrated with Docker Compose. Running in the Docker environment vastly reduces the requirements to build and run this software. See the [INSTALLATION](./INSTALLATION.md) document for instructions for how to build, run, and test ZandrEA.

There are two types of environments supported by the build process:
1. Development: Intended for software evaluation and development purposes. This is the default build environment.
2. Production: although ZandrEA should NOT be used in any production capacity (see [Current Status](#current-status) below), a more streamlined, better-performing "production" build is available. This build pre-generates all the web pages so they can be directly and immediately served instead, reducing overhead. Most or all debugging capabilities have been stripped out. You might use this on a longer-running test installation, for example, where you want more stability and less overhead.

The four Docker containers include:
- the computational engine (implemented by the `libEA` library code which exposes its functionality via a C++ API) which has been wrapped with a REST API handler to expose its functionality via a traditional HTTP (web) interface
- a front-end GUI dashboard for viewing and interacting with the analysis, written in React (a Javascript framework) for viewing in a standard web browser
- a live data collection script which periodically polls BACnet devices and pushes the collected data into the computational engine via the REST interface for analysis
- a reverse proxy container that merges the REST API and the server for the web client into a single unified web service, and could also be configured to implement SSL/https in a single location

## How to work with this repo

Before attempting to work with this code you should first familiarize yourself with the particular concepts and vocabulary used in ZandrEA by reading at least Sections 1, 2, and 4 of the ZandrEA Primer document, TN-2337, at https://doi.org/10.6028/NIST.TN.2337

We highly recommend creating your own fork of this repo so that you can customize it for your particular environment and needs.

If there are changes you think should be integrated with the base project, please submit them as a [Pull Request](https://github.com/usnistgov/ZandrEA/pulls). In general, we will only be accepting changes that apply to basic functionality that all users will need or can use. Our intention is not to make substantial changes to the base repo since that will make integration of those changes into all of the customized forks difficult. See [README-how-to-update-your-fork](./README-how-to-update-your-fork.md) for a quick overview. More detailed explanations and methods can be found via Google.

If you experience any bugs, please submit them as [Issues](https://github.com/usnistgov/ZandrEA/issues).

## System Requirements

The use of Docker allows for very minimal system requirements and a programatic, scripted build process that is portable across any type of system that can run Docker.

Any host system hardware that can run Docker or Docker Desktop should should suffice for development and testing, as long as:
- At least 4GB of RAM should be available for the Docker containers
- At least 2 CPU cores should be available for the Docker containers
- At least 200GB of disk space should be available for build/development
- At least 50GB of space should be available for a production runtime-only environment (containers built elsewhere)

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
Daniel Veronica<br>NIST Engineering Laboratory, Mechanical Systems & Controls Group<br>daniel.veronica@nist.gov

Please include "ZandrEA" in the subject line.

### Project Home Page:
<https://www.nist.gov/programs-projects/automated-fault-detection-and-diagnostics-mechanical-services-commercial-buildings>

### External Project Documentation:
<https://doi.org/10.6028/NIST.TN.2337>

Information in any document of this repo supercedes the above in case of apparent conflict.

## How to reference this work in citations

TBD.

## Links to referenced software dependencies

| Name | URL | Notes |
| --- | --- | --- |
| Docker | <https://docs.docker.com/engine/install/> | Most Linux distros have a `docker-ce` package (a free "community edition") that will suffice |
| Docker Compose plugin | <https://docs.docker.com/compose/install/> | May already be included with your Docker installation |
| GNU make | <https://www.gnu.org/software/make/> | Install from Linux distro, or as part of software development framework, or sometimes standalone |
| HDF5 | <https://www.hdfgroup.org/solutions/hdf5/> | Source code is included in this repo |
| Cereal | <https://uscilab.github.io/cereal/> | Open source C++ serialization library |
| CPPRESTSDK | <https://github.com/microsoft/cpprestsdk> | The opensource C++ REST API library used by the REST container |

## Current Status

Note that as the purpose of this project is to facilitate research, it has intentionally NOT been developed into a complete production-ready product. It is NOT intended for use as-is in a production environment. **It currently lacks any form of authentication or access control, so although it is read-only as far as your BACnet devices go, any client with network access can push data to it, possibly misleading (at best) or corrupting (at worst) any fault detection or knowledge bases you develop during use. We recommend that you only run it on a private network without Internet access - which you should be doing with your building BACnet network also. DO NOT expose this application to the open Internet. _USE AT YOUR OWN RISK!_**

As of October 1, 2024:

- The `ea-rest` REST daemon is functional. The [REST API documentation](./EAd/REST-API-v3.md) is up to date.
- The `ea-webapp` container is feature-complete (as far as the capabilities available through the REST API)
- The `ea-bacnet` docker container has been reworked, and with proper configuration MIGHT work now. It is currently untested. See the [EAbacnet/README](./EAbacnet/README.md) file for configuration information.
