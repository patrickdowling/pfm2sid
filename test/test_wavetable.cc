#include "fmt/core.h"
#include "fmt/format.h"
#include "gtest/gtest.h"
#include "synth/wavetable.h"

template <>
class fmt::formatter<pfm2sid::synth::WaveTable::Entry> {
public:
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(pfm2sid::synth::WaveTable::Entry& entry, Context& ctx) const
  {
    char buf[5] = {0};
    return format_to(ctx.out(), "[ {} {:+2} {} ]", pfm2sid::synth::action_to_string(entry, buf),
                     entry.transpose, static_cast<uint8_t>(entry.waveform));
  }
};

namespace pfm2sid::test {

using synth::WaveTable;
using synth::WaveTableScanner;

TEST(WaveTableTest, Basics)
{
  WaveTable wt;

  int i = 0;
  for (auto s : wt) {
    if (i++ < 8) fmt::println("{:2x} {}", i, s);

    EXPECT_TRUE(WaveTable::END == s.action);
    EXPECT_EQ(0, s.transpose);
    EXPECT_EQ(sidbits::OSC_WAVE::SILENCE, s.waveform);
  }
}

TEST(WaveTableTest, Scanner)
{
  WaveTable wt;
  WaveTableScanner wts;

  auto& data = wt.mutable_data();
  data[0] = {60, sidbits::OSC_WAVE::SAW};
  data[1] = {61, sidbits::OSC_WAVE::SAW};
  data[2] = {WaveTable::LOOP};

  EXPECT_FALSE(wts.active());

  wts.SetSource(&wt);
  wts.set_rate(WaveTableScanner::kRateMax);
  EXPECT_TRUE(wts.active());

  for (int i = 0; i < 4; ++i) {
    auto step = wts.Update();
    fmt::println("{:2x} {}", i, step);
    EXPECT_EQ(WaveTable::PLAY, step.action);
  }
}
}  // namespace pfm2sid::test
