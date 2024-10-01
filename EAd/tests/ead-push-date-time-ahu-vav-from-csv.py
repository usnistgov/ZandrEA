#!/usr/bin/env python3

import json
import requests
import time
import sys
import os
from dateutil.parser import parse
import re
import argparse

# Expected points must match subject to determine subject type
#pointsAHU = ["Psas", "Tao", "Udm", "Tam",  "Tar",  "Uvc",  "Tas", "TasSetpt", "Bso", "Uvh", "Qas"]
pointsAHU = [
        "Pressure_static_air_supply",
        "Temperature_air_outside",
        "Position_damper_mixingBox",
        "Temperature_air_mixed",
        "Temperature_air_return",
        "Position_valve_chw",
        "Temperature_air_supply",
        "Temperature_air_supply_setpt",
        "Binary_systemOccupied",
        "Position_valve_hw",
        "FlowRateVolume_air_ahu"
]
pointsVAV = [
        "Pressure_static_air_supply",
        "Temperature_air_supply",
        "Temperature_air_discharge",
        "Temperature_air_zone",
        "Temperature_air_zone_setpt_htg",
        "Temperature_air_zone_setpt_clg",
        "Position_valve_hw",
        "Position_damper_vav",
        "FlowRateVolume_air_vav",
        "FlowRateVolume_air_vav_setpt",
        "Binary_zoneOccupied"
]

# Column definitions in the CSV for this 1AHU+6VAV datafile
c_date = 0
c_time = 1
c_gtc = 2
c_data = 3
ahu_channels = len(pointsAHU)
vav_channels = len(pointsVAV)

# Parse command line options
cli = argparse.ArgumentParser(
    description="Used to push stored data samples from a CSV file to the EA REST server",
    epilog="""
THE CSV FORMAT MUST MATCH THE FORMAT EXPECTED BY THIS SCRIPT.
CSV input files must use comma-separated column format:
Col     Description
{}      date
{}      time
{}      ground truth code
followed by 0 or more AHUs of {} columns each (not currently used). Remaining columns are assumed to be 0 or more VAV of {} columns each.

In turn, the devices in the datafile will be matched to the devices in the DLL simply by order. Currently the script only recognizes DLL subjects as VAVs. AHU data will be ignored. VAVs are matched to subjects in the orders provided by both the datafile and the DLL. (First subject from DLL -> first VAV in datafile). This loose coupling is fragile so beware.
""".format(c_date, c_time, c_gtc, ahu_channels, vav_channels),
)
cli.add_argument("filenames", help="Name of the CSV file(s) to load (- for stdin)", nargs="+")
cli.add_argument("-a", "--ahu-count", help="Number of AHUs in the input files", type=int, default=2)
cli.add_argument("-l", "--loop", help="Repeat forever", action="store_true")
cli.add_argument("-i", "--interval", help="Time to pause between samples", type=int, default=0)
cli.add_argument("-t", "--time", help="Time string for starting timestamp - overrides timestamps in data (use 'now' for current time)")
cli.add_argument("-s", "--timestep", help="Seconds to increment timestamp between samples (only used with --time)", type=int, default=60)
cli.add_argument("-u", "--baseurl", help="Base URL to use for REST API")
cli.add_argument("-p", "--port", help="Port number to use for REST API URL (EA_PORT)", type=int, default=int(os.getenv("EA_PORT", "9876")))
cli.add_argument("-H", "--host", help="Hostname to use for REST API URL (EA_HOST)", default=os.getenv("EA_HOST", "127.0.0.1"))
cli.add_argument("-P", "--protocol", help="Protocol to use for REST API URL (EA_PROTO)", default=os.getenv("EA_PROTO", "http"))
cli.add_argument("-A", "--api-path", help="Path prefix to use for REST API URL (EA_APIPATH)", default=os.getenv("EA_APIPATH", ""))
args = cli.parse_args()

