#include "fmt/core.h"
#include "gtest/gtest.h"
#include "pfm2sid_test.h"
#include "sidbits/asid_parser.h"

namespace pfm2sid::test {

using sidbits::ASIDParser;

TEST(ASIDParserTest, Basics)
{
  ASIDParser parser;
  EXPECT_FALSE(parser.active());

  {
    auto r = parser.ParseSysex(nullptr, 0);
    EXPECT_FALSE(r);
  }

  {
    const uint8_t data[] = {0x2d, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto r = parser.ParseSysex(data, sizeof(data));
    ASSERT_TRUE(r);
    EXPECT_EQ(r.value(), 0);
  }

  {
    const uint8_t data[] = {0x2d, 0x4e, 0x11, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x7f, 0x70};
    auto r = parser.ParseSysex(data, sizeof(data));
    EXPECT_TRUE(r);
    EXPECT_EQ(r.value(), 2);
    DumpRegisterMap(parser.register_map());
    EXPECT_EQ(0xff, parser.register_map().peek(0));
    EXPECT_EQ(0xf0, parser.register_map().peek(5));
  }
}

TEST(ASIDParserTest, AllRegisters)
{
  ASIDParser parser;
  uint8_t data[2 + 8 + 28] = {0x2d, 0x4e, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00};

  for (uint8_t r = 0x0; r < 28; ++r) data[10 + r] = r & 1 ? 0x80 + r : r;
  auto r = parser.ParseSysex(data, sizeof(data));
  ASSERT_TRUE(r);
  EXPECT_EQ(r.value(), 28);
  DumpRegisterMap(parser.register_map());
}

TEST(ASIDParserTest, lcd)
{
  ASIDParser parser;

  {
    const uint8_t data[] = {0x2d, 0x4f};
    parser.ParseSysex(data, sizeof(data));

    fmt::println("'{}'", parser.lcd_data());
    EXPECT_STREQ("", parser.lcd_data());
  }

  {
    const uint8_t data[] = {0x2d, 0x4f, 'A', 'B', 'C', 'D'};
    parser.ParseSysex(data, sizeof(data));

    fmt::println("'{}'", parser.lcd_data());
    EXPECT_STREQ("ABCD", parser.lcd_data());
  }

  {
    const uint8_t data[40] = {0x2d, 0x4f, '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
                              '.',  '.',  '.', '.', '.', '.', '.', '.', '.', '.', '!'};

    parser.ParseSysex(data, sizeof(data));

    fmt::println("'{}'", parser.lcd_data());
    EXPECT_EQ(strlen(parser.lcd_data()), ASIDParser::LCD_DATA_LEN);
    EXPECT_STREQ("....................", parser.lcd_data());
  }
}

}  // namespace pfm2sid::test
