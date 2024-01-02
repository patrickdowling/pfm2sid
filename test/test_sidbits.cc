#include "fmt/core.h"
#include "gtest/gtest.h"
#include "midi/midi_types.h"
#include "sidbits/sidbits.h"

namespace pfm2sid::test {

void DumpRegisterMap(const sidbits::RegisterMap &register_map)
{
  int i = 0;
  for (size_t r = 0; r < sidbits::RegisterMap::kNumRegisters; ++r) {
    fmt::print("{:02x}", register_map.peek(r));
    ++i;
    if (i < (int)sidbits::RegisterMap::VOICE_REG_COUNT) {
      fmt::print(" ");
    } else {
      i = 0;
      fmt::println("");
    }
  }
  fmt::println("");
}

TEST(sidbitsTest, RegisterMap)
{
  using namespace sidbits;
  RegisterMap register_map;
  for (size_t i = 0; i < sidbits::RegisterMap::kNumRegisters; ++i) {
    EXPECT_EQ(0, register_map.peek(i));
  }

  register_map.voice_set_freq(sidbits::VOICE1, 0xabcd);
  register_map.voice_set_waveform(sidbits::VOICE1, OSC_WAVE::NOISE);

  register_map.voice_set_freq(sidbits::VOICE2, 0xbeef);
  register_map.voice_set_waveform(sidbits::VOICE2, OSC_WAVE::PULSE);

  register_map.voice_set_freq(sidbits::VOICE3, 0x1234);
  register_map.voice_set_waveform(sidbits::VOICE3, OSC_WAVE::SAW);
  DumpRegisterMap(register_map);

  EXPECT_EQ((uint8_t)OSC_WAVE::NOISE, register_map.peek(4));
  EXPECT_EQ((uint8_t)OSC_WAVE::PULSE, register_map.peek(11));
  EXPECT_EQ((uint8_t)OSC_WAVE::SAW, register_map.peek(18));
}

TEST(sidbitsTest, midi_to_osc_freq)
{
  for (int i = 0, lc = 0; i < 127; ++i) {
    auto f = sidbits::midi_to_osc_freq(i);
    fmt::print("| {:04x} {:05} ", f, f);

    if (lc < 11) {
      ++lc;
    } else {
      lc = 0;
      fmt::println("|");
    }
  }
  fmt::println("|");
}

TEST(sidbitsTest, midi_to_filter_freq)
{
  auto f = sidbits::midi_to_filter_freq(0, 0);
  EXPECT_EQ(0, f);

  f = sidbits::midi_to_filter_freq(midi::C0, 0);
  EXPECT_EQ(0, f);

  for (int32_t i = midi::C0; i < 128; ++i) {
    fmt::print("note={:2}", i);
    for (int32_t r = 0; r <= 64; r += 4) {
      auto s = fmt::format("{}={}", r, sidbits::midi_to_filter_freq(i, r));
      fmt::print(" {:8}", s);
    }
    fmt::println("");
  }
}

}  // namespace pfm2sid::test
