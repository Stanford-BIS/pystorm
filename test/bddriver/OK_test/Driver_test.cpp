#include "Driver.h"

#include <iostream>
#include <vector>
#include <unistd.h>

using namespace pystorm;
using namespace bddriver;

using std::cout;
using std::endl;

int main () {
  Driver driver;
  driver.Start();

  cout << "started" << endl;

  driver.ResetBD();

  usleep(1000000);

  cout << "reset" << endl;

  //std::vector<BDWord> to_write;
  //for (unsigned int i = 0; i < 64; i++) {
  //  to_write.push_back(i);
  //}
  //driver.SetMem(0, bdpars::BDMemId::PAT, to_write, 0);

  //cout << "set mem" << endl;

  //cout << "dumping contents" << endl;
  //std::vector<BDWord> PAT_dump = driver.DumpMem(0, bdpars::BDMemId::PAT);
  //for (unsigned int i = 0; i < PAT_dump.size(); i++) {
  //  cout << "data " << i << " = " << PAT_dump[i] << endl;
  //}

  //cout << "dumping contents" << endl;
  //PAT_dump = driver.DumpMem(0, bdpars::BDMemId::PAT);
  //for (unsigned int i = 0; i < PAT_dump.size(); i++) {
  //  cout << "data " << i << " = " << PAT_dump[i] << endl;
  //}

  //usleep(1000000);

  //cout << "dumping contents" << endl;
  //PAT_dump = driver.DumpMem(0, bdpars::BDMemId::PAT);
  //for (unsigned int i = 0; i < PAT_dump.size(); i++) {
  //  cout << "data " << i << " = " << PAT_dump[i] << endl;
  //}

  usleep(1000000);

  // set TAT
  //driver.SetMemoryDelay(0, bdpars::BDMemId::TAT1, 2, 2);

  std::vector<BDWord> to_write;
  to_write.clear();
  for (unsigned int i = 0; i < 1024; i++) {
    to_write.push_back(i);
  }
  driver.SetMem(0, bdpars::BDMemId::TAT1, to_write, 0);

  cout << "dumping contents" << endl;
  std::vector<BDWord> TAT1_dump = driver.DumpMemRange(0, bdpars::BDMemId::TAT1, 0, 1024);
  for (unsigned int i = 0; i < TAT1_dump.size(); i++) {
    cout << "data " << i << " = " << TAT1_dump[i] << endl;
  }

  usleep(1000000);

  //cout << "dumping fewer contents" << endl;
  //TAT1_dump = driver.DumpMemRange(0, bdpars::BDMemId::TAT1, 0, 200);
  //for (unsigned int i = 0; i < TAT1_dump.size(); i++) {
  //  cout << "data " << i << " = " << TAT1_dump[i] << endl;
  //}

  //usleep(1000000);

  //cout << "dumping fewer contents" << endl;
  //TAT1_dump = driver.DumpMemRange(0, bdpars::BDMemId::TAT1, 800, 1024);
  //for (unsigned int i = 0; i < TAT1_dump.size(); i++) {
  //  cout << "data " << i << " = " << TAT1_dump[i] << endl;
  //}

  //usleep(1000000);
  driver.Stop();

}
