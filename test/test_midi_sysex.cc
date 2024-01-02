#include <vector>

#include "fmt/core.h"
#include "gtest/gtest.h"
#include "midi/midi_parser.h"

namespace pfm2sid::test {

class MidiHandlerSysex : public midi::MidiHandler {
public:
  MidiHandlerSysex() : midi::MidiHandler(1) {}

  bool MidiSysex(const uint8_t* data, unsigned len, midi::SYSEX_STATUS status)
  {
    Dump(data, len, status);
    switch (status) {
      case midi::SYSEX_STATUS::START: return len > 1 && data[0] == 0x2D;
      case midi::SYSEX_STATUS::EOX:
        std::copy(data, data + len, std::back_inserter(data_));
        return true;
      default: break;
    }
    return false;
  }

  std::vector<uint8_t> data_;

  static void Dump(const uint8_t* data, unsigned len, midi::SYSEX_STATUS status)
  {
    fmt::print("status={} len={}: [ ", to_string(status), len);
    while (len--) fmt::print("{:02X} ", *data++);
    fmt::println("]");
  }
};

class MidiSysexTest : public ::testing::Test {
  void SetUp() final { midi_parser_.Init({&midi_handler_, nullptr, nullptr}); }
  void TearDown() final {}

protected:
  MidiHandlerSysex midi_handler_;
  midi::MidiParser midi_parser_;
};

TEST_F(MidiSysexTest, Basics)
{
  auto state = midi_parser_.current_state();
  EXPECT_EQ(midi::SYSEX_STATUS::IDLE, state.sysex_status);

  const uint8_t data[] = {0xF0, 0x2D, 0x00, 0x01, 0xF7};
  midi_parser_.Parse(data, sizeof(data));

  EXPECT_EQ(midi_handler_.data_.size(), 3);
}

TEST_F(MidiSysexTest, Abort)
{
  const uint8_t data[] = {0xF0, 0x2F, 0x00, 0x00, 0xF7};
  midi_parser_.Parse(data, sizeof(data));

  EXPECT_TRUE(midi_handler_.data_.empty());
}

}  // namespace pfm2sid::test
