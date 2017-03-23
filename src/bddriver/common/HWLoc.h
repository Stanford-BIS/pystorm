namespace pystorm {
namespace bddriver {

#include <string>

class HWLoc {
  // identifies a location in the hardware
  // XXX this doesn't do anything interesting yet
  // mainly creating this now to be more flexible later
  
  public:
    HWLoc(unsigned int chip_id, const std::string& leaf_name);

    inline unsigned int ChipId() const { return chip_id_; }
    inline const std::string * LeafName() const { return &leaf_name_; }

  private:
    unsigned int chip_id_;
    std::string leaf_name_;
}


} // bddriver
} // pystorm
