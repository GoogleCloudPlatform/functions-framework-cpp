# Configuring the CI Environment

We use several Google Cloud Platform services to perform integration tests.
These notes describe how to setup a project to run these integration tests.
The main audience for these notes are developers working to improve the
Functions Framework for C++. They assume the reader is familiar with GCP and
with the Google Cloud SDK command-line tool.

## Pre-requisites

These notes assume you have setup a valid GCP project, with billing enabled.

> :warning: most of these services will incur billing costs, probably in the
> hundreds of dollars per month. Do not run these steps unless you understand
> these costs.

In these notes we will use `GOOGLE_CLOUD_PROJECT=...` as a shell variable with
the name of the project you want to use.

## Enable the Services

```sh
gcloud services enable --project="${GOOGLE_CLOUD_PROJECT}" \
    bigtable.googleapis.com \
    cloudbuild.googleapis.com \
    cloudfunctions.googleapis.com \
    container.googleapis.com \
    containerregistry.googleapis.com \
    eventarc.googleapis.com \
    logging.googleapis.com \
    pubsub.googleapis.com \
    run.googleapis.com \
    spanner.googleapis.com
```

## Create Pub/Sub Topics

```sh
gcloud pubsub topics create testing \
  --project="${GOOGLE_CLOUD_PROJECT}"
gcloud pubsub topics create gcs-changes \
  --project="${GOOGLE_CLOUD_PROJECT}"
```

## Create a Bucket

```sh
gsutil mb -p "${GOOGLE_CLOUD_PROJECT}" \
    "gs://${GOOGLE_CLOUD_PROJECT}-testing-bucket"
```

### Grant the GCS service permissions to publish to Pub/Sub

Get the GCS service account:

```shell
curl -X GET -H "Authorization: Bearer $(gcloud auth print-access-token)" \
  "https://storage.googleapis.com/storage/v1/projects/${GOOGLE_CLOUD_PROJECT}/serviceAccount"
GCS_SA="... @gs-project-accounts.iam.gserviceaccount.com"
```

Add this service account to the topic:Grant the 

```shell
gcloud pubsub topics add-iam-policy-binding \
  --project="${GOOGLE_CLOUD_PROJECT}" \
  --role="roles/pubsub.publisher" \
  --member="serviceAccount:${GCS_SA}" \
  gcs-changes
```

### Configure the Bucket to notify changes via Pub/Sub

```sh
gsutil notification create -f json \
  -t "projects/${GOOGLE_CLOUD_PROJECT}/topics/gcs-changes" \
  "gs://${GOOGLE_CLOUD_PROJECT}-testing-bucket"
```

### Create a service account for EventARC

```sh
readonly SA_ID="eventarc-trigger-sa"
readonly SA_NAME="${SA_ID}@${GOOGLE_CLOUD_PROJECT}.iam.gserviceaccount.com"

gcloud iam service-accounts create "${SA_ID}" \
    "--project=${GOOGLE_CLOUD_PROJECT}" \
    --description="Event Arg Triggers"
```

### Grant this SA permissions to invoke Cloud Run

```sh
gcloud projects add-iam-policy-binding "${GOOGLE_CLOUD_PROJECT}" \
    "--member=serviceAccount:${SA_NAME}" \
    "--role=roles/run.invoker"
```


## Verify the Resources

Create the buildpack builder:

```sh
cd functions-framework-cpp
docker build -t gcf-cpp-develop -f build_scripts/Dockerfile .
docker build -t gcf-cpp-runtime --target gcf-cpp-runtime -f build_scripts/Dockerfile build_scripts
pack create-builder gcf-cpp-builder:bionic --config pack/builder.toml
pack trust-builder gcf-cpp-builder:bionic
pack set-default-builder gcf-cpp-builder:bionic
```

Create containers for the Hello World examples:

```sh
pack build -v \
    --env FUNCTION_SIGNATURE_TYPE=cloudevent \
    --env TARGET_FUNCTION=hello_world_pubsub \
    --path examples/site/hello_world_pubsub \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-pubsub"
pack build -v \
    --env FUNCTION_SIGNATURE_TYPE=cloudevent \
    --env TARGET_FUNCTION=hello_world_storage \
    --path examples/site/hello_world_storage \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage"
pack build -v \
    --env TARGET_FUNCTION=hello_world_http \
    --path examples/site/hello_world_http \
    "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-http"
```

Push these containers to Google Container Registry:

```sh
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-pubsub:latest"
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage:latest"
docker push "gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-http:latest"
```

### Verify HTTP Configuration

Create a Cloud Run deployment running the hello world example:

```sh
gcloud run deploy gcf-hello-world-http \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-http:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

Test it by sending a request with `curl(1)`:

```sh
HTTP_SERVICE_URL=$(gcloud run services describe \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --platform="managed" \
    --region="us-central1" \
    --format="value(status.url)" \
    gcf-hello-world-http)

curl "${HTTP_SERVICE_URL}"
```


### Verify Pub/Sub Configuration

Create a Cloud Run deployment running the hello world example, and setup the triggers:

```sh
gcloud run deploy gcf-hello-world-pubsub \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-pubsub:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

Create a Pub/Sub trigger for this deployment:

```sh
PROJECT_NUMBER="$(gcloud projects list --filter="PROJECT_ID=${GOOGLE_CLOUD_PROJECT}" --format="value(project_number)")"
gcloud beta eventarc triggers create gcf-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --destination-run-service="gcf-hello-world-pubsub" \
    --destination-run-region="us-central1" \
    --matching-criteria="type=google.cloud.pubsub.topic.v1.messagePublished"
```

Test by sending a message to the right topic:

```sh
TOPIC=$(gcloud beta eventarc triggers describe gcf-hello-world-pubsub-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --format="value(transport.pubsub.topic)")
NONCE=$(date +%s)-${RANDOM}
gcloud pubsub topics publish "${TOPIC}" --message="Event ${NONCE}"
```

And then verify this message shows up in the log:

```sh
gcloud logging read \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --format="value(textPayload)" \
    "resource.type=cloud_run_revision AND resource.labels.service_name=gcf-hello-world-pubsub AND logName:stdout"
```

### Verify Storage Configuration

Create a Cloud Run deployment running the hello world example, and setup the triggers:

```sh
gcloud run deploy gcf-hello-world-storage \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --image="gcr.io/${GOOGLE_CLOUD_PROJECT}/gcf-hello-world-storage:latest" \
    --region="us-central1" \
    --platform="managed" \
    --allow-unauthenticated
```

Create a trigger for Cloud Storage events:

```sh
gcloud beta eventarc triggers create gcf-hello-world-storage-trigger \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --location="us-central1" \
    --destination-run-service="gcf-hello-world-storage" \
    --destination-run-region="us-central1" \
    --destination-run-region="us-central1" \
    --transport-topic="gcs-changes" \
    --matching-criteria="type=google.cloud.pubsub.topic.v1.messagePublished"
```

And then verify this message shows up in the log:

```sh
gcloud logging read \
    --project="${GOOGLE_CLOUD_PROJECT}" \
    --format="value(textPayload)" \
    "resource.type=cloud_run_revision AND resource.labels.service_name=gcf-hello-world-storage AND logName:stdout"
```

## Run the Cloud Build

Finally verify this works by running the Cloud Build 

```sh
gcloud builds submit \
    "--project=${GOOGLE_CLOUD_PROJECT}" \
    "--substitutions=SHORT_SHA=$(git rev-parse --short HEAD)" \
    "--config=ci/build-examples.yaml"
```
