
# gchsd - Git Cache HTTP Server Daemon

[![Build Status](http://xaws6t1emwa2m5pr.myfritz.net:8080/buildStatus/icon?job=craflin%2Fgit-cache-http-server%2Fmaster)](http://xaws6t1emwa2m5pr.myfritz.net:8080/job/craflin/job/git-cache-http-server/job/master/)

A daemon to mirror remote Git repositories and serve them over HTTP, automatically updating the mirror as needed.

It was developed to be used in CI environments to improve the performance of Git clone and fetch operations.

The project is a C++ clone of jonasmalacofilho's [git-cache-http-server](https://github.com/jonasmalacofilho/git-cache-http-server).
However, there are these differences and improvements:
* The repository cache is updated sequentially and for each git fetch or clone operation to avoid race conditions when the remote is updated while some request is updating the cache.
* The authentication works by fetching or cloning the remote instead of testing if the remote is accessible using the credentials.
* It can easily be configured to run as a systemd daemon.

## Build Instructions

(It was developed on Debian-based platforms, it can probably also be compiled on other platforms, but don't ask me how.)

* Clone the Git repository and initialize submodules.
* Install zlib1g-dev `sudo apt-get install zlib1g-dev`
* Build the project with CMake.
* You can build a *deb* package using the target *package* in CMake.

## Server Setup

* Install *git*.
* Install *gchsd* from the *deb* package.
* Configure a cache directory (default is */tmp/gchsd*) and listen port (default is *80*) in */etc/gchsd.conf*.
* Start the *gchsd* daemon with `sudo systemctl start gchsd`.
  * You can use `sudo systemctl enable gchsd` to start the daemon automatically after a system restart.

## Client Setup

* Run `git config --global url."http://<mirror_server_addr>[:<port>]/https://".insteadOf https://` to use the mirror instead of the original source.
