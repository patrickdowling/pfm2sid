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
#ifndef PFM2SID_SYNTH_LFO_H_
#define PFM2SID_SYNTH_LFO_H_

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "misc/platform.h"

namespace pfm2sid::synth {

// The actual phase computation might be marginally more efficient with unsigned 8.24 or .32 since
// it can just wrap. But we have floats available, and the value will likely be scaled eventually so
// why not for a change...
//
// Assumes the LFO gets updated once every sample block, not per sample
//
// TODO single shot
// TODO invert/abs
class Lfo {
public:
  Lfo() = default;

  enum SYNC { SYNC_NONE, SYNC_NOTE_ON };
  enum SHAPE { TRI, SAW, SQUARE, SINE, RAMP, RAND };

  enum ABS : bool {};

  void Init(int32_t rate, int start_phase);

  void Reset(int32_t start_phase)
  {
    start_phase = static_cast<float>(std::clamp<int32_t>(start_phase, 0, 127)) / 128.f;
    reset_ = true;
  }

  float Update(int32_t rate, SHAPE shape, ABS abs)
  {
    phase_inc_ = phase_increment(rate);

    float phase;
    if (unlikely(reset_)) {
      phase = phase_ = start_phase_;
      reset_ = false;
    } else {
      phase = phase_;
      auto ph = phase + phase_inc_;
      if (ph >= 1.f) {
        if (RAMP == shape)
          ph = 1.f;
        else
          ph -= 1.f;
      }
      phase_ = ph;
    }

    auto output = 0.f;
    switch (shape) {
      case TRI: output = phase < .5f ? (phase * 4.f - 1.f) : (1.f - (phase - .5f) * 4.f); break;
      case SAW: output = -1.f + phase * 2.f; break;
      case SQUARE: output = phase < .5f ? -1.f : 1.f; break;
      case SINE: output = sine_table(phase); break;
      case RAMP: output = phase; break;
      case RAND: output = 1.f; break;  // TODO
    }
    if (abs) output = fabs(output);
    return output;
  }

private:
  bool reset_ = false;
  float start_phase_ = 0.f;
  float phase_ = 0.f;
  float phase_inc_ = 0.f;

  // Available for tests
public:
  static constexpr size_t kSinTableSize = 1024;

  static float lfo_freq(int32_t rate);
  static float phase_increment(int32_t rate);
  static float sine_table(float phase);
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_LFO_H_