proto = args.protocol
host = args.host
port = args.port
apipath = args.api_path.strip().rstrip('/')
if args.baseurl:
    baseurl = args.baseurl
else:
    baseurl = os.getenv("EA_BASEURL", "{}://{}:{}{}".format(proto, host, port, apipath))

# Initialize some other variables
headers = {'Content-Type': 'application/json'}
subjects = {}
subjectkeys = []
vavkeys = []
ahukeys = []
seen_alerts = dict()

# Column definitions in the CSV for this set of files
index_ahu = c_data
index_vav0 = index_ahu + (args.ahu_count * ahu_channels)

# Leaving this stub to remember the order for VAV channels
data = [
   # Psas
   # Tas
   # Tad
   # Taz
   # TazSetHtg
   # TazSetClg
   # Zvh
   # Zd
   # Qa
   # QaSet
   # Bocc
]

def GetSubjectKeys():
    global subjectkeys, baseurl, headers
    url = "{}{}".format(baseurl, "/domain?compact=1")
    response = requests.get(url, headers=headers)
    assert response.status_code >= 200 and response.status_code <= 299
    subjectkeys = response.json().get("subjectkeys")

def GetSubjects():
    global subjects, baseurl, headers
    url = "{}{}".format(baseurl, "/subjects")
    response = requests.get(url, headers=headers)
    assert response.status_code >= 200 and response.status_code <= 299
    subjects = response.json().get("subjects")

