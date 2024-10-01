# REST server testing

## Native build testing
You can run a suite of automated tests in a native (Linux or Mac) build by running:

```
make test
```

The test suite is currently not comprehensive but additional tests are trivial to add. When run this way, the test suite starts up a native REST server daemon, runs the tests, then shuts down the daemon and cleans up temp files afterwards. (So don't run this if you already started your own REST server.)

## Manually running test scripts
Most of the test scripts can be run from a command prompt as well, as long as the REST server is running. Most of them use http://localhost:9876/ as the base URL for the REST API, which should work for both natively running REST daemon and the docker container in the standard development build.

You can adapt it if needed by setting environment variables:
```
export EA_BASEURL="http://localhost/api"
```
(This would use the port 80 interface provided by the docker proxy container.)

The `.js` scripts are javascript, intended to by run using NodeJS. The `.py` scripts are Python.

### Required Python modules
To run the python tests, you'll need to install `requests` and `python3-dateutil`:

```
python3 -m pip install requests python3-dateutil
```

## Pushing test data
The most useful scripts can be used to push test data up into the DLL via the REST server.

| ead-functest-combined.py | Simple script to push the original 192-sample dataset as fast as possible |
| ead-push-date-time-ahu-vav-from-csv.py | Script to push Mike Galler's full-day datasets. Highly configurable. Run with `--help` for full set of command line options and description. |

### Test data files
Test data files are stored in the `testdata` directory.

Currently I only have one set of actual test data as provided by Mike Galler. It has known faults. Use the `.csv` file as input to the `ead-push-date-time-ahu-vav-from-csv.py` script. The known faults are described in the corresponding text file.

There are also minimal test data sets for testing missing and invalid data point display.

### Simulated real time data
You can use the script with options and the full-day test data file to simulate real time data.

```
./ead-push-date-time-ahu-vav-from-csv.py --loop --time now --timestep 60 --interval 60 testdata/Full_day_test_data_1AHU_2VAV_with_faults_190823.csv
```
