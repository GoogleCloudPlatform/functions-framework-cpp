# How-to Guide: Deploy your function to Cloud Run

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[howto-create-container]: /examples/site/howto_create_container/README.md
[cloud-run-quickstarts]: https://cloud.google.com/run/docs/quickstarts
[gcp-quickstarts]: https://cloud.google.com/resource-manager/docs/creating-managing-projects
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[hello-world-http]: /examples/site/hello_world_http/hello_world_http.cc

## Pre-requisites

This guide assumes you are familiar with Google Cloud, and that you have a GCP
project with Cloud Run enabled. If needed, consult:
* the [GCP quickstarts][gcp-quickstarts] to setup a GCP project
* the [cloud run quickstarts][cloud-run-quickstarts] to setup Cloud Run in your
  project

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
docker build -t gcf-cpp-run-image --target gcf-cpp-runtime -f build_scripts/Dockerfile build_scripts
docker build -t gcf-cpp-build-image --target gcf-cpp-develop -f build_scripts/Dockerfile .
pack builder create gcf-cpp-builder:bionic --config pack/builder.toml
pack config trusted-builders add gcf-cpp-builder:bionic
pack config default-builder gcf-cpp-builder:bionic
```

## Building a Docker image

Set the `GOOGLE_CLOUD_PROJECT` shell variable to the project id of your GCP
project, and create a docker image with your function:

```shell
GOOGLE_CLOUD_PROJECT=... # put the right value here
pack build \
   --builder gcf-cpp-builder:bionic \
   --env FUNCTION_SIGNATURE_TYPE=http \
   --env TARGET_FUNCTION=hello_world_http \
   --path examples/site/hello_world_http \
   "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http"
```

Push this new container image to Google Container Registry:

```shell
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http:latest"
```

## Deploy to Cloud Run

To deploy this image in Cloud Run use this command. You need to select
a Cloud Run region for your deployment. We will use `us-central1` in this
guide:

```sh
gcloud run deploy gcf-cpp-hello-world-http \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

## Send a request to your function

Find out what URL was assigned to your function, and use `curl` to send a request:

```shell
HTTP_SERVICE_URL=$(gcloud run services describe \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --platform="managed" \
    --region="us-central1" \
    --format="value(status.url)" \
    gcf-cpp-hello-world-http)

curl -H "Authorization: Bearer $(gcloud auth print-identity-token)" "${HTTP_SERVICE_URL}"
# Output: Hello World!
```

## Cleanup

Delete the Cloud Run deployment:

```sh
gcloud run services delete gcf-cpp-hello-world-http \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --region="us-central1" \
    --platform="managed"
```

And the container image:

```shell
gcloud container images delete \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http:latest"
```
