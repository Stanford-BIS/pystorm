#ifndef DRIVERPARS_H
#define DRIVERPARS_H

#include <string>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

class DriverPars {
  public:
    // init from yaml file describing driver parameters
    DriverPars();
    ~DriverPars();

    inline unsigned int Get(const std::string& component, const std::string& property) const 
      { return pars_.at(component).at(property); }

  private:
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned int> > pars_;

};

} // bddriver
} // pystorm

#endif
