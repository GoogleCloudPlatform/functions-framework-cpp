# Cutting a new release of the Functions Framework for C++

This document describes how to create a new release of the functions framework for C++. The intended audience are
developers in the `functions-framework-cpp` project.

[GitHub repository]: https://github.com/GoogleCloudPlatform/functions-framework-cpp
[GitHub workflow]: https://github.com/googleapis/google-cloud-cpp/blob/main/doc/contributor/howto-guide-forks-and-pull-requests.md
[#326]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/pull/326
[#318]: https://github.com/GoogleCloudPlatform/functions-framework-cpp/pull/318
[GCP builpacks]: https://github.com/GoogleCloudPlatform/buildpacks/

## Overview

## Prerequisites

The document assumes you are familiar with [GitHub](https://github.com) and with the [GitHub workflow] for forks and
pull requests.

This document assumes you have administrator privileges on the project's [GitHub repository]. If you do not have such 
access contact an existing administrator or the owners for the GoogleCloudPlatform organization.

The document also assumes you have an existing fork of the repository, and that this fork is up-to-date with the main
repository. If your fork is behind use the GitHub UI to update it.

Start by cloning the repository, we will use `${GITHUB_USER}` as a variable that is set to your GitHub username:

```sh
git clone git@github.com:${GITHUB_USER}/functions-framework-cpp
```

In this clone create an additional remote for the main repository:

```sh
cd functions-framework-cpp
git remote add upstream https://github.com/GoogleCloudPlatform/functions-framework-cpp
git fetch upstream
```

## Updating CHANGELOG.md and the API baseline

We keep a dump of the ABI in the repository. One of our CI builds uses this baseline to detect API breaks (and yes, we
dump the A**B**I, but try to detect only A**P**I breaks). Before each release we need to update this baseline to detect
any future breaks. Running the CI build locally, on your workstation, updates the baseline:

```sh
ci/cloudbuild/build.sh --trigger check-api-pr
```

This will create a new baseline file in `ci/abi-dumps/functions_framework_cpp.expected.abi.dump.gz`.

Update the local `CHANGELOG.md` file with a **user-level** description of the changes.  Do not think of this as a chore
that could be replaced with a summary generated from GitHub. This is an opportunity to write a summary of the changes
relevant to your users/customers. Skip changes that are purely internal cleanups, code refactoring, CI script updates,
etc. Merge changes that are split across multiple pull-requests into a single line.  The following commands will give
you a start for this document, but the output is just a reminder, not the actual list of changes:

```sh
last_tag="$(git describe --tags --abbrev=0 upstream/main)"
git log --no-merges --format="format:%s" "${last_tag}"..HEAD upstream/main | grep -E -v '^(refactor|cleanup|ci)'
```

Remember to leave room in the `CHANGELOG.md` for the changes related to the next release.

Send a PR with the changes to `CHANGELOG.md` and the API baseline. An example from a previous release is [#326]

## Create the GitHub release

Once the changes to `CHANGELOG.md` are reviewed and accepted, create the release using:

https://github.com/GoogleCloudPlatform/functions-framework-cpp/releases/new

Copy the notes from `CHANGELOG.md` to describe the release.

## Update the version numbers

This is boring, but easy, update the version numbers for the development branch (`main`). A good example from a previous
release is [#318]

## Update the package in `vcpkg`

Create a PR to update the package in `vcpkg`.  Note that the CLA may require SVP approval, it might be easier to ask
`coryan@` to create this PR for you.  A good example from a previous release would be:

https://github.com/microsoft/vcpkg/pull/19603

## Update the GCP buildpacks

Finally, update the [GCP buildpacks] to use this new version. This needs to be done via a CL inside google
(e.g. cl/393879364), which then gets automatically exported to GitHub
(e.g. [buildpacks@c693fe898e8d3f418b8060238fa737664747f685](https://github.com/GoogleCloudPlatform/buildpacks/commit/c693fe898e8d3f418b8060238fa737664747f685))
