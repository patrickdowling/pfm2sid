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
#ifndef PFM2SID_MISC_SORTED_ARRAY_H_
#define PFM2SID_MISC_SORTED_ARRAY_H_

#include <array>

namespace util {

// Maintain an vector-ish array of items in sorted order.
// TODO guarantees about uniqueness of values?
template <typename T, size_t N>
class StaticSortedArray {
public:
  constexpr auto size() const { return size_; }
  constexpr auto capacity() const { return N; }

  void clear() { size_ = 0; }

  template <typename X>
  void erase(const X &value)
  {
    size_t pos = 0;
    for (; pos < size_; ++pos)
      if (values_[pos] == value) break;

    if (pos < size_) {
      --size_;
      for (; pos < size_; ++pos) values_[pos] = values_[pos + 1];
    }
  }

  // Yep, good ol' brutey-forcey.
  // There are nice things like std::lower_bound but
  // a) we're assuming there are few items where O(N) doesn't matter.
  // b) we have to move items to insert anyway
  void insert(const T &value)
  {
    // find insert position
    size_t insert_pos = size_;
    for (size_t pos = 0; pos < size_; ++pos) {
      if (values_[pos] == value) return;
      if (values_[pos] >= value) {
        insert_pos = pos;
        break;
      }
    }

    if (size() < capacity()) {
      for (size_t pos = size_; pos > insert_pos; --pos) values_[pos] = values_[pos - 1];
      values_[insert_pos] = value;
      ++size_;
    }
  }

  auto front() const { return values_[0]; }
  auto back() const { return values_[size_ - 1]; }

  auto begin() { return values_.begin(); }
  auto end() { return values_.begin() + size_; }
  auto begin() const { return values_.begin(); }
  auto end() const { return values_.begin() + size_; }

private:
  std::array<T, N> values_ = {};
  size_t size_ = 0;
};

}  // namespace util

#endif  // PFM2SID_MISC_SORTED_ARRAY_H_
