# Functions Framework for C++

<!-- Repo links -->
[github-discussions]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/discussions
[github-issue]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/issues/new
[github-releases]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/releases
[github-repo]: https://github.com/GoogleCloudPlatform/functions-framework-cpp

<!-- Build Status links -->
[ff_cpp_unit_img]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/workflows/C++%20Unit%20CI/badge.svg
[ff_cpp_unit_link]:  https://github.com/GoogleCloudPlatform/functions-framework-cpp/actions?query=workflow%3A%22C%2B%2B+Unit+CI%22
[ff_cpp_lint_img]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/workflows/C%2B%2B%20Lint%20CI/badge.svg
[ff_cpp_lint_link]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/actions?query=workflow%3A%22C%2B%2B+Lint+CI%22
[ff_cpp_conformance_img]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/workflows/C++%20Conformance%20CI/badge.svg
[ff_cpp_conformance_link]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/actions?query=workflow%3A%22C%2B%2B+Conformance+CI%22

<!-- Reference links -->
[abseil-gh]: https://github.com/abseil/abseil-cpp
[boost-org]: https://boost.org/
[nlohmann-json-gh]: https://github.com/nlohmann/json
[google-cloud-cpp-gh]: https://github.com/googleapis/google-cloud-cpp
[fmt-gh]: https://github.com/fmtlib/fmt
[buildpacks]: https://github.com/GoogleCloudPlatform/buildpacks
[CloudEvents]: https://cloudevents.io/
[docs]: docs
[examples]: examples
[examples/hello_world/hello_world.cc]: examples/hello_world/hello_world.cc
[Google Cloud Run]:
https://cloud.google.com/run/docs/quickstarts/build-and-deploy
[Google App Engine]: https://cloud.google.com/appengine/docs/go/
[Google Cloud Functions]: https://cloud.google.com/functions/
[Knative]: https://github.com/knative/
[quickstart-local]: /examples/site/howto_local_development/README.md
[quickstart-container]: /examples/site/howto_create_container/README.md
[quickstart-cloud-run]: /examples/site/howto_deploy_to_cloud_run/README.md
[quickstart-pubsub]: /examples/site/howto_deploy_cloud_event/README.md

|Functions Framework|Unit Tests|Lint Test|Conformance Tests|
|---|---|---|---|
| [C++][github-repo] | [![][ff_cpp_unit_img]][ff_cpp_unit_link] | [![][ff_cpp_lint_img]][ff_cpp_lint_link] | [![][ff_cpp_conformance_img]][ff_cpp_conformance_link] |

An open source FaaS (Functions as a Service) framework for writing portable C++
functions -- brought to you by Google.

The Functions Framework lets you write lightweight functions that run in many
different environments, including:

- Your local development machine - see how to build and run a function in
  [local container][quickstart-container]
- [Google Cloud Run] - see how to [deploy functions][quickstart-cloud-run]
  to Cloud Run
- [Knative]-based environments

[Google Cloud Functions] does not currently provide an officially supported C++
language runtime, but we're working to make running on [Google Cloud Run] as
seamless and symmetric an experience as possible for your C++ Functions
Framework projects.

The framework allows you to go from:

[examples/hello_world/hello_world.cc]
```cc
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>

using ::google::cloud::functions::HttpRequest;
using ::google::cloud::functions::HttpResponse;

HttpResponse HelloWorld(HttpRequest) {  // NOLINT
  return HttpResponse{}
      .set_header("Content-Type", "text/plain")
      .set_payload("Hello World\n");
}
```

To:

```shell
curl https://<your-app-url>
# Output: Hello, World!
```

All without needing to worry about writing an HTTP server or request
handling logic.

See more demos under the [examples] directory.

## Features

- Build your Function in the same container environment used by Cloud Functions
  using [buildpacks]
- Invoke a function in response to a request
- Automatically unmarshal events conforming to the [CloudEvents] spec
- Portable between serverless platforms

## Versions and Status

