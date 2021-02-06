# How-to Guide: Testing Event-driven Functions (Pub/Sub triggered)

[buildpacks]: https://buildpacks.io
[boost-log-gh]: https://github.com/boostorg/log
[/examples/site/hello_world_pubsub/hello_world_pubsub.cc]: /examples/site/hello_world_pubsub/hello_world_pubsub.cc
[pubsub_unit_test.cc]: pubsub_unit_test.cc
[pubsub_integration_server.cc]: pubsub_integration_server.cc
[pubsub_integration_test.cc]: pubsub_integration_test.cc
[quickstart-guide]: /google/cloud/functions/quickstart/README.md
[pubsub-quickstart]: https://cloud.google.com/pubsub/docs/quickstart-console

Event-driven functions do not return values, their only observable behavior are
side-effects. All tests, therefore, are looking at the side-effects of these
functions. Depending on where the function is deployed this might be more or
less involved.

## Function under Test

We will use this function throughput this guide:

[/examples/site/hello_world_pubsub/hello_world_pubsub.cc]
```cc
namespace gcf = ::google::cloud::functions;

// Use Boost.Archive to decode Pub/Sub message payload
std::string decode_base64(std::string const& base64);

void hello_world_pubsub(gcf::CloudEvent event) {
  if (event.data_content_type().value_or("") != "application/json") {
    std::cerr << "Error: expected application/json data\n";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto name = decode_base64(payload["message"]["data"].get<std::string>());
  BOOST_LOG_TRIVIAL(info) << "Hello " << (name.empty() ? "World" : name);
}
```

## Writing a Unit Test

This function uses the [Boost.Log][boost-log-gh] library to facilitate unit
testing. Logging can be captured and examined using the usual testing
assertions, see [pubsub_unit_test.cc] for more details.

## Writing an Integration Test

To write an integration test we first create a local server to run the
function, as shown in [pubsub_integration_server.cc], then we launch this
server from within the [integration test][pubsub_integration_test.cc] and
examine its result.

## Writing a System Test

### Pre-requisites

This guide assumes that you have:

* a working Pub/Sub topic, see the [Pub/Sub quickstart guide][pubsub-quickstart]
  for more information,
* set the `TOPIC_ID` shell variable to the id of this topic, e.g.
  ```shell
  TOPIC_ID=my-testing-topic
  ```
* as part of the Pub/Sub quickstart guide you would have identified a GCP
  project to run your tests, set the `GOOGLE_CLOUD_PROJECT` shell variable
  to the id of his project, for example:
  ```shell
  GOOGLE_CLOUD_PROJECT=my-testing-project
  ``` 
* you need to create a [buildpacks builder][buildpacks] that can create
  Docker images from your C++ functions, see the
  [quickstart guide][quickstart-guide] for more information,

```shell
pack build pack build \
    --builder gcf-cpp-builder:bionic \
    --env "FUNCTION_SIGNATURE_TYPE=cloudevent" \
    --env "TARGET_FUNCTION=hello_world_pubsub" \
    --path "examples/site/hello_world_pubsub" \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/hello_world_pubsub"
```

Deploy this function to Cloud Run:

```shell
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/hello_world_pubsub"
gcloud run deploy gcf-hello-world-pubsub \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-pubsub:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

Setup a Pub/Sub trigger:

```shell
gcloud beta eventarc triggers create gcf-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --destination-run-service="gcf-hello-world-pubsub" \
    --destination-run-region="us-central1" \
    --matching-criteria="type=google.cloud.pubsub.topic.v1.messagePublished" \
    --transport-topic="${TOPIC_ID}"
```

Finally, you can run the system test, using environment variables to pass
any configuration parameters:

```shell
env "GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}" \
    "TOPIC_ID=${TOPIC_ID}" \
    "SERVICE_ID=gcf-hello-world-pubsub" \
    ./pubsub_system_test # use actual path to binary
```
