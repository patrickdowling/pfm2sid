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
#ifndef PFM2SID_MISC_STATIC_STACK_H_
#define PFM2SID_MISC_STATIC_STACK_H_

#include <array>
#include <memory>
#include <optional>

namespace util {

// Very simple stack-ish structure with fixed-size storage
template <typename T, size_t N>
class StaticStack {
public:
  using iterator = typename std::array<T, N>::iterator;
  using const_iterator = typename std::array<T, N>::const_iterator;

  void clear() { size_ = 0; }

  bool push_back(const T &value) noexcept
  {
    if (size() < capacity()) {
      values_[size_++] = value;
      return true;
    } else {
      return false;
    }
  }

  template <typename... Args>
  bool emplace_back(Args &&...args) noexcept
  {
    if (size() < capacity()) {
      values_[size_++] = {std::forward<Args>(args)...};
      return true;
    } else {
      return false;
    }
  }

  const auto &front() const noexcept { return values_[0]; }

  auto &back() noexcept { return values_[size_ - 1]; }
  const auto &back() const noexcept { return values_[size_ - 1]; }

  T pop() noexcept { return values_[size_--]; }

  bool empty() const noexcept { return size_ < 1; }
  bool full() const noexcept { return size_ == N; }
  auto size() const noexcept { return size_; }
  auto capacity() const noexcept { return N; }

  bool erase(iterator pos) noexcept
  {
    if (pos != end()) {
      std::move(pos + 1, end(), pos);
      --size_;
      return true;
    } else {
      return false;
    }
  }

  auto begin() { return values_.begin(); }
  auto end() { return values_.begin() + size_; }
  auto begin() const { return values_.begin(); }
  auto end() const { return values_.begin() + size_; }

private:
  std::array<T, N> values_;
  size_t size_ = 0;
};

}  // namespace util

#endif  // PFM2SID_MISC_STATIC_STACK_H_
