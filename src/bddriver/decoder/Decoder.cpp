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
    cout << "WARNING: bddriver::Decoder (upstream data processing) running " << in_buf_->TotalSize() / driverpars::READ_SIZE << " comm reads behind." << endl;
  }

  if (popped_vect->size() > 0) {
    Decode(std::move(popped_vect));

    // push to each output vector
    for (auto& core_out : decoded_outputs_) {
      uint8_t core = core_out.first;
      for (auto& it : core_out.second){
        uint8_t ep_code = it.first;
        std::unique_ptr<std::vector<DecOutput>> &vvect = it.second;
        out_bufs_[core].at(ep_code)->Push(std::move(vvect));
      }
    }
  }
}

void Decoder::Decode(std::unique_ptr<std::vector<DecInput>> input) {

  if (input->size() % BYTES_PER_WORD != 0) {
    cout << "ERROR: bddriver::Decoder::Decode: received non-multiple of 4 number of inputs. Stopping." << endl;
    Stop();
  }

  unsigned int num_blocks = input->size() % driverpars::READ_BLOCK_SIZE == 0 ? input->size() / driverpars::READ_BLOCK_SIZE : input->size() / driverpars::READ_BLOCK_SIZE + 1;
  assert(driverpars::READ_BLOCK_SIZE % BYTES_PER_WORD == 0);
  assert(input->size() % driverpars::READ_BLOCK_SIZE == 0);

  bool had_nop = false; // whether we saw a nop at all
  bool had_nop_block = false; // whether we saw a block that was completely empty
  unsigned int bytes_used = 0;

  // clear decoded_outputs_
  decoded_outputs_.clear();

  DecInput * raw_data = input->data();

  unsigned int words_processed = 0;
  unsigned int last_code = 0;
  uint32_t code_count = 0;
  for (unsigned int block_idx = 0; block_idx < num_blocks; block_idx++) {
    unsigned int start = block_idx * driverpars::READ_BLOCK_SIZE;
    unsigned int end   = (block_idx + 1) * driverpars::READ_BLOCK_SIZE;

    bool had_nop_this_block = false; 


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
      unsigned int core    = GetField<FPGAIO>(packed_word, FPGAIO::ROUTE);
      unsigned int ep_code = GetField<FPGAIO>(packed_word, FPGAIO::EP_CODE);
      uint32_t payload     = GetField<FPGAIO>(packed_word, FPGAIO::PAYLOAD);

      if(!(bd_pars_->UpEPCodeIsBDFunnelEP(ep_code)) && !(bd_pars_->UpEPCodeIsFPGAOutputEP(ep_code))){
        // cout<<"WARNING: WEIRD CODE: "<<(int)ep_code<<endl;
      }else{

      // if it's a heartbeat, set last_HB_recvd
      // we send the HBs to the driver too, so it knows the time
      if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_LSB)) {
        last_HB_LSB_recvd_ = payload;
      } else if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_MSB)) {
        BDTime this_HB = PackWord<TWOFPGAPAYLOADS>(
            {{TWOFPGAPAYLOADS::MSB, payload},
             {TWOFPGAPAYLOADS::LSB, last_HB_LSB_recvd_}});

        if (this_HB - curr_HB_recvd_ != curr_HB_recvd_ - last_HB_recvd_) { 
          cout << "WARNING: bddriver::Decoder::Decode: possibly missed an upstream HB. Jump was " <<
            this_HB - curr_HB_recvd_ << ". Last jump was " << curr_HB_recvd_ - last_HB_recvd_ <<
            ". Could indicate data loss. Also happens when FPGA timing is modified." << endl;
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
        had_nop_this_block = true;
        if (word_idx == 0) {
          had_nop_block = true;
        }
        bytes_used += (word_idx - start) * BYTES_PER_WORD;
        break; // all further words in the block are guaranteed to be nops!

      } else if (ep_code == bd_pars_->DnEPCodeFor(bdpars::BDHornEP::RI)) { // note, Dn, not UpEPCode
        cout << "WARNING: bddriver::Decoder: got tag intended for another BD (a few on startup is normal)" << endl;
      
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
        if (decoded_outputs_.count(core) == 0){
          decoded_outputs_[core] = std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>>();
        }
        if (decoded_outputs_[core].count(ep_code) == 0) {
          decoded_outputs_[core][ep_code] = std::make_unique<std::vector<DecOutput>>();
        }
        decoded_outputs_[core].at(ep_code)->push_back(to_push);

        words_processed++;
      }
    }
    if (!had_nop_this_block) {
      bytes_used += driverpars::READ_BLOCK_SIZE;
    }
  }
  }

  //cout << "decoder processed " << words_processed * 4 << " bytes" << endl;

  // if (!had_nop_block && !had_nop) {
  //   cout << "WARNING: bddriver::Decoder::Decode: read was full of data. Out of upstream throughput. Probable data loss" << endl;
  //   cout << "  " << bytes_used << " bytes used in frame out of " << driverpars::READ_SIZE << endl;
  // } else if (bytes_used > driverpars::READ_FULL_WARNING_SIZE) {
  //   cout << "WARNING: bddriver::Decoder::Decode: read was nearly full of data. Operating very close to upstream throughput limit, but probably OK" << endl;
  //   cout << "  " << bytes_used << " bytes used in frame out of " << driverpars::READ_SIZE << endl;
  // }
}

}  // bddriver
}  // pystorm