This library is considered generally available. We do not expect breaking
changes to its public APIs. See [below](#public-api-and-api-breaking-changes)
for a description of what is included in the public API, and what parts of the
library may change without notice.

This library does **not** follow the [Semantic Versioning](https://semver.org)
conventions.

## Requirements

This is a C++17-based framework. It requires a compiler supporting C++17, we
routinely test with GCC (>= 8), and with Clang (>= 10), let us know if you
have problems with other compilers. The framework also depends on a number
of other libraries, note that these libraries may have their own dependencies:

| Library | Minimum version | Description |
| ------- | --------------: | ----------- |
| [Abseil][abseil-gh] | 20200923 | C++ components to augment the standard library |
| [Boost][boost-org] | 1.73 | Peer-reviewed portable C++ libraries |
| [nlohmann/json][nlohmann-json-gh] | 3.9.1 | JSON for Modern C++ |

The examples use additional libraries, note that these libraries may have their
own dependencies:

| Library | Minimum version | Description |
| ------- | --------------: | ----------- |
| [google-cloud-cpp][google-cloud-cpp-gh] | 1.23.0 | Google Cloud C++ Client Libraries |
| [{fmt}][fmt-gh] | 7.1.3 | A formatting library |

## Public API and API Breaking Changes

In general, we avoid making backwards incompatible changes to our C++ APIs (see
below for the definition of "API"). Sometimes such changes yield benefits to
our customers, in the form of better performance, easier-to-understand APIs,
and/or more consistent APIs across services. When these benefits warrant it, we
will announce these changes prominently in our `CHANGELOG.md` file and in the
affected release's notes. Nevertheless, though we take commercially reasonable
efforts to prevent this, it is possible that backwards incompatible changes go
undetected and, therefore, undocumented. We apologize if this is the case and
welcome feedback (or bug reports) to rectify the problem.

By "API" we mean the C++ API exposed by public header files in this repo. We
are not talking about the gRPC or REST APIs exposed by Google Cloud servers. We
are also talking only about A**P**I stability -- the ABI is subject to change
without notice. You should not assume that binary artifacts (e.g. static
libraries, shared objects, dynamically loaded libraries, object files) created
with one version of the library are usable with newer/older versions of the
library. The ABI may, and does, change on "minor revisions", and even patch
releases.

We request that our customers adhere to the following guidelines to avoid
accidentally depending on parts of the library we do not consider to be part of
the public API and therefore may change (including removal) without notice:

* You should only include headers matching the `google/cloud/functions/*.h`,
  or `google/cloud/functions/mock/*.h` patterns.
* You should **NOT** directly include headers in any subdirectories, such as
  `google/cloud/functions/internal`.
* The files *included from* our public headers are **not part of our public
  API**. Depending on indirect includes may break your build in the future, as
  we may change a header "foo.h" to stop including "bar.h" if "foo.h" no longer
  needs the symbols in "bar.h". To avoid having your code broken, you should
  directly include the public headers that define all the symbols you use (this
  is sometimes known as
  [include-what-you-use](https://include-what-you-use.org/)).
* Any file or symbol that lives within a directory or namespace containing
  "internal", "impl", "test", "detail", "benchmark", "sample", or "example", is
  explicitly **not part of our public API**.
* Any file or symbol with "Impl" or "impl" in its name is **not part of our
  public API**.

Previous versions of the library will remain available on the
[GitHub Releases page][github-releases].

## Beyond the C++ API

Applications developers interact with a C++ library through more than just
the C++ symbols and headers. They also need to reference the name of the
library in their build scripts. Depending of the build system they use
this may be a CMake target, a Bazel rule, a pkg-config module, or just the
name of some object in the file system.

As with the C++ API, we try to avoid breaking changes to these interface
points. Sometimes such changes yield benefits to our customers, in the form of
easier-to-understand what names go with services, or more consistency
across services. When these benefits warrant it, we will announce these changes
prominently in our `CHANGELOG.md` file and in the affected release's notes.
Nevertheless, though we take commercially reasonable efforts to prevent this,
it is possible that backwards incompatible changes go undetected and,
therefore, undocumented. We apologize if this is the case and welcome feedback
(or bug reports) to rectify the problem.

### Experimental Libraries

From time to time we add libraries to `functions-framework-cpp` to validate
new  designs, expose experimental (or otherwise not generally available)
features, or simply because a library is not yet complete. Such libraries
will have `experimental` in their CMake target and Bazel rule. The README
file for these libraries will also document that they are experimental.
Such libraries are subject to change, including removal, without notice.
This includes, but it is not limited to, all their symbols, pre-processor
macros, files, targets, rules, and installed artifacts.

### CMake targets and packages

Only CMake packages starting with the `functions_framework_cpp_` prefix are
intended for customer use. Only targets starting with
`functions-framework-cpp::` are intended for customer use. Experimental targets
have `experimental` in their name (e.g.
`functions-framework-cpp::experimental-foo`), as previously stated,
experimental targets are subject to change or removal without notice.

### pkg-config modules

Only modules starting with `functions_framework_cpp_` are intended for customer
use.

### Unsupported use cases

We try to provide stable names for the previously described mechanisms:

* CMake targets loaded via `find_package()`,
* pkg-config modules

It is certainly possible to use the library through other mechanisms,
and while these may work, we may accidentally break these from time to time.
Examples of such, and the recommended alternatives, include:

* CMake's `FetchContent` and/or git submodules: in these approaches the
  `functions-framework-cpp` library becomes a sub-directory of a larger CMake
  build. We do not test `functions-framework-cpp` in this configuration, and we
  find it brittle as **all** CMake targets become visible to the larger
  project. This is both prone to conflicts, and makes it impossible to enforce
  that some targets are only for testing or implementation. Applications may
  want to consider source package managers, such as `vcpkg`, or CMake super
  builds via `ExternalProject_Add()` as alternatives.

* Using library names directly: applications should not use the
  library names, e.g., by using `-lfunctions_framework_cpp` in build scripts. We
  may need to split or merge libraries over time, making such names unstable.
  Applications should use CMake targets, e.g.,
  `functions-framework-cpp::framework`, or pkg-config modules, e.g.,
  `$(pkg-config functions_framework_cpp --libs)`.

### Documentation and Comments

The documentation (and its links) is intended for human consumption and not
third party websites, or automation (such as scripts scrapping the contents).
The contents and links of our documentation may change without notice.

### Other Interface Points

We think this covers all interface points, if we missed something please
file a [GitHub issue][github-issue].

## Contributing changes

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project, including how to build and test your changes as well as how to
properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](LICENSE) for details.
