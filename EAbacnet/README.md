# EAbacnet

This directory contains the files needed for a container that can be configured to gather live BACnet data and send it to the ZandrEA analysis engine for processing.

By default, the continer will be built and run by the docker-compose files, but will not have any configuration files out of the box, so the container will continue to fault and restart until configuration files are added. This is by design, so that the lack of required configuration files will be apparent. If you will not be using the ea-bacnet container then you can comment out that service definition in the docker-compose file.

This code has been revised with updates and feedback from Joel Bender, author of the BACpypes3 library in use, but has not been tested yet so you have been warned - it may take some work to get live data collection working.

## Adding configuration

The configuration files for the BACnet collection script have been moved into a separate directory named `config` so that the configurations don't have to be built in to the docker image, but can instead be mounted by `docker compose` at run-time, so they can be updated as needed. This will make a swarm deployment trickier, but otherwise should save having to rebuild the images to adjust the BACnet configuration.

Note that the default (development) compose file simply bind-mounts the entire EAbacnet directory onto /data in the container, but the production version copies the script files into the container for security, and bind-mounts only the config directory.

This code uses [Joel Bender's BACpypes3 python library](https://github.com/JoelBender/BACpypes3) for communication with BACnet devices, so the first step is creating a `BACpypes.ini` file in the `EAbacnet/config` directory. Please see the [BACpypes3 documentation](https://bacpypes3.readthedocs.io/en/latest/) for details about configuring BACpypes. `BACpypes.ini.sample` can be used as a starting point.

The next step is creating an `ead.ini` configuration file, which will define the REST server's URL (leave default which refers to the internal Docker hostname for the rest container, unless you know what you're doing) and adding a list of device ("Subject") names, and potentially changing the interval to wait before polling for new data.

After that you will need to create ini files for each of the defined subject names you listed in `ead.ini`.

As of this writing, the libEA configuration is hardcoded to 4 VAV subjects and 2 AHU subjects at compile/build time. In the future we hope more flexible run-time configuration will be added, or you may customize your fork of this project for your environment. (See: [libEA/tool.cpp](../libEA/tool.cpp) and [libEA/tool.hpp](../libEA/tool.hpp))

There are sample files in the `EAbacnet/config` directory for all of these configuration files.
