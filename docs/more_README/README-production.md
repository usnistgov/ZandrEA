# Making a Production build for deployment

The default build is a development build, which makes it easier to make live updates to the eajscli web
client code (the changes will be loaded immediately), as well as to debug the REST/libEA container code.

For the HDF5 knowledge base files, The production build uses named Docker volumes (persistent) instead of the
bind mounts of local directories used by the development build. The latter allows easier access to the knowledge
base hdf5 files for debugging, but is messier when deploying to purpose-built server.

The production build also pre-generates static HTML/JS pages that are served by an nginx dedicated web server
rather than by a development server (more secure; more performance).

To work with production images, first create (in the repo's root directory) a file named `production.env`
to set up the Docker registry and image naming you want. You need to define these four variables, something
like this:
```
DOCKER_REGISTRY=hub.docker.com/
DOCKER_PROJECT=your_dockerhub_username
DOCKER_IMAGE_PREFIX=
DOCKER_IMAGE_SUFFIX=_prod
```

Things to note:
1. The registry name should be the hostname of the image registry you will use *WITH A TRAILING SLASH*, or else blank.
1. The project name could either be the name of your project (e.g. zandrea) or your registry username if required by the registry. It cannot not be blank because it's used for naming docker resources when starting up the containers.
1. The image prefix is an optional string to add in front of the service names to create the image name.
1. The image suffix is an optional string to add at the end of the service names to create the image name.

The full image name is generated as `${DOCKER_REGISTRY}${DOCKER_PROJECT}/${DOCKER_IMAGE_PREFIX}servicename${DOCKER_IMAGE_SUFFIX}, where servicename will be one of "ead", "eawebapp", or "eabacnet".

Now you can use the targets in the Makefile with the `docker-production-` prefix, e.g.

```
make docker-production-build  # generates production-images.tar.gz for manual transfer to a remote server
make docker-production-up     # starts the production containers
make docker-production-stop   # stops the production containers
make docker-production-push   # pushes the production containers to the registry (hub)
```

## Transferring images to a production docker server

There are three options provided:

1. If you are building the project on your production server (not usually recommended), you don't need to do anything, the images that are built can be used directly.
1. If your production server has access to your docker registry server ($DOCKER_REGISTRY), the DOCKER_ variables above are set correctly, and you have used `docker login` to log in to that registry, you can use `make docker-production-push` to push the images to that registry server. On the production server you would then create a compose file that could pull those images from the registry.
1. If you can't or don't want to use a registry, the Makefile creates a file named `production-images.tar.gz` for you when you do a production build. You can copy that file to the production server and then load the images with, e.g. `gunzip < production-images.tar.gz | docker image load`
