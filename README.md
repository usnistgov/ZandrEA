# ZandrEA 

[DAN REWRITE THIS] This repo contains software for commercial building HVAC automated fault detection. It monitors data from the HVAC system (usually via BACnet) and attempts to identify potential faults, while suggesting actions to take to address those faults. Over time it develops a knowledge base to improve its recommendations.

This build consists of four Docker containers orchestrated with docker-compose:
- `ea-rest` is the core REST server component that houses all of the EA functionality, and wraps it with a REST server.
- `ea-webapp` is a React single page (web) application to serve as the user interface and front end.
- `ea-bacnet` is the Python bacnet code to collect data from bacnet devices and push it to the REST server
- `ea-proxy` is a Traefik server running as a reverse proxy for the `ea-rest` and `ea-webapp` containers.

The reverse proxy consolidates all HTTP services (both web app and REST API) - everything runs on the standard HTTP port now. The proxy moves the REST API to a /api URI prefix to avoid conflicts with the web app, and presents all HTTP services as normal port 80 HTTP service.

Not yet implemented: a configuration component and a device discovery component.

The code will build and run on Mac or Linux and can be built outside of Docker for development/testing if need be,
but Docker is the target production environment and provides a standardized build environment cross-platform - in this way it can be
built and and run on a Windows server with Docker installed, if docker-compose is also installed.

Note that as this project has not been developed into a complete production-ready product, it is NOT intended for use in a production environment. *It currently lacks any form of authentication of access control, so although it is read-only as far as your BACnet devices go, any client with network access can push data to it, possibly misleading (at best) or corrupting (at worst) any fault detection or knowledge bases you develop during use. USE AT YOUR OWN RISK!*

## Current Status

As of September 30, 2024:

- The `ea-bacnet` docker container has been reworked, and with proper configuration MIGHT work now. It is currently untested. See the [EAbacnet/README](./EAbacnet/README.md) file for configuration information.
- The `ea-rest` REST daemon is functional. The [REST API documentation](./EAd/REST-API-v3.md) is up to date.
- The `ea-webapp` container is feature-complete (as far as the capabilities available through the REST API)

## How to build

Please see the [INSTALLATION](./INSTALLATION.md) instructions for how to build, run, and test ZandrEA.
