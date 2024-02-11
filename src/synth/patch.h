// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2023-2024 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef PFM2SID_SYNTH_PATCH_H_
#define PFM2SID_SYNTH_PATCH_H_

#include "synth/parameter_structs.h"
#include "synth/wavetable.h"
#include "util/util_fourcc.h"
#include "util/util_stream_buffer.h"

namespace pfm2sid::synth {

class PatchBank;
class PatchBrowser;

class Patch {
public:
  static constexpr util::FOURCC kStorageID = "PTCH"_4CC;
  static constexpr util::FOURCC kEndMarker = "ENDE"_4CC;
  static constexpr util::FOURCC kVersionID = "v001"_4CC;

  static constexpr size_t kBinaryPatchSize = 1024;

  Patch() : name_{"Default"} { InitWaveTables(*this); }

  int number() const { return number_; } // Not persistent
  const char *name() const { return name_; }

  Parameters parameters;
  WaveTable wavetables[kNumWaveTables];

  static constexpr size_t kStorageSize =    //
      4 + 4 + 4 + kPatchNameLength          // header
      + ((1 + kNumGlobalParameters)         //
         + (3 * (1 + kNumVoiceParameters))  //
         + (kNumLfos * (1 + kNumLfoParameters))) *
            sizeof(parameter_value_type)  //
      + (kNumWaveTables * WaveTable::kStorageSize);

  bool Save(util::StreamBufferWriter &sbw) const;
  bool Load(util::StreamBufferReader &sbr);

private:
  char name_[kPatchNameLength] = {0};
  int number_ = 0;

  friend class PatchBank;
  friend class PatchBrowser;

  void set_number(int n) { number_ = n; }
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PATCH_H_
