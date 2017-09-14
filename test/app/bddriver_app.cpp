#include "model/BDModelDriver.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>

#include <chrono>
#include <thread>

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;

int main(char* argc, char* argv[]) {
      BDModelDriver* driver = new BDModelDriver();
      bdmodel::BDModel* model = driver->GetBDModel();
      
      printf("Starting\n");
      driver->Start();
      printf("InitBD\n");
      driver->InitBD();

      std::this_thread::sleep_for(std::chrono::seconds(2));
      printf("Stopping");
      driver->Stop();

      //std::this_thread::sleep_for(std::chrono::seconds(2));
      delete driver;
      return 0;
}
