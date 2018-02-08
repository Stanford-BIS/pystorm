#include "BDModelUtil.h"

#include "common/BDState.h"
#include "common/vector_util.h"
#include "encoder/Encoder.h" // for bytesPerOutput (XXX should be in BDPars??)
#include "decoder/Decoder.h" // for bytesPerInput (XXX should be in BDPars??)

//#include <bitset>

namespace pystorm {
namespace bddriver {
namespace bdmodel {

std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars* pars) {
  // for now, just deserialize USB bytestream
  // don't have to worry about remainders/deserialization

  assert(inputs.size() % Encoder::bytesPerOutput == 0);

  std::vector<uint32_t> deserialized_words;
  for (unsigned int i = 0; i < inputs.size() / Encoder::bytesPerOutput; i++) {
    uint32_t word = PackWord<FPGABYTES>(
        {{FPGABYTES::B0, inputs[Encoder::bytesPerOutput * i + 0]},
         {FPGABYTES::B1, inputs[Encoder::bytesPerOutput * i + 1]},
         {FPGABYTES::B2, inputs[Encoder::bytesPerOutput * i + 2]},
         {FPGABYTES::B3, inputs[Encoder::bytesPerOutput * i + 3]}});
    deserialized_words.push_back(word);
  }
  return deserialized_words;
}

std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars) {

  std::vector<DecInput> retval;
  for (auto& input : inputs) {
    uint8_t b0 = GetField<FPGABYTES>(input, FPGABYTES::B0);
    uint8_t b1 = GetField<FPGABYTES>(input, FPGABYTES::B1);
    uint8_t b2 = GetField<FPGABYTES>(input, FPGABYTES::B2);
    uint8_t b3 = GetField<FPGABYTES>(input, FPGABYTES::B3);
    retval.push_back(b0);
    retval.push_back(b1);
    retval.push_back(b2);
    retval.push_back(b3);
  }

  assert(retval.size() % Decoder::bytesPerInput == 0);
  return retval;
}

std::vector<BDWord> DeserializeEP(uint8_t code, const std::vector<uint32_t>& inputs, unsigned int D) {
  static std::unordered_map<uint8_t, std::unique_ptr<VectorDeserializer<uint32_t>>> deserializers;

  if(deserializers.count(code) == 0) {
    deserializers[code] = std::make_unique<VectorDeserializer<uint32_t>>(D);
  }

  std::vector<BDWord> words;

  // read first word
  if (D == 1) {
    // have to cast
    for (auto& it : inputs) {
      words.push_back(it);
    }
  } else if (D == 2 || D == 3) {
    // we shouldn't have to worry about remainders with BDModel
    // use a VectorDeserializer anyway so we can copy-paste from RecvFromEP
    // XXX should maybe figure out a way to reuse this code better

    // XXX make a copy so we can get a unique_ptr
    auto input_copy = std::make_unique<std::vector<uint32_t>>(inputs);
    deserializers[code]->NewInput(std::move(input_copy));

    std::vector<uint32_t> deserialized; // continuosly write into here
    deserializers[code]->GetOneOutput(&deserialized);
    while (deserialized.size() > 0) {
      // for now, D == 2 for all deserializers, so we can do this hack
      // if the width of a single data object returned from the FPGA ever
      // exceeds 64 bits, we may need to rethink this
      assert(deserialized.size() == D); // the only case to deal with for now
      
      BDWord payload_all;
      if (D == 2) {
        uint32_t payload_lsb = deserialized.at(0);
        uint32_t payload_msb = deserialized.at(1);

        // concatenate lsb and msb to make output word
        payload_all = PackWord<TWOFPGAPAYLOADS>({
            {TWOFPGAPAYLOADS::LSB, payload_lsb}, 
            {TWOFPGAPAYLOADS::MSB, payload_msb}});
      } else if (D == 3) {
        uint32_t payload_w0 = deserialized.at(0);
        uint32_t payload_w1 = deserialized.at(1);
        uint32_t payload_w2 = deserialized.at(2);

        //std::bitset<20> b0(payload_w0);
        //std::bitset<20> b1(payload_w1);
        //std::bitset<20> b2(payload_w2);
        //cout << "model  " << b2 << b1 << b0 << endl;

        // concatenate lsb and msb to make output word
        payload_all = PackWord<THREEFPGAPAYLOADS>({
            {THREEFPGAPAYLOADS::W0, payload_w0},
            {THREEFPGAPAYLOADS::W1, payload_w1},
            {THREEFPGAPAYLOADS::W2, payload_w2}});
      }
      words.push_back(payload_all);

      deserializers[code]->GetOneOutput(&deserialized);
    }
  } else {
    cout << "WARNING: BDModel deserializer received unhandled deserialization factor, throwing the words away" << endl;
  }
  return words;
}

std::vector<uint32_t> SerializeEP(const std::vector<BDWord>& inputs, unsigned int D) {

  std::vector<uint32_t> serialized;

  if (D > 1) {
    // if the width of a single data object sent downstream ever
    // exceeds 64 bits, we may need to rethink this
    assert(D == 2); // only case to deal with for now
    for (auto& it : inputs) {
      // lsb first, msb second
      serialized.push_back(GetField(it, TWOFPGAPAYLOADS::LSB));
      serialized.push_back(GetField(it, TWOFPGAPAYLOADS::MSB));
    }
  } else {
    // XXX we have to cast for now. Probably should just change EncInput to have uint64_t/BDWord input
    for (auto& it : inputs) {
      serialized.push_back(static_cast<uint32_t>(it));
    }
  }
  return serialized;
}

}  // bdmodel
}  // bddriver
}  // pystorm
