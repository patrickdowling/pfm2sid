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

namespace pfm2sid::synth {

class PatchBank;
class PatchBrowser;

class Patch {
public:
  Patch() : name_{"Default"} {}

  int number() const { return number_; }
  const char *name() const { return name_; }

  Parameters parameters;
  WaveTable wavetables[kNumWaveTables];

private:
  char name_[kMaxNameLength] = {0};
  int number_ = 0;

  friend class PatchBank;
  friend class PatchBrowser;

  void set_number(int n) { number_ = n; }
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PATCH_H_
