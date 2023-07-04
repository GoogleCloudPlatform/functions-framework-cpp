# How-to Guide: Running your function as Docker container

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

In this guide we will be using this [function][snippet source]:

<!-- inject-snippet-start -->
[snippet source]: /examples/site/hello_world_http/hello_world_http.cc
```cc
#include <google/cloud/functions/function.h>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse hello_world_http_impl(gcf::HttpRequest request) {
  auto greeting = [r = std::move(request)] {
    auto request_json = nlohmann::json::parse(r.payload(), /*cb=*/nullptr,
                                              /*allow_exceptions=*/false);
    if (request_json.count("name") && request_json["name"].is_string()) {
      return "Hello " + request_json.value("name", "World") + "!";
    }
    return std::string("Hello World!");
  };

  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload(greeting());
}

gcf::Function hello_world_http() {
  return gcf::MakeFunction(hello_world_http_impl);
}
```
<!-- inject-snippet-end -->

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

## Building a Docker image

> :warning: This will automatically download and compile the functions
> framework and all its dependencies. Consequently, the first build of a
> function may take several minutes (and up to an hour) depending on the
> performance of your workstation. Subsequent builds cache many binary
> artifacts, but these caches are *not* shared across functions, so plan
> accordingly.

We use the [Google Cloud buildpack] builder to create the Docker image
containing your function:

```shell
pack build \
    --builder gcr.io/buildpacks/builder:latest \
    --env GOOGLE_FUNCTION_TARGET=hello_world_http \
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

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[Google Cloud buildpack]: https://github.com/GoogleCloudPlatform/buildpacks
