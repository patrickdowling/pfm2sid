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
#include "sidbits.h"

#include <array>
#include <cmath>
#include <utility>

#include "midi/midi_types.h"
#include "misc/platform.h"
#include "util/util_lut.h"

ENABLE_WCONVERSION();

namespace pfm2sid::sidbits {

// Based on multiple sources, e.g.
// https://codebase64.org/doku.php?id=base:how_to_calculate_your_own_sid_frequency_table
//
// The table could also be a single octave but we're feeling generous and it's 190 (95 x 2) bytes
// or as floats 95x4

static constexpr float BASEFREQ = 440.f;
static constexpr int32_t C0 = -(5 * 12) + 3;
static constexpr int32_t kNumTableEntries = 95;

constexpr float CONSTANT = static_cast<float>((256 << 8) << 8) / CLOCK_FREQ_PAL;

static constexpr float FREQ_HZ(int note)
{
  return BASEFREQ * powf(2.f, static_cast<float>(C0 + note) / 12.f);
}

constexpr auto SID_FREQ(int note, size_t /*N*/)
{
  return static_cast<uint16_t>(CONSTANT * FREQ_HZ(note));
}

LUT_CONSTEXPR auto SID_FREQ_TABLE_U16 =
    util::LookupTable<uint16_t, kNumTableEntries, int32_t>::generate(SID_FREQ);

uint16_t midi_to_osc_freq(int32_t midi_note)
{
  auto idx = (size_t)std::clamp<int32_t>(midi_note - midi::C0, 0, kNumTableEntries - 1);
  return SID_FREQ_TABLE_U16[idx];
}

uint16_t midi_to_osc_freq_fp(util::s816 midi_note)
{
  auto idx = (size_t)std::clamp<int32_t>(midi_note.integral() - midi::C0, 0, kNumTableEntries - 1);

  uint32_t a = SID_FREQ_TABLE_U16[idx];
  uint32_t b = SID_FREQ_TABLE_U16[idx + 1];

  uint32_t interp = ((b - a) * static_cast<uint32_t>(midi_note.fractional())) >> 16;

  return static_cast<uint16_t>(a + interp);
}

int32_t midi_to_filter_freq(int32_t midi_note, int32_t scale)
{
  // This isn't based on frequency, but can be used as an offset to an initial value that gets
  // scaled according to the midi note.
  //
  // Filter range is [0, 1023)
  // Scale is [-64, 64) but we divide by 32 to allow x2

  scale = std::clamp<int32_t>(scale, -64, 63);
  midi_note = std::clamp<int32_t>(midi_note, 0, 127);

  return (midi_note * scale * 1024) / 32 / 128;
}

}  // namespace pfm2sid::sidbits
