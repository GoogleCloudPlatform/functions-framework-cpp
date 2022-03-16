# How-to Guide: Testing Event-driven Functions

Event-driven functions do not return values, therefore, their only observable
behavior are their side-effects and any testing for them is based in observing
these side-effects. In this example we will examine the log of a event-driven
function to test it. Depending on where the function is deployed this might be
more or less involved.

## Function under Test

We will use this [function][snippet source] throughout this guide:

<!-- inject-snippet-start -->
[snippet source]: /examples/site/hello_world_storage/hello_world_storage.cc
```cc
#include <google/cloud/functions/cloud_event.h>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

void hello_world_storage(gcf::CloudEvent event) {
  if (event.data_content_type().value_or("") != "application/json") {
    BOOST_LOG_TRIVIAL(error) << "expected application/json data";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  BOOST_LOG_TRIVIAL(info) << "Event: " << event.id();
  BOOST_LOG_TRIVIAL(info) << "Event Type: " << event.type();
  BOOST_LOG_TRIVIAL(info) << "Bucket: " << payload.value("bucket", "");
  BOOST_LOG_TRIVIAL(info) << "Object: " << payload.value("name", "");
  BOOST_LOG_TRIVIAL(info) << "Metageneration: "
                          << payload.value("metageneration", "");
  BOOST_LOG_TRIVIAL(info) << "Created: " << payload.value("timeCreated", "");
  BOOST_LOG_TRIVIAL(info) << "Updated: " << payload.value("updated", "");
}
```
<!-- inject-snippet-end -->

This test receives storage events and logs a few of the fields in these
events.

## Writing a Unit Test

This function uses the [Boost.Log][boost-log-gh] library to facilitate unit
testing. Logging can be captured and examined using the usual testing
assertions. See [storage_unit_test.cc] for more details.

## Writing an Integration Test

To write an integration test we first create a local server to run the
function, as shown in [storage_integration_server.cc], then we launch this
server from within the [integration test][storage_integration_test.cc] and
examine its result.

## Writing a System Test

### Pre-requisites

This guide assumes you are familiar with deploying C++ functions to Cloud Run.
If necessary consult the [Howto Guide][container-guide] for more information.
We will create a container for the storage "hello world" function as usual:

```shell
pack build \
    --builder gcr.io/buildpacks/builder:latest \
    --env "GOOGLE_FUNCTION_SIGNATURE_TYPE=cloudevent" \
    --env "GOOGLE_FUNCTION_TARGET=hello_world_storage" \
    --path "examples/site/hello_world_storage" \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage"
```

Then deploy this function to Cloud Run:

```shell
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage"
gcloud run deploy gcf-hello-world-storage \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

If needed, create a Google Cloud Storage bucket for these tests:

```shell
BUCKET_NAME=... # assign a valid bucket name
gsutil mb -p "${GOOGLE_CLOUD_PROJECT}" "gs://${BUCKET_NAME}"
```

Then change the bucket to publish all changes to a Cloud Pub/Sub topic. This
command will create the topic if needed, and assign the right permissions:

```shell
TOPIC_ID=... # set this variable to a valid topic id.
gsutil notification create -f json -t "${TOPIC_ID}" "gs://${BUCKET_NAME}"
```

Setup a Cloud Pub/Sub trigger for your function when messages appear on this
topic:

```shell
gcloud beta eventarc triggers create gcf-hello-world-storage-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --destination-run-service="gcf-hello-world-storage" \
    --destination-run-region="us-central1" \
    --matching-criteria="type=google.cloud.pubsub.topic.v1.messagePublished" \
    --transport-topic="${TOPIC_ID}"
```

Finally, you can run the system test, using environment variables to pass
any configuration parameters:

```shell
env "GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}" \
    "BUCKET_NAME=${BUCKET_NAME}" \
    "SERVICE_ID=gcf-hello-world-storage" \
    ./pubsub_system_test # use actual path to binary
```

## Cleanup

Remember to remove your deployment and the image once you have finished:

```sh
gcloud run services delete gcf-cpp-hello-world-storage \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --region="us-central1" \
    --platform="managed"
gcloud container images delete \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-cpp-hello-world-storage:latest"
```

[buildpacks]: https://buildpacks.io
[boost-log-gh]: https://github.com/boostorg/log
[storage_unit_test.cc]: storage_unit_test.cc
[storage_integration_server.cc]: storage_integration_server.cc
[storage_integration_test.cc]: storage_integration_test.cc
[quickstart-guide]: /examples/site/howto_local_development/README.md
[container-guide]: /examples/site/howto_create_container/README.md
[pubsub-quickstart]: https://cloud.google.com/pubsub/docs/quickstart-console
