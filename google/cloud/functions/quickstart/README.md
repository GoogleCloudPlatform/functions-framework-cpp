# Quickstart with Functions Framework for C++

[vcpkg-gh]: https://github.com/microsoft/vcpkg
[vcpkg-install]: https://github.com/microsoft/vcpkg#getting-started

This directory contains a small example showing how to start using the
Functions Framework for C++. The example can be compiled as a standalone local
server, or as a container to be deployed in Cloud Run or a similar environment.

## How-to Guide: Local Development

### Installing Dependencies

> :warning: `functions-framework-cpp` is not published in the main branch
> for vcpkg yet. Use this branch:
>
> https://github.com/coryan/vcpkg/tree/new-port-functions-framework-cpp

The Functions Framework for C++ recommends using [vcpkg][vcpkg-gh] to manage
dependencies. This is how dependencies will be installed and compiled in the
production environment, so you probably want to use the same approach in
development. Follow the [vcpkg install instructions][vcpkg-install] to get
vcpkg installed on your development environment. For example, on Linux you
would use:

```shell
cd $HOME
git clone https://github.com/microsoft/vcpkg
(cd vcpkg && ./bootstrap.sh)
```

### Compiling a function

Once vcpkg is compiled you can build the quickstart example using:

> :warning: the first time you build the framework it might take several
> minutes, maybe as much as 1/2 hour, depending on your workstation
> performance. Future builds, even in different directories, should be
> faster as `vcpkg` caches binary artifacts in `$HOME/.cache/vcpkg`.

```shell
cd $HOME/functions-framework-cpp/google/cloud/functions/quickstart
cmake -H. -B.build -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .build
```

### Running the function locally

This will produce a standalone HTTP server, which you can run locally using:

```shell 
.build/quickstart --port 8080
```

Test this program using `curl`. In a separate terminal window run:

```shell
curl http://localhost:8080
```

And terminate the program gracefully using:

```shell
curl http://localhost:8080/quit/program/0
```

## How-to Guide: Running your function as Docker container

1. Install [Docker](https://store.docker.com/search?type=edition&offering=community) and
   the [`pack` tool](https://buildpacks.io/docs/install-pack/).

1. Create the `pack` builder for C++. The first time your run this it can take
   several minutes, maybe as long as an hour, depending on the capabilities of
   your workstation.
   ```shell
   cd $HOME/functions-framework-cpp
   docker build -t gcf-cpp-runtime --target gcf-cpp-runtime -f build_scripts/Dockerfile .
   docker build -t gcf-cpp-develop --target gcf-cpp-develop -f build_scripts/Dockerfile .
   pack create-builder gcf-cpp-builder:bionic --config pack/builder.toml
   pack trust-builder gcf-cpp-builder:bionic
   ```

1. Build a Docker image from your function using this buildpack:
   ```shell
   pack build \
       --builder gcf-cpp-builder:bionic \
       --env FUNCTION_SIGNATURE_TYPE=http \
       --env TARGET_FUNCTION=HelloWorld \
       --path google/cloud/functions/quickstart/hello_function \
       gcf-cpp-quickstart
   ```

1. Start a Docker container in the background with this image:
   ```shell
   ID=$(docker run --detach --rm -p 8080:8080 gcf-cpp-quickstart)
   ```

1. Send requests to this function using `curl`:
   ```shell
   curl http://localhost:8080
   # Output: Hello, World!
   ```

1. Stop the background container:
   ```shell
   docker kill "${ID}"
   ```
