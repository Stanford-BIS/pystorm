#include "Decoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/DriverTypes.h"
#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"
#include "common/vector_util.h" // VectorDeserializer

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

static const unsigned int READ_SIZE = 512 * 256;

void Decoder::RunOnce() {
  // we may time out for the Pop, (which can block indefinitely), giving us a chance to be killed
  std::unique_ptr<std::vector<DecInput>> popped_vect = in_buf_->Pop(timeout_us_);

  if (in_buf_->TotalSize() > READ_SIZE*4) { // four buffers behind XXX shouldn't hardcode
    cout << "WARNING: Decoder running " << in_buf_->TotalSize() / READ_SIZE << " comm reads behind." << endl;
  }

  if (popped_vect->size() > 0) {
    std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> to_push_vects = Decode(std::move(popped_vect));

    // push to each output vector
    for (auto& it : to_push_vects) {
      uint8_t ep_code = it.first;
      std::unique_ptr<std::vector<DecOutput>> &vvect = it.second;
      out_bufs_.at(ep_code)->Push(std::move(vvect));
    }

  }
}

std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decoder::Decode(std::unique_ptr<std::vector<DecInput>> input) {

  const unsigned int BYTES_PER_WORD = 4;

  if (input->size() % BYTES_PER_WORD != 0) {
    cout << "ERROR: Decoder::Decode: received non-multiple of 4 number of inputs. Stopping." << endl;
    Stop();
  }

  const unsigned int READ_BLOCK_SIZE = 1024; // XXX shouldn't hardcode this, should get from Comm
  unsigned int num_blocks = input->size() % READ_BLOCK_SIZE == 0 ? input->size() / READ_BLOCK_SIZE : input->size() / READ_BLOCK_SIZE + 1;
  assert(READ_BLOCK_SIZE % BYTES_PER_WORD == 0);

  bool had_nop = false; // we want to see some nops before the end, otherwise the USB may have held up BD
  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> outputs;

  DecInput * raw_data = input->data();

  unsigned int words_processed = 0;
  for (unsigned int block_idx = 0; block_idx < num_blocks; block_idx++) {
    unsigned int start = block_idx * READ_BLOCK_SIZE;
    unsigned int end   = (block_idx + 1) * READ_BLOCK_SIZE;
    end = end > input->size() ? end : input->size();

    for (unsigned int word_idx = start; word_idx < end; word_idx += BYTES_PER_WORD) { // iterating by 4!!

      // deserialize
      // because we can skip parts of the stream, this isn't suitable for the deserializer
      DecInput b0 = raw_data[word_idx+0];
      DecInput b1 = raw_data[word_idx+1];
      DecInput b2 = raw_data[word_idx+2];
      DecInput b3 = raw_data[word_idx+3];

      uint32_t packed_word = PackWord<FPGABYTES>(
         {{FPGABYTES::B0, b0}, 
          {FPGABYTES::B1, b1}, 
          {FPGABYTES::B2, b2}, 
          {FPGABYTES::B3, b3}});

      // break out ep_code/payload
      unsigned int ep_code = GetField<FPGAIO>(packed_word, FPGAIO::EP_CODE);
      uint32_t payload     = GetField<FPGAIO>(packed_word, FPGAIO::PAYLOAD);

      // if it's a heartbeat, set last_HB_recvd
      if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB)) {
        // unpack current time MSB and LSB
        uint64_t curr_HB_msb = GetField(last_HB_recvd_, TWOFPGAPAYLOADS::MSB);
        uint64_t curr_HB_lsb = GetField(last_HB_recvd_, TWOFPGAPAYLOADS::LSB);

        // update last_HB_recvd_ LSB
        if (next_HB_significance_ == NextHBSignificance::LSB) {
          last_HB_recvd_ = PackWord<TWOFPGAPAYLOADS>(
              {{TWOFPGAPAYLOADS::MSB, curr_HB_msb}, 
               {TWOFPGAPAYLOADS::LSB, payload}});
          next_HB_significance_ = NextHBSignificance::MSB;

        // update last_HB_recvd_ MSB
        } else if (next_HB_significance_ == NextHBSignificance::MSB) {
          last_HB_recvd_ = PackWord<TWOFPGAPAYLOADS>(
              {{TWOFPGAPAYLOADS::MSB, payload},
               {TWOFPGAPAYLOADS::LSB, curr_HB_lsb}});
          next_HB_significance_ = NextHBSignificance::LSB;
        } else {
          assert(false && "something wrong with next_HB_significance_ enum");
        }
        //cout << "got HB: " << payload << " curr_HB_ = " << last_HB_recvd_ << endl;
        // we send the HBs to the driver too, so it knows the time
      }

      // ignore queue counts (first word of each block)
      if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::DS_QUEUE_CT)) {
        // pass
      // break on nop
      } else if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::NOP)) {
        had_nop = true;
        break; // all further words in the block are guaranteed to be nops!
        
      // otherwise, forward to Driver
      } else {
        //cout << "decoder got something that wasn't a HB" << endl;
        //cout << "  ep# = " << ep_code << endl;
        
        // XXX core id?

        DecOutput to_push;
        to_push.payload = payload;

        // update times for "push" output problem
        // edit: for debugging, no attempt at correction
        to_push.time       = last_HB_recvd_;
        //word_i_min_2_time_ = word_i_min_1_time_;
        //word_i_min_1_time_ = last_HB_recvd_;

        if (outputs.count(ep_code) == 0) {
          outputs[ep_code] = std::make_unique<std::vector<DecOutput>>();
        }
        outputs.at(ep_code)->push_back(to_push);

        words_processed++;
      }
    }
  }

  //cout << "decoder processed " << words_processed * 4 << " bytes" << endl;

  if (!had_nop) {
    cout << "WARNING: Decoder::Decode: didn't receive any nops in a read, FPGA possibly stalled BD" << endl;
  }

  return outputs;
}

}  // bddriver
}  // pystorm
