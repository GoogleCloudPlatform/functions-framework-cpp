# How-to Guide: Local Development

[vcpkg-gh]: https://github.com/microsoft/vcpkg
[vcpkg-install]: https://github.com/microsoft/vcpkg#getting-started

This guide describes how to compile and run a function locally. This can be
useful when writing unit test, or to accelerate the edit -> compile -> test
cycle.

## Installing Dependencies

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

## Compiling a function

Once vcpkg is compiled you can build the example using:

> :warning: the first time you build the framework it might take several
> minutes, maybe as much as 1/2 hour, depending on your workstation
> performance. Future builds, even in different directories, should be
> faster as `vcpkg` caches binary artifacts in `$HOME/.cache/vcpkg`.

```shell
cd $HOME/functions-framework-cpp/examples/site/howto_local_development
cmake -H. -B.build -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .build
```

## Running the function locally

This will produce a standalone HTTP server, which you can run locally using:

```shell 
.build/local_server --port 8080
```

Test this program using `curl`. In a separate terminal window run:

```shell
curl http://localhost:8080
```

And terminate the program gracefully using:

```shell
curl http://localhost:8080/quit/program/0
```
