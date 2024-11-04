# How-to Guide: Deploy your function to Cloud Run

This guide shows how to deploy a C++ function handling HTTP requests to
[Cloud Run].

> **WARNING:** The development version of this document may not work with the
> released version of the functions framework. Please use this document as it
> appears in the [corresponding release][github-releases] if you are using a
> released version of the library. In particular, buildpacks use the latest
> release.

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

Set the `GOOGLE_CLOUD_PROJECT` shell variable to the project id of your GCP
project, and use the [Google Cloud buildpack] builder to create a docker image
containing your function:

```shell
GOOGLE_CLOUD_PROJECT=... # put the right value here
pack build \
    --builder gcr.io/buildpacks/builder:v1 \
    --env GOOGLE_FUNCTION_TARGET=hello_world_http \
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

Set the active project:

```sh
gcloud config set project ${GOOGLE_CLOUD_PROJECT}
```


```sh
gcloud run deploy gcf-cpp-hello-world-http \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

## Send a request to your function

Find out what URL was assigned to your function, and use `curl` to send a request:

```shell
HTTP_SERVICE_URL=$(gcloud run services describe \
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
    --region="us-central1" \
    --platform="managed"
```

And the container image:

```shell
gcloud container images delete \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-http:latest"
```

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[howto-create-container]: /examples/site/howto_create_container/README.md
[cloud-run-quickstarts]: https://cloud.google.com/run/docs/quickstarts
[gcp-quickstarts]: https://cloud.google.com/resource-manager/docs/creating-managing-projects
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[Cloud Run]: https://cloud.google.com/run
[github-releases]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/releases
