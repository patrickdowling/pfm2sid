#include <fmt/core.h>

#include <vector>

#include "gtest/gtest.h"
#include "midi/midi_parser.h"

namespace pfm2sid::test {

class MidiHandler : public midi::MidiHandler {
public:
  MidiHandler() : midi::MidiHandler(midi::ALL_CHANNELS) {}
  void MidiNoteOff(midi::Channel channel, midi::Note note, midi::Velocity velocity) final
  {
    fmt::println("NOTE OFF {}, {}, {}", channel, note, velocity);
    notes_.push_back({false, channel, note, velocity});
  }
  void MidiNoteOn(midi::Channel channel, midi::Note note, midi::Velocity velocity) final
  {
    fmt::println("NOTE ON {}, {}, {}", channel, note, velocity);
    notes_.push_back({true, channel, note, velocity});
  }
  void MidiPitchbend(midi::Channel channel, int16_t value) final
  {
    fmt::println("BEND {}, {}", channel, value);
    pitch_bend_ = value;
  }

  struct Note {
    bool on = false;
    midi::Channel channel = 0;
    midi::Note note = 0;
    midi::Velocity velocity = 0;
  };

  std::vector<Note> notes_;
  int16_t pitch_bend_ = 0;
};

class MidiHandlerRT : public midi::MidiHandlerRT {
public:
  void MidiClock() final { ++counters.midi_clock; }
  void MidiStart() final { ++counters.midi_start; }
  void MidiContinue() final { ++counters.midi_continue; }
  void MidiStop() final { ++counters.midi_stop; }
  void MidiActiveSensing() final { ++counters.midi_act_sense; }
  void MidiReset() final { ++counters.midi_reset; }

  struct {
    int midi_clock = 0;
    int midi_start = 0;
    int midi_continue = 0;
    int midi_stop = 0;
    int midi_act_sense = 0;
    int midi_reset = 0;
  } counters;
};

class MidiTest : public ::testing::Test {
public:
  void SetUp() final { midi_parser_.Reset(); }

  void TearDown() final {}

protected:
  midi::MidiParser midi_parser_;
  MidiHandler midi_handler_;
  MidiHandlerRT midi_handler_rt_;
};

TEST_F(MidiTest, Sanity)
{
  auto state = midi_parser_.current_state();
  EXPECT_EQ(0U, state.running_status);
  EXPECT_EQ(0U, state.msg_len);
  EXPECT_EQ(0U, state.expected_msg_len);
  EXPECT_EQ(midi::SYSEX_STATUS::IDLE, state.sysex_status);
}

TEST_F(MidiTest, RT)
{
  midi_parser_.Init({nullptr, nullptr, &midi_handler_rt_});

  for (int byte = midi::RT_CLOCK; byte <= midi::RT_SYS_RESET; ++byte) midi_parser_.Parse(byte);

  EXPECT_EQ(1, midi_handler_rt_.counters.midi_clock);
  EXPECT_EQ(1, midi_handler_rt_.counters.midi_start);
  EXPECT_EQ(1, midi_handler_rt_.counters.midi_continue);
  EXPECT_EQ(1, midi_handler_rt_.counters.midi_stop);
  EXPECT_EQ(1, midi_handler_rt_.counters.midi_act_sense);
  EXPECT_EQ(1, midi_handler_rt_.counters.midi_reset);
}

TEST_F(MidiTest, NoteOnOff)
{
  midi_parser_.Init({&midi_handler_});

  const uint8_t data[] = {0x91, 0x3F, 0x7F, 0x81, 0x3F, 0x7f};

  midi_parser_.Parse(data, sizeof(data));

  ASSERT_EQ(2, midi_handler_.notes_.size());

  EXPECT_EQ(true, midi_handler_.notes_.front().on);
  EXPECT_EQ(1, midi_handler_.notes_.front().channel);
  EXPECT_EQ(63, midi_handler_.notes_.front().note);
  EXPECT_EQ(127, midi_handler_.notes_.front().velocity);
}

TEST_F(MidiTest, PitchBend)
{
  midi_parser_.Init({&midi_handler_});
  EXPECT_EQ(0, midi_handler_.pitch_bend_);

  {
    const uint8_t data[] = {0xE0, 0x00, 0x00};
    midi_parser_.Parse(data, sizeof(data));

    EXPECT_EQ(-8192, midi_handler_.pitch_bend_);
  }
  {
    const uint8_t data[] = {0xE0, 0x00, 0x40};
    midi_parser_.Parse(data, sizeof(data));

    EXPECT_EQ(0, midi_handler_.pitch_bend_);
  }
  {
    const uint8_t data[] = {0xE0, 0x7f, 0x7f};
    midi_parser_.Parse(data, sizeof(data));

    EXPECT_EQ(8191, midi_handler_.pitch_bend_);
  }
}

TEST_F(MidiTest, Incomplete)
{
  midi_parser_.Init({&midi_handler_});

  const uint8_t data[] = {0x81, 0x3F, 0x91, 0x0f, 0x7F};
  midi_parser_.Parse(data, sizeof(data));

  ASSERT_EQ(1, midi_handler_.notes_.size());

  EXPECT_EQ(true, midi_handler_.notes_.front().on);
  EXPECT_EQ(1, midi_handler_.notes_.front().channel);
  EXPECT_EQ(15, midi_handler_.notes_.front().note);
  EXPECT_EQ(127, midi_handler_.notes_.front().velocity);
}

TEST_F(MidiTest, ChannelFilter)
{
  midi_parser_.Init({&midi_handler_});
  midi_handler_.set_channel_mask(0x2);

  midi_parser_.Parse(0x92);
  midi_parser_.Parse(0x3F);
  midi_parser_.Parse(0x7F);

  EXPECT_TRUE(midi_handler_.notes_.empty());

  midi_parser_.Parse(0x91);
  midi_parser_.Parse(0x3F);
  midi_parser_.Parse(0x7F);

  ASSERT_EQ(1, midi_handler_.notes_.size());
}

TEST_F(MidiTest, RunningStatus)
{
  midi_parser_.Init({&midi_handler_});

  midi_parser_.Parse(0x92);
  midi_parser_.Parse(0x3F);
  midi_parser_.Parse(0x7F);
  midi_parser_.Parse(0x3E);
  midi_parser_.Parse(0x7E);

  ASSERT_EQ(2, midi_handler_.notes_.size());

  EXPECT_TRUE(midi_handler_.notes_[1].on);
  EXPECT_EQ(63, midi_handler_.notes_[0].note);
  EXPECT_EQ(62, midi_handler_.notes_[1].note);
  EXPECT_EQ(127, midi_handler_.notes_[0].velocity);
  EXPECT_EQ(126, midi_handler_.notes_[1].velocity);
}

}  // namespace pfm2sid::test
