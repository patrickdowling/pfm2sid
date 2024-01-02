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
#include "wavetable.h"

#include <ios>

namespace pfm2sid::synth {

// std::array<WaveTable, kNumWaveTables> wavetables = {
WaveTable wavetables[kNumWaveTables];

void InitWaveTables()
{
  {
    auto &data = wavetables[0].mutable_data();
    data[0] = {0};
    data[1] = {12};
    data[2] = {0};
    data[3] = {12};
    data[4] = {WaveTable::LOOP};
  }

  {
    auto &data = wavetables[1].mutable_data();
    data[0] = {0, sidbits::OSC_WAVE::TRI};
    data[1] = {0, sidbits::OSC_WAVE::NOISE};
    data[2] = {0, sidbits::OSC_WAVE::PULSE};
    data[3] = {-24, sidbits::OSC_WAVE::PULSE};
    data[3].action = WaveTable::END;
  }

  {
    auto &data = wavetables[2].mutable_data();
    for (int i = 0; i <= 12; ++i) { data[i] = {i}; }
    data[12].action = WaveTable::END;
  }

  {
    auto &data = wavetables[3].mutable_data();
    for (int i = 0; i <= 12; ++i) { data[i] = {-i}; }
    data[12].action = WaveTable::END;
  }
}

// TODO This seems overly complex
WaveTable::Entry WaveTableScanner::Update()
{
  auto pos = pos_;
  auto action = source_->at(pos).action;
  if (WaveTable::LOOP == action) {
    pos = pos_ = 0;
    action = source_->at(pos).action;
  }

  bool ticked = true;
  if (rate_) {
    if (ticks_) {
      --ticks_;
      ticked = false;
    } else {
      ticks_ = rate_;
    }
  }

  if (ticked && WaveTable::END != action) {
    if (pos_ < source_->size() - 1) ++pos_;
  }

  return source_->at(pos);
}
}  // namespace pfm2sid::synth
