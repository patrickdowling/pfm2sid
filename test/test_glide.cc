#include "fmt/core.h"
#include "gtest/gtest.h"
#include "synth/glide.h"
#include "synth/synth.h"

namespace pfm2sid::test {

using synth::Glide;

TEST(GlideTest, Basics)
{
  Glide glide;
  EXPECT_FALSE(glide.active());
}

TEST(GlideTest, off)
{
  Glide glide;

  glide.Init(63, 0);
  EXPECT_FALSE(glide.active());

  auto note = glide.Update();
  EXPECT_EQ(63, note.integral());
}

TEST(GlideTest, active)
{
  Glide glide;
  glide.Init(64, 1);
  EXPECT_TRUE(glide.active());
  auto note = glide.Update();
  EXPECT_NE(64, note.integral());

  int i = 0;
  do {
    note = glide.Update();
    ++i;
  } while (note.integral() != 64);
  fmt::println("->64 {}", i);
  EXPECT_FALSE(glide.active());
  EXPECT_EQ(64, note.integral());
  EXPECT_EQ(0, note.fractional());

  glide.Init(0, 1);
  EXPECT_TRUE(glide.active());
  i = 0;
  do {
    note = glide.Update();
    ++i;
  } while (note.value() != 0);
  fmt::println("->0 {}", i);
  EXPECT_FALSE(glide.active());
  EXPECT_EQ(0, note.integral());
  EXPECT_EQ(0, note.fractional());
}

static void print_rate(int32_t rate)
{
  auto ph = Glide::glide_increment(rate);
  auto time = static_cast<float>(Glide::value_type::kFracMult) / static_cast<float>(ph) /
              synth::kModulatorUpdateRateHz;
  fmt::print("| {:3} {:6} {:6.2}", rate, ph, time);
}

TEST(GlideTest, glide_increment)
{
  int i = 0;
  int32_t last = 0;
  for (int rate = 0; rate < 128; ++rate) {
    print_rate(rate);
    auto inc = Glide::glide_increment(rate);
    if (rate > 0 && last == inc) {
      fmt::println("");
      ASSERT_NE(last, inc) << "rate=" << rate;
    }
    last = inc;
    if (i < 11) {
      ++i;
    } else {
      i = 0;
      fmt::println("|");
    }
  }
  fmt::println("|");
}

}  // namespace pfm2sid::test
