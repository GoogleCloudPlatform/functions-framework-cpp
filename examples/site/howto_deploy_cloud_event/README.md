# How-to Guide: Deploy a C++ Pub/Sub function to Cloud Run

[repository-gh]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[howto-create-container]: /examples/site/howto_create_container/README.md
[cloud-run-quickstarts]: https://cloud.google.com/run/docs/quickstarts
[gcp-quickstarts]: https://cloud.google.com/resource-manager/docs/creating-managing-projects
[buildpacks]: https://buildpacks.io
[docker]: https://docker.com/
[docker-install]: https://store.docker.com/search?type=edition&offering=community
[sudoless docker]: https://docs.docker.com/engine/install/linux-postinstall/
[pack-install]: https://buildpacks.io/docs/install-pack/
[hello-world-pubsub]: /examples/site/hello_world_pubsub/hello_world_pubsub.cc
[gcloud-eventarc-create]: https://cloud.google.com/sdk/gcloud/reference/beta/eventarc/triggers/create

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

In this guide we will be using the [Pub/Sub hello word][hello-world-pubsub] function:

```cc
namespace gcf = ::google::cloud::functions;

// Use Boost.Archive to decode Pub/Sub message payload
std::string decode_base64(std::string const& base64);

void hello_world_pubsub(gcf::CloudEvent event) {  // NOLINT
  if (event.data_content_type().value_or("") != "application/json") {
    std::cerr << "Error: expected application/json data\n";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto name = decode_base64(payload["message"]["data"].get<std::string>());
  BOOST_LOG_TRIVIAL(info) << "Hello " << (name.empty() ? "World" : name);
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

Set the `GOOGLE_CLOUD_PROJECT` shell variable to the project id of your GCP
project, and create a docker image with your function:

```shell
GOOGLE_CLOUD_PROJECT=... # put the right value here
pack build \
   --builder gcf-cpp-builder:bionic \
   --env FUNCTION_SIGNATURE_TYPE=cloudevent \
   --env TARGET_FUNCTION=hello_world_pubsub \
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
function in the correct format. For simplicity, this guide will create create a
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
