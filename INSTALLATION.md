## How to build in Docker

Make sure you have Docker (Docker-CE is free and fine on Linux; on Windows or Mac you'll probably need to download Docker Desktop and possibly purchase a license) installed on your workstation. You will also need a recent version of the docker compose plugin - maybe not the one from your distro since it's likely older. It can be installed (on Linux or Mac) from [Homebrew](https://brew.sh/) but you can also get it directly from docker: <https://docs.docker.com/compose/install/>. It also comes bundled with Docker Desktop.

Before starting, note that this repo supports two types of builds: the default development build, which allows run-time updates in the source repo to the front-end code for faster development and testing and uses a bind mount for the knowledge base volume for easier access from the host, as well as adding some remote debugging capabilities to the REST container; and a production build which uses a persistent docker named volume for the knowledge base and generates containers with the pre-compiled static React front-end application suitable for deployment on a production server.

Then do the following to clone the repo, build, and start the Docker containers (for the development version):

```
git clone https://github.com/usnistgov/ZandrEA.git
cd ZandrEA
make docker-up      # Starts the application containers. Same as docker compose up --build -d
make pushtestdata   # Push some sample data in to make sure everything is working (requires local python3)
make docker-down    # Stops the application containers. Same as docker compose down
```

Once the containers are up, you can point your web browser to the web interface. If the containers are running on your local workstation, use <http://localhost/> or replace localhost with the hostname of the server running the containers.

To run the production version instead, make sure the development version is shut down first (they cannot both run at the same time), then inside the repo directory use the same make commands, but instead of `docker-up` use `docker-production-up` ... similarly, `docker-down` becomes `docker-production-down`, etc.  You can find all the supported make target names in the Makefile. See [README-production](./README-production.md) for more information.

### Knowledge Base storage

The default development build compose uses a bind-mount of a directory named `data` in the repo root directory, which allows easier examination of the HDF5 knowledgebase files during development. Note that since docker runs the containers as root, the data directory and its contents will be owned by root.

The production build instead creates a Docker named volume (named `rest-kb`) where the HDF5 files will be stored.

*IMPORTANT:* In either case this volume is persistent, so if you stop and restart the docker containers the knowledge base will persist!

In order to start fresh you can remove that volume by one of the following:

Development:
```
sudo rm -rf data    # You should stop the containers first
```

Production:
```
make docker-rm-kb   # Just deletes the persistent volume (must not be in use)
make docker-clean   # Stops and removes ALL docker containers, images, volumes
```

## How to access the web app

Simply point your browser to <http://localhost/>. Note that the default development build also still exports the non-proxy interfaces, but beware that cross-domain and cross-site scripting blocking by browsers may interfere with operation this way. (API: <http://localhost:9876/>, web app: <http://localhost:3000/> )

Barring any firewall rules to the contrary, you can also access the web app and API from a remote machine, e.g. <http://example.com/> and <http://example.com/api/...>.

## Testing the REST server

You can use `curl` from the command line to query the REST server and display the JSON returned. Note if using the default developer build that the API endpoits can also be accessed on port 9876 without the /api prefix. If running outside of Docker you will have to use port 9876.

```
# On Docker:
curl --compressed http://localhost/api/noop                # no-op to check connection and seq
# Alternatively:
curl --compressed http://localhost:9876/noop               # no-op to check connection and seq

curl --compressed http://localhost/api/subject?subject=2   # dump subject 2
curl --compressed http://localhost/api/subjects            # dump all subjects
curl --compressed http://localhost/api/subjects?details=2  # shallow dump all subjects but full dump of subject 2
curl --compressed http://localhost/api/alerts              # list all alerts
curl --compressed http://localhost/api/cases?subject=2     # list all cases for the given subject
```

There are a number of ways to pretty-print the JSON for easier viewing:
- pipe to the python JSON tool: `curl ... | python3 -m json.tool | less`
- pipe to `jsonpp` from homebrew/linuxbrew

You can also use curl with the PUT method to change knob settings, submit data, or provide feedback to cases. See the scripts in `EAd/tests/` for details. REST API documentation is currently out of date so see EAd/handler.cpp for details about available REST endpoints.

## Pushing sample data

You can use one of the test scripts to push sample data so you'll have something more interesting to look at.

First you'll need to set up a Python venv with the modules the script will need:
```
python3 -m venv venv
venv/bin/pip3 install -r EAd/tests/requirements.txt
```

Then you can run test scripts, like:
```
venv/bin/python3 EAd/tests/ead_functest_combined.py
venv/bin/python3 EAd/tests/ead-push-date-time-ahu-vav-from-csv.py --time now --timestep 60 EAd/tests/testdata/ibal_240227_NF.csv
```

## How to build the REST server outside of Docker

This is not commonly used anymore so some information may be out of date. However, note that a Docker build will run the a `make install` on an Ubuntu base image, so that much should continue to work fine if installing directly on an Ubuntu host or VM. See the Dockerfile for the exact commands used to set up the host for building.

First see [README-build-deps](README-build-deps.md) and build/install dependencies as needed.

Once your development environment is set up you can then:
```
make
make test
make install
```

That will install the libEA library in `lib/`, and the REST server `ead` in the `bin/` directory.

### Testing libEA outside of Docker

There are a number of functional test scripts in `EAd/tests`. You can run them in an automated way from the top level directory with::
```
make test
```

Some of those test scripts only work with non-Docker builds.

### How to build and run the JavaScript client outside of Docker

```
cd eajscli
npm install
npm start
```

This will run the client webapp on port 3000 by default (or the next available port after that if 3000 is already in use), so you can access it via <http://localhost:3000/>
