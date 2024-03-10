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
#ifndef PFM2SID_SAMPLE_BUFFER_H_
#define PFM2SID_SAMPLE_BUFFER_H_

#include <cstdint>
#include <memory>

#include "util/util_templates.h"

namespace pfm2sid::synth {

// This is a somewhat special purpose ring buffer or fifo.
// It makes a bunch of assumptions about the use cases...
//
// Technically nothing _needs_ to be power-of-two, but it makes the % operation so simple.
//
template <typename T, size_t sample_block_size, size_t num_sample_blocks>
class SampleBufferT {
public:
  static_assert(util::has_single_bit(sample_block_size));
  static_assert(util::has_single_bit(num_sample_blocks));

  constexpr size_t block_size() const { return sample_block_size; }
  constexpr size_t num_blocks() const { return num_sample_blocks; }

  // size_t readable() const { return write_pos_ - read_pos_; }
  bool writeable() const { return read_block_ != write_block_; }

  template <typename type>
  struct Block {
    type* const begin_;
    type* const end_;

    auto operator[](size_t i) const { return begin_[i]; }

    auto begin() const { return begin_; }
    auto end() const { return end_; }
  };
  using MutableBlock = Block<T>;
  using ConstBlock = Block<const T>;

  MutableBlock WriteableBlock()
  {
    auto begin = buffers_[write_block_];
    return {begin, begin + sample_block_size};
  }

  template <size_t N>
  void Commit(/*MutableBlock*/)
  {
    static_assert(N == sample_block_size);
    write_block_ = (write_block_ + 1) % num_sample_blocks;
  }

  template <size_t N>
  ConstBlock ReadableBlock() const
  {
    static_assert(1 == N);
    auto begin = buffers_[read_block_] + read_pos_;
    return {begin, begin + N};
  }

  template <size_t N>
  auto Consume(/*ConstBlock*/)
  {
    static_assert(1 == N || sample_block_size == N);
    auto read_pos = (read_pos_ + N) % sample_block_size;
    bool next_block = 0 == read_pos;
    if (next_block) read_block_ = (read_block_ + 1) % num_sample_blocks;
    read_pos_ = read_pos;
    return next_block;
  }

private:
  // TODO More correcter would be to use std::atomic, but there's no also cache or DMA here...
  // (famous last words).

  size_t read_pos_ = 0;
  size_t read_block_ = 0;
  size_t write_block_ = num_sample_blocks / 2;

  T buffers_[num_sample_blocks][sample_block_size];
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SAMPLE_BUFFER_H_
