# Khepri Versioning

Khepri uses [Semantic Versioning](https://semver.org/).

## Official versions

Official versions do not contain build information. That is, example versions are "`1.2.3`", "`1.2.1-alpha`", "`1.2.2-rc1`", etc.

## Internal versions

Internally, for tracing and debugging purposes, the version information that is embedded in the library at build time contains build metadata. An example of such version is "`1.2.3+123abcd.dirty`". This version indicates official version "1.2.3" with build metadata "123abcd.dirty". The build information contains the exact version of the source code used in the build (i.e. the Git hash) and whether or not the source code has any changes on top of the version.

## Deriving the version from Git

The build system derives the version information from Git at build time. To do this, it follows the following process:
* Check out the branch to build. The HEAD commit is the Git hash for the build metadata.
* Find the latest tag that is reachable from the HEAD commit. This tag is the version number (e.g. "`1.2.3`")
* If the tag points to a different commit, the build is considered "dirty".
* If the workspace has staged or unstaged changes (relevant for local builds), the build is considered "dirty".