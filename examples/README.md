# Functions Framework Examples

This directory contains a series of examples showing how to use the Functions Framework.

A good place to start is [hello_world](hello_world). It contains a simple example, with a simple function in a single
`.cc` file.

## Create the Development Image

To compile the examples you will need a Docker image with the development tools and core dependencies pre-compiled.
To create this image run this command:

```sh
docker build -t gcf-cpp-develop -f build_scripts/Dockerfile build_scripts
```

## Creating a Docker image for the examples

The examples can run in Docker containers. Using the development image from above you can create a docker image for any
of the examples using:

```sh
docker build  -t my-image -f build_scripts/build-application.Dockerfile \
  --build-arg=TARGET_FUNCTION=$EXAMPLE_FUNCTION_NAME \
  examples/$EXAMPLE_DIRECTORY
```

for example:

```sh
docker build -t hello-world -f build_scripts/build-application.Dockerfile \
  --build-arg=TARGET_FUNCTION=HelloWorld \
  examples/hello_world
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
docker build -t gcr.io/${GOOGLE_CLOUD_PROJECT}/hello-world:latest \
  -f build_scripts/build-application.Dockerfile \
  --build-arg=TARGET_FUNCTION=HelloWorld \
  examples/hello_world
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
