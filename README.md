
# gchsd - Git Cache HTTP Server Daemon

[![Build Status](http://xaws6t1emwa2m5pr.myfritz.net:8080/buildStatus/icon?job=craflin%2Fgit-cache-http-server%2Fmaster)](http://xaws6t1emwa2m5pr.myfritz.net:8080/job/craflin/job/git-cache-http-server/job/master/)

A daemon to mirror remote Git repositories and serve them over HTTP, automatically updating the mirror as needed.

It can be used in CI environments to improve the performance of Git clone operations.

This is a C++/CURL clone of jonasmalacofilho's [git-cache-http-server](https://github.com/jonasmalacofilho/git-cache-http-server).

## Build Instructions

* Clone the repository and initialize submodules.
* Build the project with `cmake`.
* You can build a `deb` package using the target `package` in CMake.

## Setup

* Install `gchsd` from the `deb` package.
* Configure a cache directory in `/etc/gchsd.conf`.
* Start the `gchsd` daemon with `sudo systemctl start gchsd`.
    * You can use `sudo systemctl enable gchsd` to start the daemon automatically after a system restart.
