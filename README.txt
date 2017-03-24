./documentation - PyStorm documentation
./src - All source files. 
./test - Tests for all public methods and functions defined in the include
         directory.

Project structure

The PyStorm repository is structured to enable the development of three 
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
    └── neuromorph                  
