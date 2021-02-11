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
git clone -q https://github.com/microsoft/vcpkg
# Expected output: none
(cd vcpkg && ./bootstrap-vcpkg.sh --disableMetrics --useSystemBinaries)
```

You should see output like this:

```console
Downloading vcpkg tool sources
...
..
Building vcpkg-tool...
...
..
[88/88] Linking CXX executable vcpkg
```

This will create a `vcpkg` executable in the `$HOME/vcpkg` directory.

## Compiling a function

Once vcpkg is compiled you can build the example. If you have not cloned
the `functions-framework-cpp` repository yet, you need to do so now:

```shell
cd $HOME
git clone -q https://github.com/GoogleCloudPlatform/functions-framework-cpp.git
# Expected output: none
```

Then go to the directory hosting this example and use CMake to build
the example:

```shell
cd functions-framework-cpp/examples/site/howto_local_development
```

Run the CMake configure step. If needed, this will download and build any
dependencies for your function:

```shell
cmake -H. -B.build -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
```

You should see output like this:

> :warning: the first time you use or build the framework it might take several
> minutes, maybe as much as 1/2 hour, depending on your workstation
> performance. Future builds, even in different directories, should be faster
> as `vcpkg` caches binary artifacts in `$HOME/.cache/vcpkg`.

```console
-- Running vcpkg install
...
-- Running vcpkg install - done
-- Configuring done
-- Generating done
```

If you have previously built the dependencies with vcpkg the output may include
informational messages about reusing these binary artifacts. With some versions
of CMake you may see a warning about your version of Boost, something like:

```console
CMake Warning at /usr/share/cmake-3.16/Modules/FindBoost.cmake:1161 (message):
       New Boost version may have incorrect or missing dependencies and imported
       targets
```

You can safely ignore this warning.

With the dependencies built and CMake configured you can now build the code:

```shell
cmake --build .build
```

This will output some informational messages about the build progress, and will
create a binary called `local_server` in the .build subdirectory.

## Running the function locally

This will produce a standalone HTTP server, which you can run locally using:

```shell 
.build/local_server --port 8080
```

Test this program using `curl`. In a separate terminal window run:

```shell
curl http://localhost:8080
```

You can just interrupt (`Ctrl-C`) the program to terminate it.
