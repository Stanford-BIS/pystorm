#include "Decoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/DriverTypes.h"
#include "common/DriverPars.h"
#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"
#include "common/vector_util.h" // VectorDeserializer

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Decoder::RunOnce() {
  // we may time out for the Pop, (which can block indefinitely), giving us a chance to be killed
  std::unique_ptr<std::vector<DecInput>> popped_vect = in_buf_->Pop(timeout_us_);

  if (in_buf_->TotalSize() > driverpars::READ_LAG_WARNING_SIZE) { 
    cout << "WARNING: Decoder running " << in_buf_->TotalSize() / driverpars::READ_SIZE << " comm reads behind." << endl;
  }

  if (popped_vect->size() > 0) {
    Decode(std::move(popped_vect));

    // push to each output vector
    for (auto& it : decoded_outputs_) {
      uint8_t ep_code = it.first;
      std::unique_ptr<std::vector<DecOutput>> &vvect = it.second;
      out_bufs_.at(ep_code)->Push(std::move(vvect));
    }

  }
}

void Decoder::Decode(std::unique_ptr<std::vector<DecInput>> input) {

  if (input->size() % BYTES_PER_WORD != 0) {
    cout << "ERROR: Decoder::Decode: received non-multiple of 4 number of inputs. Stopping." << endl;
    Stop();
  }

  unsigned int num_blocks = input->size() % driverpars::READ_BLOCK_SIZE == 0 ? input->size() / driverpars::READ_BLOCK_SIZE : input->size() / driverpars::READ_BLOCK_SIZE + 1;
  assert(driverpars::READ_BLOCK_SIZE % BYTES_PER_WORD == 0);
  assert(input->size() % driverpars::READ_BLOCK_SIZE == 0);

  bool had_nop = false; // we want to see some nops before the end, otherwise the USB may have held up BD

  // clear decoded_outputs_
  decoded_outputs_.clear();

  DecInput * raw_data = input->data();

  unsigned int words_processed = 0;
  for (unsigned int block_idx = 0; block_idx < num_blocks; block_idx++) {
    unsigned int start = block_idx * driverpars::READ_BLOCK_SIZE;
    unsigned int end   = (block_idx + 1) * driverpars::READ_BLOCK_SIZE;

    had_nop = false; 

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

      if(!bd_pars_->UpEPCodeIsBDFunnelEP(ep_code) && !bd_pars_->UpEPCodeIsFPGAOutputEP(ep_code)){
        cout<<(int)ep_code<<endl;
        assert(false);
      }

      // if it's a heartbeat, set last_HB_recvd
      // we send the HBs to the driver too, so it knows the time
      if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_LSB)) {
        last_HB_LSB_recvd_ = payload;
      } else if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_MSB)) {
        BDTime this_HB = PackWord<TWOFPGAPAYLOADS>(
            {{TWOFPGAPAYLOADS::MSB, payload},
             {TWOFPGAPAYLOADS::LSB, last_HB_LSB_recvd_}});
        if (this_HB - curr_HB_recvd_ != curr_HB_recvd_ - last_HB_recvd_) { 
          cout << "WARNING: Decoder::Decode: possibly missed an upstream HB. Jump was " <<
            this_HB - curr_HB_recvd_ << ". Last jump was " << curr_HB_recvd_ - last_HB_recvd_ << endl;
        }
        last_HB_recvd_ = curr_HB_recvd_;
        curr_HB_recvd_ = this_HB;
        //cout << "got HB: " << payload << " curr_HB_ = " << curr_HB_recvd_ << endl;
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

        //if (had_nop) {
        //  cout << "had real data after nop" << endl;
        //}

        DecOutput to_push;
        to_push.payload = payload;

        // update times for "push" output problem
        // edit: for debugging, no attempt at correction
        to_push.time       = curr_HB_recvd_;
        //word_i_min_2_time_ = word_i_min_1_time_;
        //word_i_min_1_time_ = curr_HB_recvd_;

        if (decoded_outputs_.count(ep_code) == 0) {
          decoded_outputs_[ep_code] = std::make_unique<std::vector<DecOutput>>();
        }
        decoded_outputs_.at(ep_code)->push_back(to_push);

        words_processed++;
      }
    }
  }

  //cout << "decoder processed " << words_processed * 4 << " bytes" << endl;

  if (!had_nop) {
    cout << "WARNING: Decoder::Decode: didn't receive any nops in a read, FPGA possibly stalled BD" << endl;
  }
}

}  // bddriver
}  // pystorm
