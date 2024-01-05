#include "fmt/core.h"
#include "gtest/gtest.h"

#undef RESID_FILTER_CONSTEXPR  // just in case
#include "reSID/src/filter.h"
#include "reSID/src/spline.h"

namespace pfm2sid::test {

// Some of the tables in the reSID filter are calculated at runtime, but are really static.
// This is a proof-of-concept of how to constexpr the whole thing, which seems feasible for >=
// c++17. Definitely falls in the category of "works for me" with a pinch of hack.

using reSID::fc_point;
using reSID::Filter;
using reSID::PointPlotter;
using reSID::sound_sample;

static constexpr fc_point f0_points_6581[] = {
    //  FC      f         FCHI FCLO
    // ----------------------------
    {0, 220},       // 0x00      - repeated end point
    {0, 220},       // 0x00
    {128, 230},     // 0x10
    {256, 250},     // 0x20
    {384, 300},     // 0x30
    {512, 420},     // 0x40
    {640, 780},     // 0x50
    {768, 1600},    // 0x60
    {832, 2300},    // 0x68
    {896, 3200},    // 0x70
    {960, 4300},    // 0x78
    {992, 5000},    // 0x7c
    {1008, 5400},   // 0x7e
    {1016, 5700},   // 0x7f
    {1023, 6000},   // 0x7f 0x07
    {1023, 6000},   // 0x7f 0x07 - discontinuity
    {1024, 4600},   // 0x80      -
    {1024, 4600},   // 0x80
    {1032, 4800},   // 0x81
    {1056, 5300},   // 0x84
    {1088, 6000},   // 0x88
    {1120, 6600},   // 0x8c
    {1152, 7200},   // 0x90
    {1280, 9500},   // 0xa0
    {1408, 12000},  // 0xb0
    {1536, 14500},  // 0xc0
    {1664, 16000},  // 0xd0
    {1792, 17100},  // 0xe0
    {1920, 17700},  // 0xf0
    {2047, 18000},  // 0xff 0x07
    {2047, 18000}   // 0xff 0x07 - repeated end point
};

constexpr auto generate_f0_table(const fc_point* src, unsigned N)
{
  // The magic of auto...
  struct {
    sound_sample data[2048] = {};
    operator const sound_sample*() const { return data; }
  } table;
  reSID::interpolate(src, src + N - 1, PointPlotter<sound_sample>(table.data), 1.0);
  return table;
}

static constexpr auto f0_table =
    generate_f0_table(f0_points_6581, sizeof(f0_points_6581) / sizeof(*f0_points_6581));

TEST(reSIDTest, constexprFilterTables)
{
  Filter filter;

  int i = 0;
  for (auto value : f0_table.data) {
    EXPECT_EQ(filter.get_f0()[i], value);
    ++i;
  }
}

TEST(reSIDTest, DISABLED_contexprFilterTablesPrint)
{
  int i = 0;
  for (auto value : f0_table.data) {
    fmt::print(" 0x{:04x}", value);
    if (i < 15) {
      ++i;
    } else {
      i = 0;
      fmt::println("");
    }
  }
  fmt::println("|");
}

}  // namespace pfm2sid::test
