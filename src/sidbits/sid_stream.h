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
#ifndef PFM2SID_SID_STREAM_H_
#define PFM2SID_SID_STREAM_H_

#include <algorithm>
#include <array>

#include "sidbits.h"

namespace pfm2sid::sidbits {

struct SIDStreamData {
  const uint8_t *start = nullptr;
  const uint8_t *end = nullptr;
};

// Play a SID file from memory
class SIDStream {
public:
  SIDStream() = default;

  size_t size() const { return stream_data_.end - stream_data_.start; }
  size_t pos() const { return pos_; }

  float percent() const { return 100.f * static_cast<float>(pos_) / static_cast<float>(size()); }

  void Init(const SIDStreamData &stream_data)
  {
    stream_data_ = stream_data;
    pos_ = 0;
  }

  void Reset()
  {
    pos_ = 0;
    register_map_.Reset();
  }

  void Step()
  {
    if (!stream_data_.start) return;

    for (uint8_t r = 0; r < sidbits::SID_REGISTER_COUNT; ++r) {
      auto value = stream_data_.start[pos_++];
      register_map_.poke(r, value);
    }

    if (pos_ >= size()) { pos_ = 0; }
  }

  auto register_map() const { return register_map_; }

private:
  SIDStreamData stream_data_;
  size_t pos_ = 0;

  sidbits::RegisterMap register_map_;
};

}  // namespace pfm2sid::sidbits

#endif  // PFM2SID_SID_STREAM_H_
