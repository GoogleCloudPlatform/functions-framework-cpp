# How-to Guide: Deploy a C++ Pub/Sub function to Cloud Run

This guide shows how to deploy a C++ function consuming cloud events to
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
[snippet source]: /examples/site/hello_world_pubsub/hello_world_pubsub.cc
```cc
#include <google/cloud/functions/function.h>
#include <boost/log/trivial.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

void hello_world_pubsub_impl(gcf::CloudEvent const& event) {
  if (event.data_content_type().value_or("") != "application/json") {
    BOOST_LOG_TRIVIAL(error) << "expected application/json data";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto const name = cppcodec::base64_rfc4648::decode<std::string>(
      payload["message"]["data"].get<std::string>());
  BOOST_LOG_TRIVIAL(info) << "Hello " << (name.empty() ? "World" : name);
}

gcf::Function hello_world_pubsub() {
  return gcf::MakeFunction(hello_world_pubsub_impl);
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
    --env GOOGLE_FUNCTION_TARGET=hello_world_pubsub \
    --path examples/site/hello_world_pubsub \
   "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-pubsub"
```

Push this new container image to Google Container Registry:

```shell
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-pubsub:latest"
```

## Deploy to Cloud Run

To deploy this image in Cloud Run use this command. You need to select
a Cloud Run region for your deployment. We will use `us-central1` in this
guide:

```shell
gcloud run deploy gcf-cpp-hello-world-pubsub \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-pubsub:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

Verify unauthenticated requests are allowed:

```shell
gcloud run services get-iam-policy gcf-cpp-hello-world-pubsub \
     --project="${GOOGLE_CLOUD_PROJECT}" \
     --region="us-central1" \
     --platform="managed"
```

The result should include `allUsers`.

> :warning: It is possible your organization has disabled unauthenticated
> requests to Cloud Run. If this is the case the rest of this guide will not
> work, as `eventarc` only supports unauthenticated connections at this time.

## Setup a Pub/Sub trigger

An eventarc trigger receives events from Pub/Sub and forward them to your
function in the correct format. For simplicity, this guide will create a
new topic as part of setting up the trigger. If you prefer to use an existing
topic read about the the `--topic-transport` option in the
[gcloud documentation][gcloud-eventarc-create]. Once you have decided how to
proceed run this command (maybe with an addition `--topic-transport` option):

```shell
gcloud beta eventarc triggers create gcf-cpp-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --destination-run-service="gcf-cpp-hello-world-pubsub" \
    --destination-run-region="us-central1" \
    --matching-criteria="type=google.cloud.pubsub.topic.v1.messagePublished"
```

Find out what topic is used to this new trigger:

```shell
TOPIC=$(gcloud beta eventarc triggers describe gcf-cpp-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --format="value(transport.pubsub.topic)")
```

## Publish a Pub/Sub and verify your function receives it

Use the following `gcloud` to publish a message

```shell
gcloud pubsub topics publish "${TOPIC}" --message="Event"
```

CloudEvent functions produce no responses, but you can examine their log
to verify the Pub/Sub message was received:

```shell
gcloud logging read \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --format="value(textPayload)" \
    "resource.type=cloud_run_revision AND resource.labels.service_name=gcf-cpp-hello-world-pubsub AND logName:stdout"
# Output: Hello Event
```

## Cleanup

Delete the trigger:

```shell
gcloud beta eventarc triggers delete gcf-cpp-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1"
```

Delete the Cloud Run deployment:

```sh
gcloud run services delete gcf-cpp-hello-world-pubsub \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --region="us-central1" \
    --platform="managed"
```

And the container image:

```shell
gcloud container images delete \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-pubsub:latest"
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
[gcloud-eventarc-create]: https://cloud.google.com/sdk/gcloud/reference/beta/eventarc/triggers/create
[Google Cloud buildpack]: https://github.com/GoogleCloudPlatform/buildpacks
[Cloud Run]: https://cloud.google.com/run
[github-releases]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/releases
