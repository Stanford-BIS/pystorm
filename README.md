[![Build Status](https://ng-hippocampus.stanford.edu/jenkins/job/Pystorm/job/master/badge/icon)](https://ng-hippocampus.stanford.edu/jenkins/job/Pystorm/job/master/)

Pystorm is a set of software modules allowing Nengo and Nengo_GUI to 
communicate with Braindrop providing an API that abstracts hardware 
specific data structures.

# Product structure

The Pystorm repository is structured to enable the development of three 
sub-projects, bddriver, bdhal and neuromorph. The three subprojects form
the software stack known as PyStorm.

The following diagram illustrates the structure

	├── pystorm
        │                                                                           
	    ├── CMakeLists.txt                                                          
        │                                                                           
	    ├── Jenkinsfile
        │                                                                           
        ├── bindings                                                                   
        │   └── python 
        │       └── 3.5
        │           └── src
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
        ├── src                                                                
        │   ├── bddriver                                                      
        │   ├── bdhal                                                      
        │   └── neuromorph 
        │                                                                           
        ├── test                                                                
        │   ├── bddriver
        │   ├── bdhal
        │   └── neuromorph 



# Build

Pystorm modules can be built issuing the following commands from the repositories
base directory.

```
    mkdir build
    cd build
    cmake ..
    make
```

# TEST 

From the build directory all module tests can be executed issuing the 
following command.

```
    make test ARGS="-V"
```

# Dependencies

Pystorm was built and is dependent on the following software packages:

    Ubuntu 16.04 
    CMake 3.5
    Python 3.5 
    GCC 5.4 (or later) (g++ component)
    Boost 1.58 with Python3.5 component
    Google Test
    libusb 1.0

# Docker

Docker can be used to build and test Pystorm. The folder `docker` has a 
Docker file (named `Dockerfile_JENKINS_CI`) and shell script that can be 
used to build an image and build/test Pystorm on it.

The following is an example of how to use Docker to build and test using 
Docker.

    sudo docker build --file docker/Dockerfile_JENKINS_CI .
