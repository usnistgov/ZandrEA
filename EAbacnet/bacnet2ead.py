#!/usr/bin/env python3

"""
bacnet2ead.py

This application reads a static list of points that it would like to poll from
an INI config file.  It reads the presentValue of each of them in turn and then
quits.

TODO: push the collected data to the EAd

TODO: dockerize
"""

import sys
import asyncio
import json
import os.path
from configparser import ConfigParser, ExtendedInterpolation

from bacpypes3.settings import settings
from bacpypes3.debugging import bacpypes_debugging, ModuleLogger
from bacpypes3.argparse import SimpleArgumentParser
from bacpypes3.app import Application

from bacpypes3.lib.batchread import DeviceAddressObjectPropertyReference, BatchRead

# some debugging
_debug = 0
_log = ModuleLogger(globals())


@bacpypes_debugging
def callback(key, value) -> None:
    if _debug:
        _log.debug("callback %r %r", key, value)

    print(f"{key} = {value}")


async def main() -> None:
    try:
        app = None
        parser = SimpleArgumentParser()
        parser.add_argument(
            "--ini",
            help="name of the INI file",
            default="ead.ini",
        )

        args = parser.parse_args()
        if _debug:
            _log.debug("args: %r", args)
            _log.debug("settings: %r", settings)

        # build the application
        app = Application.from_args(args)

        # Read the settings for the REST server
        if not os.path.exists(args.ini):
            if _debug:
                _log.debug("Aborting: %r does not exist", args.ini)
            sys.exit(0)
        eadparser = ConfigParser()
        eadparser.read(args.ini)
        if _debug:
            _log.debug("eadparser.keys: %r", list(eadparser.keys()))

        # get the devices, the list of additional INI files to read in
        config = eadparser["DEFAULT"]
        url = config.get("url", "")
        devices = json.loads(config.get("devices", "[]"))
        interval = config.getint("interval", 60)
        if _debug:
            _log.debug("interval: %r", interval)

        if url == "":
            sys.stderr.write(f"{args.ini}: 'url' property must be defined\n")
            sys.exit(1)
        if len(devices) <= 0:
            sys.stderr.write(
                f"{args.ini}: 'devices' property must exist with at least one value\n"
            )
            sys.exit(1)

        # Now read the settings for the device(s) and generate a point_list
        # ExtendedInterpolation is required here!
        point_list = []
        for d in devices:
            if _debug:
                _log.debug("d: %r", d)

            dp = ConfigParser(interpolation=ExtendedInterpolation())
            dp.read("{}.ini".format(d))
            if _debug:
                _log.debug("dp.keys: %r", list(dp.keys()))

            for pname in json.loads(dp["Points"]["points"]):
                daopr = DeviceAddressObjectPropertyReference(
                    d + "." + pname,  # key
                    dp[pname]["devId"],  # deviceAddress
                    dp[pname]["channel"],  # objectIdentifier
                    dp[pname]["dataType"],  # propertyIdentifier
                )
                point_list.append(daopr)

        if not point_list:
            sys.stderr.write("point_list evaluated to an empty list\n")
            sys.exit(1)

        # create a batch object
        batch_read = BatchRead(point_list)

        # run until the batch is complete
        await batch_read.run(app, callback)

    finally:
        if app:
            app.close()


if __name__ == "__main__":
    asyncio.run(main())