def SetCurrentTM(timestamp):
    global baseurl, headers
    url = "{}{}".format(baseurl, "/ctrl/time")
    body = {'time': timestamp}
    response = requests.put(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    #print("SetCurrentTM({}) succeeded".format(timestamp))

def SetCurrentSamples(subject, samples):
    global baseurl, headers
    url = "{}{}".format(baseurl, "/ctrl/sample")
    body = {'subject': subject, 'values': samples}
    response = requests.put(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    #print("SetCurrentSamples() succeeded")

def SampleTimeStep(timestamp, valuesbysubject):
    """ Push timestamp, push values to ALL subjects, and singlestep """
    global subjectkeys, baseurl, headers
    url = "{}{}".format(baseurl, "/ctrl/sampletimestep")
    body = {'time': timestamp, 'values_by_subject': valuesbysubject}
    response = requests.put(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    #print("SetCurrentSamples() succeeded")

def SingleStepDomainOnTimeAndInputs():
    global baseurl, headers
    url = "{}{}".format(baseurl, '/ctrl/singlestep')
    response = requests.post(url, headers=headers)
    assert response.status_code >= 200 and response.status_code <= 299
    #print("SingleStepDomainOnTimeAndInputs() succeeded")

def SayNewAlertsFifoFromDomainThenClear():
    global seen_alerts, baseurl, headers
    newalerts = []
    url = "{}{}".format(baseurl, '/alerts')
    response = requests.get(url, headers=headers)
    assert response.status_code >= 200 and response.status_code <= 299
    for a in response.json().get("alerts"):
        if a["id"] not in seen_alerts:
            newalerts.append(a["message"])
            seen_alerts[a["id"]] = 1
    return newalerts

def SayCaseKeys(subject):
    global baseurl, headers
    url = "{}{}".format(baseurl, '/casekeys')
    body = {'subject': subject}
    response = requests.get(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    casekeys = response.json().get("casekeys")
    #print("SayCaseKeys() returned ", casekeys)
    return casekeys

def SayCase(c,subject):
    global baseurl, headers
    url = "{}{}".format(baseurl, '/case')
    body = {'key': c, 'subject':subject}
    response = requests.get(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    case = response.json()
    #print("SayCase({}) returned ".format(c), case)
    return case

def SayFeatureKeys(subject):
    global baseurl, headers
    url = "{}{}".format(baseurl, '/featurekeys')
    body = {'subject': subject}
    response = requests.get(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    featurekeys = response.json().get("featurekeys")
    #print("SayFeatureKeys() returned ", featurekeys)
    return featurekeys

def SayFeature(f):
    global baseurl, headers
    url = "{}{}".format(baseurl, '/feature')
    body = {'key': f}
    response = requests.get(url, headers=headers, json=body)
    assert response.status_code >= 200 and response.status_code <= 299
    feature = response.json()
    #print("SayFeature({}) returned ".format(k), feature)
    return feature


GetSubjectKeys()
GetSubjects()

AHUs = []
VAVs = []

for s in subjects:
    if s['points'] == pointsVAV:
        VAVs.append(s['key'])
        print("Subject {} ({}) identified as VAV".format(s['key'], s['idtext']))
    elif s['points'] == pointsAHU:
        AHUs.append(s['key'])
        print("Subject {} ({}) identified as AHU".format(s['key'], s['idtext']))
    else:
        print("WARNING: subject {} has unrecognized type with points {}".format(s['key'], s['points']))

if args.time == "now":
    timestamp = int(time.time())
elif args.time != None:
    timestamp = int(time.mktime(parse(args.time).timetuple()))
else:
    timestamp = None

while True:   # Python version of a do-while loop (do...while(args.loop))

    for datafile in args.filenames:  # loop through all input files
        if datafile == '-':
            datafile = "/dev/stdin"

        if not os.access(datafile, os.R_OK):
            print("ERROR: datafile '{}': unable to read, ABORTING".format(datafile))
            sys.exit(1)

        with open(datafile) as csvfile:
            headerline = csvfile.readline().rstrip().strip()
            header = re.split(r'\s*,\s*', headerline)
            linenum = 0
            count = 0

            for line in csvfile:

                linenum = linenum + 1
                row = re.split(r'\s*,\s*', line.rstrip().strip())
        
                if timestamp:
                    ts = timestamp
                else:
                    ts_string = row[c_date] + " " + row[c_time]
                    ts = int(time.mktime(parse(ts_string).timetuple()))

                values_by_subject = []
                for i in range(len(AHUs)):
                    s = AHUs[i]
                    j = (i * ahu_channels) + index_ahu
                    if j < len(row):
                        sample = [float(x) for x in row[j:j+ahu_channels]]
                        values_by_subject.append({'subject': s, 'values': sample})
                    else:
                        print("WARNING: not enough columns in {} for AHU subject {} (skipped)".format(datafile, s))
            
                for i in range(len(VAVs)):
                    s = VAVs[i]
                    j = (i * vav_channels) + index_vav0
                    if j < len(row):
                        sample = [float(x) for x in row[j:j+vav_channels]]
                        values_by_subject.append({'subject': s, 'values': sample})
                    else:
                        print("WARNING: not enough columns in {} for VAV subject {} (skipped)".format(datafile, s))

                SampleTimeStep(ts, values_by_subject)  # Consolidates SetCurrentSamples, SetCurrentTM, and SingleStepAFDD

                count = count + 1
                print("Single step {} taken for timestamp {}".format(count, ts))

                if timestamp:
                    timestamp += args.timestep

                # sleep if needed
                if args.interval > 0:
                    time.sleep(args.interval)

                alertsNow = SayNewAlertsFifoFromDomainThenClear()
                for a in alertsNow:
                    print(a)

    # If not looping we can quit now
    if args.loop == False:
        break

# Dan's code comments this after the API update out but I need it to validate
for s in subjectkeys:
    casekeys = SayCaseKeys(s)
    for cid in casekeys:
        print("")
        c = SayCase(cid,s)
        error = c.get("error")
        if error is None:
            label = c.get("label")
            report = c.get("report")
            print("WARNING: case {} from subject {}: {}: {}".format(cid, s, c.get("label"), c.get("report")))
        else:
            print("ERROR: case {} from subject {}: SayCase failed".format(cid, s))

