# How-to Guide: Running your function as Docker container

1. Install [Docker](https://store.docker.com/search?type=edition&offering=community) and
   the [`pack` tool](https://buildpacks.io/docs/install-pack/).

1. Create the `pack` builder for C++. The first time your run this it can take
   several minutes, maybe as long as an hour, depending on the capabilities of
   your workstation.
   ```shell
   cd $HOME/functions-framework-cpp
   docker build -t gcf-cpp-runtime --target gcf-cpp-runtime -f build_scripts/Dockerfile .
   docker build -t gcf-cpp-develop --target gcf-cpp-develop -f build_scripts/Dockerfile .
   pack create-builder gcf-cpp-builder:bionic --config pack/builder.toml
   pack trust-builder gcf-cpp-builder:bionic
   ```

1. Build a Docker image from your function using this buildpack:
   ```shell
   pack build \
       --builder gcf-cpp-builder:bionic \
       --env FUNCTION_SIGNATURE_TYPE=http \
       --env TARGET_FUNCTION=HelloWorld \
       --path examples/site/hello_world_http \
       gcf-cpp-hello-world-http
   ```

1. Start a Docker container in the background with this image:
   ```shell
   ID=$(docker run --detach --rm -p 8080:8080 gcf-cpp-hello-world-http)
   ```

1. Send requests to this function using `curl`:
   ```shell
   curl http://localhost:8080
   # Output: Hello, World!
   ```

1. Stop the background container:
   ```shell
   docker kill "${ID}"
   ```
