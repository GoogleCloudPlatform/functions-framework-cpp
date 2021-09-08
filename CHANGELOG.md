# Changelog

## v1.1.0 - TBD

## v1.0.0 - 2021-09

We are pleased to announce that the C++ Functions Framework is
now generally available (GA). We think the code is ready for
production use, and that the public APIs are stable.

* fix: use correct version macro for pkg-config (#322)

## v0.6.0 - 2021-08

* feat: support mapping of legacy GCF events (#313)
* fix: actually handle sessions in parallel (#308)
* feat: improve error message for CloudEvent (#310)
* doc: simplify how-to guides (#304)
* feat: more efficient access to HttpRequest data (#300)

## v0.5.0 - 2021-03

* feat: make HttpResponse fluent (#293)
* ci: add a build for MSVC 2019 (#290)
* refactor: use vcpkg features to control dependencies (#289)
* fix: build problems with MSVC (#288)

## v0.4.0 - 2021-02

* feat: use structured logging for exceptions (#278)
* feat: convert storage Pub/Sub events (#273)
* feat: how-to guide using Cloud Build for builder (#267)
* chore: update to pack v0.17.0 (#263)
* feat: implement code for Cloud Spanner tutorial (#239)
* feat: implement code for Cloud Bigtable tutorial (#238)
* doc: this library is now published in vcpkg (#237)
* feat: fix compilation problems on MSVC (#233)
* doc: new guide to deploy Pub/Sub function to Cloud Run (#234)
* doc: new guide to deploy on Cloud Run (#232)
* doc: describe how to create a Docker image (#212)
* doc: describe how to run a local server (#209)
* We implemented many other examples, but this changelog is
  too long already.

## v0.3.0 - 2021-02

* **BREAKING CHANGE:** Renamed package and targets to use
  `functions-framework-cpp` as the project's prefix.

* doc: clarify backwards compatibility guidelines (#204)
* feat: implement storage_integration_test example (#198)
* feat: implement pubsub_integration_test example (#195)
* feat: implement storage_unit_test example (#196)
* feat: implement pubsub_unit_test example (#194)
* refactor: use Boost.Log in some examples (#192)
* feat: implement http_system_test example (#193)

## v0.2.0 - 2021-01

* Moved `Run()` to the public API.

## Prior History

* Completed interfaces for `HTTP` and `CloudEvent` handlers.
