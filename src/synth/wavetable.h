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
#ifndef PFM2SID_SYNTH_WAVETABLE_H_
#define PFM2SID_SYNTH_WAVETABLE_H_

#include <array>
#include <cstdint>

#include "sidbits/sidbits.h"

namespace pfm2sid::synth {

static constexpr size_t kNumWaveTables = 4;
// Can we afford to be a bit wasteful with the types?
//
// This uses fixed-purpose fields rather than generic ones that can be assigned to parameters/CC,
// which can be added later.
//
// TODO Move the enable_ flags to the scanner and make it parametrizable?
// TODO Store an array of values and use detail::typed_value (\sa parameter_types.h_ to convert

class WaveTable {
public:
  static constexpr size_t kMaxSteps = 32;

  enum ACTION { PLAY, LOOP, END };
  enum TRACK : unsigned { TRANSPOSE, WAVEFORM };

  template <WaveTable::TRACK track>
  static constexpr auto track_mask() -> unsigned
  {
    return 1 << track;
  }

  struct Entry {
    ACTION action = END;

    int16_t transpose = 0;
    sidbits::OSC_WAVE waveform = sidbits::OSC_WAVE::SILENCE;

    // template <TRACK track> auto get() const { return values_[track]; }
    constexpr Entry() = default;
    constexpr Entry(ACTION a) : action(a) {}
    constexpr Entry(int t, sidbits::OSC_WAVE w) : action(PLAY), transpose(t), waveform(w) {}
    constexpr Entry(int t) : action(PLAY), transpose(t) {}
  };

  auto size() const { return kMaxSteps; }

  auto at(size_t pos) const { return data_[pos]; }

  auto begin() const { return data_.begin(); }
  auto end() const { return data_.end(); }

  auto &mutable_data() { return data_; }

  template <WaveTable::TRACK track>
  void enable(bool enable)
  {
    if (enable)
      enabled_tracks_ |= track_mask<track>();
    else
      enabled_tracks_ &= ~track_mask<track>();
  }

  template <WaveTable::TRACK track>
  constexpr auto is_enabled() const -> bool
  {
    return enabled_tracks_ & track_mask<track>();
  }

private:
  std::array<Entry, kMaxSteps> data_;
  unsigned enabled_tracks_ = 0;
};

class Patch;
void InitWaveTables(Patch &);

class WaveTableScanner {
public:
  static constexpr int32_t kRateMin = 0;
  static constexpr int32_t kRateMax = 127;

  void SetSource(const WaveTable *src)
  {
    source_ = src;
    pos_ = 0;
    ticks_ = rate_;
  }

  void set_rate(int32_t rate)
  {
    rate = (kRateMax - std::clamp(rate, kRateMin, kRateMax));
    if (ticks_ > rate) ticks_ = rate;
    rate_ = rate;
  }

  void Reset()
  {
    SetSource(nullptr);
    set_rate(0);
    ticks_ = 0;
  }

  bool active() const { return nullptr != source_; }

  auto Update() -> WaveTable::Entry;

  template <WaveTable::TRACK track>
  constexpr auto is_enabled() const -> bool
  {
    return source_ && source_->is_enabled<track>();
  }

private:
  const WaveTable *source_ = nullptr;
  size_t pos_ = 0;
  int32_t rate_ = kRateMax;
  int32_t ticks_ = kRateMax;
};

constexpr const char *action_to_string(const WaveTable::Entry &entry, char *buf)
{
  if (WaveTable::PLAY == entry.action)
    buf[0] = 'P';
  else if (WaveTable::LOOP == entry.action)
    buf[0] = 'L';
  else if (WaveTable::END == entry.action)
    buf[0] = 'E';
  else
    buf[0] = '?';

  return buf;
}

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_WAVETABLE_H_
