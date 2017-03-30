Pystorm is a set of software modules allowing Nengo and Nengo_GUI to 
communicate with Braindrop providing an API that abstracts hardware 
specific data structures.

# Product structure

The Pystorm repository is structured to enable the development of three 
sub-projects, bddriver, bdhal and neuromorph. The three subprojects form
the software stack known as PyStorm.

The following diagram illustrates the structure

└── pystorm
    ├─  CMakeLists.txt
    ├── bddriver                    
    │   ├── CMakeLists.txt
    │   ├── build                   (out-of-source tree)
    │   ├── lib                     (out-of-source tree)
    │   ├── docs                    (out-of-source tree)
    │   ├── test                    (out-of-source tree)
    │   │   ├── CMakeLists.txt
    │   │   ├── comm                
    │   │   │   ├── CMakeLists.txt
    │   │   ├── common              
    │   │   │   ├── CMakeLists.txt
    │   │   ├── encoder             
    │   │   │   ├── CMakeLists.txt
    │   │   ├── driverifc           
    │   │   │   ├── CMakeLists.txt
    │   │   └── decoder             
    │   │       └── CMakeLists.txt
    │   │       
    │   ├── src
    │   │   ├── CMakeLists.txt
    │   │   ├── comm                
    │   │   │   ├── CMakeLists.txt
    │   │   ├── common              
    │   │   │   ├── CMakeLists.txt
    │   │   ├── encoder             
    │   │   │   ├── CMakeLists.txt
    │   │   ├── driverifc           
    │   │   │   ├── CMakeLists.txt
    │   │   └── decoder             
    │   │       └── CMakeLists.txt
    │   │       
    │   └── include                 (public C++ API)
    │  
    ├── bdhal                       
    │   ├── build                   (out-of-source tree)
    │   ├── lib                     (out-of-source tree)
    │   ├── docs                    (out-of-source tree)
    │   ├── test                    (out-of-source tree)
    │   ├── src
    │   └── include                 (public C++ API)
    │
    ├── neuromorph                  
    │
    └── docker                  

# Build
The module bddriver is currently buildable by cd'ing to the 
`pystorm/bddriver` directory and running the `build_driver.sh` script.

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
Docker file (named `Dockerfile`) and shell script that can be used to build 
an image and build/test Pystorm on it.

The files can be moved to any directory on a machine with Docker installed and
used to build and test Pystorm.

The following is an example of how to use Docker to build and test using 
Docker.

    cp pystorm/docker/* <your_docker_directory>
    cd <your_docker_directory>
    git clone git@github.com:Stanford-BIS/pystorm.git
    sudo docker build -t <your_build_name> .
    sudo docker run --rm -t <your_build_name>:latest
