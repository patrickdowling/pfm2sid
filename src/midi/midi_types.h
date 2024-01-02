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
#ifndef PFM2SID_MIDI_TYPES_H_
#define PFM2SID_MIDI_TYPES_H_

#include <bitset>
#include <cinttypes>

namespace pfm2sid::midi {
using Channel = uint_fast8_t;
using Note = uint_fast8_t;
using Velocity = uint_fast8_t;

static constexpr unsigned kNumMidiChannels = 16;

using ChannelMask = std::bitset<kNumMidiChannels>;
static constexpr ChannelMask ALL_CHANNELS = 0xffff;

static constexpr Note C0 = 24;
static constexpr Note C4 = 60;
static constexpr Note A4 = 69;

static constexpr Note INVALID_NOTE = 0xff;

}  // namespace pfm2sid::midi

#endif  // PFM2SID_MIDI_TYPES_H_
