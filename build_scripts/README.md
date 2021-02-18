# Scripts to build Functions Framework-based applications

This directory contains the scripts to

1. Create a Docker image with all the development tools to build applications based on the functions
   framework
1. Create the runtime image to execute applications built with said image
1. Support scripts to automatically create the `main()` entry point.

## Creating the Docker images

```sh
docker build -t gcf-cpp-run-image --target gcf-cpp-runtime - <Dockerfile
docker build -t gcf-cpp-build-image .
```
