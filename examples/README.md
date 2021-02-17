# Functions Framework Examples

This directory contains a series of examples showing how to use the Functions Framework.

A good place to start is [hello_world](hello_world). It contains a simple example, with a simple function in a single
`.cc` file.

## Create the Development and Runtime Docker Images

To compile the examples you will need a Docker image with the development tools and core dependencies pre-compiled.
To create this image run this command:

```sh
docker build -t gcf-cpp-build-image --target gcf-cpp-develop -f build_scripts/Dockerfile .
```

The runtime image is contains just the minimal components to execute a program using the framework:

```sh
docker build -t gcf-cpp-run-image --target gcf-cpp-runtime -f build_scripts/Dockerfile build_scripts
```

## Create the buildpack builder

We use [buildpacks](https://buildpacks.io) to compile the functions into runnable Docker images. First create a builder:

```sh
pack builder create gcf-cpp-builder:bionic --config pack/builder.toml
pack config trusted-builders add gcf-cpp-builder:bionic
```

To avoid using the `--builder gcf-cpp-builder:bionic` option in each command we make this builder the default:

```sh
pack config default-builder gcf-cpp-builder:bionic
```

## Creating a Docker image for the examples

Compile any HTTP example using:

```sh
pack build "my-image" \
  --env TARGET_FUNCTION=$EXAMPLE_FUNCTION_NAME \
  --env FUNCTION_SIGNATURE_TYPE=http \
  --path examples/$EXAMPLE_DIRECTORY
```

for example:

```sh
pack build "hello-world" \
  --env TARGET_FUNCTION=HelloWorld \
  --env FUNCTION_SIGNATURE_TYPE=http \
  --path examples/hello_world
```

and then run the image as usual:

```sh
ID=$(docker run --detach --publish 8080:8080 hello-world)
curl http://localhost:8080/
docker kill ${ID}
```

## Deploying to Cloud Run

This example assumes that `GOOGLE_CLOUD_PROJECT` is set to a GCP project with the right services enabled:

```sh
GOOGLE_CLOUD_PROJECT=...          # use a real project
GOOGLE_CLOUD_REGION=us-central1   # use a different region if desired 
```

Create the Docker image, with a tag suitable for deployment to Cloud Run:

```sh
pack build "gcr.io/${GOOGLE_CLOUD_PROJECT}/hello-world" \
  --env TARGET_FUNCTION=HelloWorld \
  --env FUNCTION_SIGNATURE_TYPE=http \
  --path examples/hello_world
```

Push this image to Google Container Registry:

```sh
docker push gcr.io/${GOOGLE_CLOUD_PROJECT}/hello-world:latest
```

Deploy this application to Cloud Run:

```sh
gcloud run deploy gcf-cpp-hello \
    "--project=${GOOGLE_CLOUD_PROJECT}" \
    "--image=gcr.io/${GOOGLE_CLOUD_PROJECT}/hello-world:latest" \
    "--region=${GOOGLE_CLOUD_REGION}" \
    "--platform=managed" \
    "--no-allow-unauthenticated"
```

Fetch the service URL:

```bash
SERVICE_URL=$(gcloud run services list \
    "--project=${GOOGLE_CLOUD_PROJECT}" \
    "--platform=managed" \
    '--format=csv[no-heading](URL)' \
    "--filter=SERVICE:gcf-cpp-hello")
```

Test by sending a request using `curl`:

```bash
curl -H "Authorization: Bearer $(gcloud auth print-identity-token)" "${SERVICE_URL}"
```
