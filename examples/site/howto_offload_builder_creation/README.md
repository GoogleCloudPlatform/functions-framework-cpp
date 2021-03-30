# How-to Guide: Use Cloud Build to create a buildpacks builder

This guide shows you how to use [Cloud Build][cloud-build] to offload the
creation of [buildpacks] builders.

## Pre-requisites

This guide assumes you are familiar with Google Cloud, and that you have a GCP
project with Cloud Build enabled. If needed, consult:
* the [GCP quickstarts][gcp-quickstarts] to setup a GCP project
* the [cloud build quickstarts][cloud-build-quickstarts] to setup Cloud Build
  in your project

This guide shows you how to test the builder by using it to create a local
container. These steps will require you to have [docker] and the
[pack tool][pack-install] on your workstation.

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

## Create the buildpacks builder

Set the `GOOGLE_CLOUD_PROJECT` shell variable to the project id of your GCP
project, and schedule a build to create your buildpack:

```shell
GOOGLE_CLOUD_PROJECT=... # put the right value here
gcloud builds submit \
    "--project=${GOOGLE_CLOUD_PROJECT}" \
    "--config=build_scripts/pack/cloudbuild.yaml"
```

## The test function

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

  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload(greeting());
}
```

## Building a Docker image

Build a Docker image from your function using this buildpack:

```shell
pack build \
    --builder gcr.io/${GOOGLE_CLOUD_PROJECT}/functions-framework-cpp/builder:bionic \
    --env FUNCTION_SIGNATURE_TYPE=http \
    --env TARGET_FUNCTION=hello_world_http \
    --path examples/site/hello_world_http \
    gcf-cpp-hello-world-http
```

To avoid typing the long builder name you can make this your default
builder:

```shell
pack config default-builder \
    gcr.io/${GOOGLE_CLOUD_PROJECT}/functions-framework-cpp/builder:bionic
```

Then the command becomes:

```shell
pack build \
    --env FUNCTION_SIGNATURE_TYPE=http \
    --env TARGET_FUNCTION=hello_world_http \
    --path examples/site/hello_world_http \
    gcf-cpp-hello-world-http
```

## Cleanup

Delete the images created by Cloud Build:

```shell
for image in \
    gcr.io/${GOOGLE_CLOUD_PROJECT}/functions-framework-cpp/{builder,cache,build-image,run-image}; do
  gcloud container images list-tags "${image}" --format='get(digest)' | \
    xargs printf "${image}@%s\n" | \
    xargs gcloud container images delete --force-delete-tags
done
```

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[cloud-build-quickstarts]: https://cloud.google.com/build/docs/quickstarts
[gcp-quickstarts]: https://cloud.google.com/resource-manager/docs/creating-managing-projects
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[hello-world-http]: /examples/site/hello_world_http/hello_world_http.cc
[cloud-build]: https://cloud.google.com/cloud-build
