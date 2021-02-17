# How-to Guide: Running your function as Docker container

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[hello-world-http]: /examples/site/hello_world_http/hello_world_http.cc

This guide shows you how to create a container image for an example function,
and how to run said image in a local container on your workstation.

## Pre-requisites

Verify the [docker tool][docker] is functional on your workstation:

```shell
docker run hello-world
# Output: Hello from Docker! and then some more informational messages.
```

If needed, use the [online instructions][docker-install] to download and install
this tool. This guide assumes that you have configured [sudoless docker]. If
you prefer replace all `docker` commands below with `sudo docker`.

Verify the [pack tool][pack-install] is functional on our workstation. These
instructions were tested with v0.17.0, although they should work with newer
versions. Some commands may not work with older versions.

```shell
pack version
# Output: a version number, e.g., 0.17.0+git-d9cb4e7.build-2045
```

In this guide we will be using the [HTTP hello word][hello-world-http] function:

```cc
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse hello_world_http(gcf::HttpRequest request) {
  auto greeting = [r = std::move(request)] {
    auto request_json = nlohmann::json::parse(r.payload(), /*cb=*/nullptr,
                                              /*allow_exceptions=*/false);
    if (request_json.count("name") && request_json["name"].is_string()) {
      return "Hello " + request_json.value("name", "World") + "!";
    }
    return std::string("Hello World!");
  };

  gcf::HttpResponse response;
  response.set_payload(greeting());
  response.set_header("content-type", "text/plain");
  return response;
}
```

## Getting the code for this example

This example is included in the Functions Framework for C++
[source code repository][repository-gh]. Download this code as usual:

```shell
cd $HOME
git clone https://github.com/GoogleCloudPlatform/functions-framework-cpp
```

The rest of this guide will assume you are issuing commands in the framework's
clone:

```shell
cd $HOME/functions-framework-cpp
```

## Setting up the buildpacks builder

We will be using a [buildpacks][buildpacks] builder to create the container
image deployed to Cloud Run. The first time your run these commands it can take
several minutes, maybe as long as an hour, depending on your workstation's
performance.

```sh
docker build -t gcf-cpp-runtime --target gcf-cpp-runtime -f build_scripts/Dockerfile build_scripts
docker build -t gcf-cpp-develop --target gcf-cpp-develop -f build_scripts/Dockerfile .
pack builder create gcf-cpp-builder:bionic --config pack/builder.toml
pack config trusted-builders add gcf-cpp-builder:bionic
pack config default-builder gcf-cpp-builder:bionic
```

## Building a Docker image

Build a Docker image from your function using this buildpack:

```shell
pack build \
    --builder gcf-cpp-builder:bionic \
    --env FUNCTION_SIGNATURE_TYPE=http \
    --env TARGET_FUNCTION=hello_world_http \
    --path examples/site/hello_world_http \
    gcf-cpp-hello-world-http
```

## Starting a local container

Start a Docker container in the background the image you just created:

```shell
ID=$(docker run --detach --rm -p 8080:8080 gcf-cpp-hello-world-http)
```

## Send a request to your function

You can use `curl` (or a similar HTTP client) to send requests to your
function:

```shell
curl http://localhost:8080
# Output: Hello, World!
```

## Cleanup

Stop the background container:

```shell
docker kill "${ID}"
```

And delete the local image:

```shell
docker image rm gcf-cpp-hello-world-http
```