#include "fmt/core.h"
#include "gtest/gtest.h"
#include "misc/fixed_point.h"

namespace pfm2sid::test {

TEST(FixedPointTest, s824)
{
  util::s824 value;
  EXPECT_EQ(0, value.value());

  value.assign(std::numeric_limits<int32_t>::max());
  EXPECT_EQ(std::numeric_limits<int32_t>::max(), value.value());
  EXPECT_EQ(std::numeric_limits<int8_t>::max(), value.integral());

  value.assign(std::numeric_limits<int32_t>::min());
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), value.value());
  EXPECT_EQ(std::numeric_limits<int8_t>::min(), value.integral());

  value = util::s824::from_integral(127);
  EXPECT_EQ(std::numeric_limits<int8_t>::max(), value.integral());
  EXPECT_EQ(0, value.fractional());

  value.assign(0x7f12'3456);
  EXPECT_EQ(0x12'3456, value.fractional());

  value = value.from_float(1.f);
  EXPECT_EQ(1, value.integral());
  EXPECT_EQ(0, value.fractional());
}

TEST(FixedPointTest, u824)
{
  util::u824 value;
  EXPECT_EQ(0U, value.value());
}

TEST(FixedPointTest, s816)
{
  util::s816 value;
  value.assign(0x7fff'ffff);
  EXPECT_EQ(0x7f, value.integral());
  EXPECT_EQ(0xffff, value.fractional());

  value.assign(0xffff);
  auto i = value.scale(1024);
  EXPECT_EQ(1023, i);

  value = 1;
  i = value.scale(1024);
  EXPECT_EQ(1024, i);

  value = -1;
  i = value.scale(1024);
  EXPECT_EQ(-1024, i);
}

TEST(FixedPointTest, s1616)
{
  util::s1616 value;
  EXPECT_EQ(0, value.value());

  value = util::s1616::from_float(1.f);
  EXPECT_EQ(1, value.integral());
  EXPECT_EQ(0, value.fractional());

  value = util::s1616::from_float(32767.f);
  EXPECT_EQ(32767, value.integral());
  EXPECT_EQ(0, value.fractional());

  value = util::s1616::from_float_clamped(32768.f);
  EXPECT_EQ(32767, value.integral());

  value = util::s1616::from_float(.5f);
  EXPECT_EQ(0, value.integral());
  EXPECT_EQ(0x8000, value.fractional());  // TODO is this correct?

  value = util::s1616::from_float(-1.f);
  EXPECT_EQ(-1, value.integral());
  EXPECT_EQ(0, value.fractional());

  value = util::s1616::from_float(-32768.f);
  EXPECT_EQ(-32768, value.integral());
}

}  // namespace pfm2sid::test
