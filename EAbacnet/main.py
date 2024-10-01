#!/usr/bin/env python3

import sys, os.path
from time import sleep, time, ctime, localtime
from subprocess import run, PIPE, TimeoutExpired
from configparser import ConfigParser

default_interval = 60       # How often to gather samples (in secs)
configdir = "config"
pypesfile = "BACpypes.ini"
inifile = "ead.ini"
timeout = 8                 # Maximum length of time to allow for sample collection (in secs)

# Change into the configuration directory
if not os.path.isdir(configdir):
    print(f"ERROR: BACnet configuration directory {configdir} does not exist; exiting", file=sys.stderr)
    sys.exit(1)
os.chdir(configdir)

# Read the sample interval from the ead.ini config file
if not os.path.exists(inifile):
    print(f"ERROR: BACnet configuration file {inifile} does not exist; exiting", file=sys.stderr)
    sys.exit(1)
eadparser = ConfigParser()
eadparser.read(inifile)
config = eadparser["DEFAULT"]
interval = config.getint("interval", default_interval)

# Make sure the BACpypes config file is present
if not os.path.exists(pypesfile):
    print(f"ERROR: BACpypes configuration file {pypesfile} does not exist; exiting", file=sys.stderr)
    sys.exit(1)

print(ctime())
sleep(interval - (localtime()[5] % interval))
while True:
    print(ctime())
    try:
        cp = run(["python3", "../bacnet2ead.py"], timeout=timeout, stdout=PIPE)
        print(cp.stdout)
    except TimeoutExpired:
        print("WARNING: BACnet poll timed out!", file=sys.stderr)
    sleep(interval - (timeout + 1))  # gets us most of the way there (past any underlap)
    sleep(interval - (localtime()[5] % interval))  # the last little bit
