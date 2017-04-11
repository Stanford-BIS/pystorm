#ifndef DRIVER_H
#define DRIVER_H   

#include <memory>

#include "common/BDPars.h"
#include "common/DriverPars.h"
#include "common/HWLoc.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"

namespace pystorm
{
namespace bddriver
{
/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */
class Driver
{
  public:
    // Return a global instance of bddriver
    static Driver& getInstance();

    // Just a test call
    void testcall(const std::string& msg);

    void Start(); // starts child workers, e.g. encoder and decoder

    void StartTraffic();
    void KillTraffic();

    void StartNeuronTraffic();
    void KillNeuronTraffic();

    void SetDACValue(const std::string & dac_name, unsigned int value);

    void ConnectDACtoADC(const std::string & dac_name);
    void DisconnectDACsfromADC();

    void SetMemoryDelay(const std::string & memory_name, unsigned int value);
    void InitFIFO();

  protected:
    Driver();
    ~Driver();

    // parameters describing parameters of the software (e.g. buffer depths)
    DriverPars * driver_pars_; 
    // parameters describing BD hardware
    BDPars * bd_pars_; 

    // thread-safe circular buffers sourcing/sinking encoder and decoder
    MutexBuffer<EncInput> * enc_buf_in_;
    MutexBuffer<EncInput> * enc_buf_out_;

    MutexBuffer<DecInput> * dec_buf_in_;
    MutexBuffer<DecInput> * dec_buf_out_;

    Encoder * enc_; // encodes traffic to BD
    Decoder * dec_; // decodes traffic from BD

    // low-level programming calls, breadth of high-level downstream API goes through these
    void ToggleStream(const std::string & stream_name, bool traffic_on, bool dump_on); // wrapper around SetRegister
    void SetRegister(const std::string & register_name, unsigned int val);
    void SetRIWIMemory(const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRMWMemory(const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRWMemory(const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);

};

} // bddriver namespace
} // pystorm namespace

#endif
