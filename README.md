[![Build Status](https://ng-hippocampus.stanford.edu/jenkins/job/Pystorm/job/master/badge/icon)](https://ng-hippocampus.stanford.edu/jenkins/job/Pystorm/job/master/)

pystorm is a set of software modules allowing Nengo and Nengo_GUI to
communicate with Braindrop and Brainstorm by providing an API that abstracts
away hardware-specific details. pystorm provides shared headers, libraries,
and a python interface.

# Product structure

The Pystorm repository is structured to enable the development of two
sub-projects, bddriver, and bdhal. These subprojects form
the software stack known as pystorm.

The following diagram illustrates the structure

    └── pystorm
        │
        ├── CMakeLists.txt
        │
        ├── Jenkinsfile
        │
        ├── docker
        │   └── Dockerfile_JENKINS_CI
        │
        ├── ext                         (externally sourced libraries)
        │   ├── yaml-cpp
        │   └── gtest
        │
        ├── include                     (public shared headers)
        │
        ├── lib                         (public shared libs)
        │
        ├── pystorm                     (python package source)
        │   └── bdhal
        │
        ├── src                         (c++ source)
        │   ├── bddriver
        │   ├── bdhal
        │   └── bindings
        │       └── python
        │           └── 3.5
        │
        └── test
            ├── bddriver
            ├── bdhal
            └── bindings
                └── python
                    └── 3.5


Python code lives in the pystorm directory tree, C++ code lives in the src directory tree.

# Python package setup

To set up the python interface for Pystorm, first run the [Build](#build). Next, run

`python setup.py develop`

from the repository base directory so that you may import pystorm as a Python package.

Note that if you want to install only for the current user, you should specify

`python setup.py develop --user`

# Build

We recommend building the pystorm C++ modules by executing the following commands from the
repository base directory.

```
    mkdir build
    cd build
    cmake ..
    cmake --build .
```

Note that after the `cmake ..` command, you can build using your favorite tool like make, ninja, XCode, or Microsoft Visual studio.
However, we recommend `cmake --build .` as a platform-agnostic and tool-agnostic method of invoking the build
(i.e. you shouldn't have to worry about the platform specific methods of invoking a build).

## cmake Options

Beyond the default cmake options, our CMakeLists.txt files use following options:

* `-DPYTHON_EXECUTABLE=<path>` tells cmake to use the Python interpreter at `<path>`
* `-DCMAKE_BUILD_TYPE=<[Release, Debug]>` Used for single-configuration generators (e.g. GNU make and its relatives) to tell which build type to set up and build. For multi-configuration generators (e.g. Visual Studio and Xcode), use the `cmake --build`  `--config` option as described [below](#cmake-build-options). For the distinction between single and multi-configuration generators, see [here](https://stackoverflow.com/a/24470998)
* `-DBD_COMM_TYPE=<[MODEL, OPALKELLY, SOFT, USB]>` tells which kind of communication to expect

For example,

`cmake -DPYTHON_EXECUTABLE=/path/to/python -DCMAKE_BUILD_TYPE=Release -DBD_COMM_TYPE=MODEL ..`

tells cmake to use the python interpreter at `/path/to/python`, to make a `Release` type build, and to use the `MODEL` of braindrop as the communication type.

## cmake Build Options

`cmake --build .` tells cmake to run the build. We support the following options for the build command:

* `--config <[Release, Debug]>` tells cmake which configuration type to build for multi-configuration generators.

We can also pass native compiler options to the build after adding a `--` in the call:

As an example,

`cmake --build . --config Release -- -j6`

tells cmake to build the project in the current directory for the `Release` configuration and pass `-j6` to the compiler, which for g++ says to use 6 threads for the build.

## Opal Kelly udev Rules

To use the Opal Kelly board, you must copy pystorm/60-opalkelly.rules to /etc/udev/rules.d/

Then reboot or issue the following commands:

```
   /sbin/udevadm control --reload-rules
   /sbin/udevadm trigger
```

# TEST

## C Code Tests

After the build, C code can be executed issuing the
following command from the build directory.

```
    ctest -C Debug -j6 -T test -VV --timeout 300
```

* `-C <[Debug, Release]>` selects between debug and release configurations
* `-j<number of threads>` parallelizes the build
* `-T test` specifies the type of test (always `test` for us)
* `-VV` specifies extra verbosity
* `--timout 300` specifieds that the tests should be halted at 5 minutes if they're still running (i.e. in case the tests are hanging)

Individual tests can be run from their executables in the `lib/<Release Type>/` directory.

## Python tests 

Tests of the python interface may be executed by running `pytest` from within

`<Repository Root>/pystorm/test/`

These tests require a Braindrop to be attached.

# Dependencies

Pystorm is built and tested using docker.
Pystorm's dependencies can be found in the dockerfile `docker/Dockerfile_build_environment_image`,
which is used to create the docker image used for building and testing.

# Docker

Docker can be used to build and test Pystorm. The folder `docker` has a
Docker file (named `Dockerfile_compile_source`) and shell script that can be
used to build an image and build/test Pystorm on it.

The following is an example of how to use Docker to run the build build

    sudo docker build --file docker/Dockerfile_compile_source .
