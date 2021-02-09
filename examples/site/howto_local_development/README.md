# How-to Guide: Local Development

[vcpkg-gh]: https://github.com/microsoft/vcpkg
[vcpkg-install]: https://github.com/microsoft/vcpkg#getting-started
[cmake]: https://cmake.org
[cmake-install]: https://cmake.org/install/

This guide describes how to compile and run a function locally. This can be
useful when writing unit test, or to accelerate the edit -> compile -> test
cycle.

## Installing Dependencies

Because the Functions Framework for C++ uses C++17, you will need a working C++
compiler with support for C++17. If you are a GCC user, any version after 8.0
should work. For Clang any version after 6.0 should work. We have not tested
these instructions with MSVC.

This guide uses [CMake (>= 3.5)][cmake] as a build tool. There are detailed
[install instructions][cmake-install], but many system package managers have
packages for it. Verify your CMake tool version:

```shell
cmake --version
# Output: cmake version X.Y.Z ... verify this is >= 3.5
```

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
cmake --build .build -- -j $(nproc)
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
