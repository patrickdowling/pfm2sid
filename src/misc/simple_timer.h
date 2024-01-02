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
#ifndef UTIL_TIMER_H_
#define UTIL_TIMER_H_

#include <cstdint>
#include <type_traits>

namespace util {

// These are something like "user space timers", i.e. they aren't particularly precise, but helpful
// to generate basic timeouts and things to sequence events.

template <typename id_type, id_type zero>
class SimpleTimer {
public:
  auto id() const { return id_; }
  bool enabled() const { return timeout_ > 0; }
  bool elapsed() const { return elapsed_; }

  void Arm(id_type id, uint32_t start, uint32_t timeout)
  {
    id_ = id;
    start_ = start;
    timeout_ = timeout;
    elapsed_ = false;
  }

  void Update(uint32_t now)
  {
    uint32_t timeout = timeout_;
    if (timeout) {
      if (now - start_ > timeout) {
        elapsed_ = true;
        timeout_ = 0;
      }
    }
  }

  void Reset()
  {
    id_ = zero;
    timeout_ = 0;
    elapsed_ = false;
  }

private:
  id_type id_ = zero;
  uint32_t start_ = 0;
  uint32_t timeout_ = 0;
  bool elapsed_ = false;
};

}  // namespace util

#endif  // UTIL_TIMER_H_
