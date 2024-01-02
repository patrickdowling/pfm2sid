#include "fmt/core.h"
#include "gtest/gtest.h"
#include "synth/lfo.h"

namespace pfm2sid::test {

using synth::Lfo;

TEST(LfoTest, phase_increment)
{
  int i = 0;
  for (int rate = 0; rate < 128; ++rate) {
    auto f = Lfo::lfo_freq(rate);
    auto ph = Lfo::phase_increment(rate);
    fmt::print("| {:3} {:7.4f} {:.6f} ", rate, f, ph);
    if (i < 11) {
      ++i;
    } else {
      i = 0;
      fmt::println("|");
    }
  }
  fmt::println("|");
}

TEST(LfoTest, ramp)
{
  synth::Lfo lfo;

  auto expected_steps = 1 + static_cast<int>(1.f / Lfo::phase_increment(127) + .5f);

  auto value = lfo.Update(127, Lfo::SHAPE::RAMP, Lfo::ABS{false});
  EXPECT_LT(value, 1.f);

  int steps = 0;
  do {
    value = lfo.Update(127, Lfo::SHAPE::RAMP, Lfo::ABS{false});
    ++steps;
  } while (steps < 1024 && value < 1.f);

  EXPECT_NE(steps, 0);
  EXPECT_EQ(steps, expected_steps);
  EXPECT_EQ(value, 1.f);

  while (steps++ < 1024) value = lfo.Update(127, Lfo::SHAPE::RAMP, Lfo::ABS{false});
  EXPECT_EQ(value, 1.f);
}

}  // namespace pfm2sid::test
