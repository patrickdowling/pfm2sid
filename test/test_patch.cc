#include "fmt/core.h"
#include "gtest/gtest.h"
#include "synth/patch.h"
#include "synth/patch_bank.h"
#include "pfm2sid_test.h"

namespace pfm2sid::test {

TEST(PatchTest, basics)
{
  fmt::println("sizeof(Patch)={}", sizeof(synth::Patch));
  fmt::println("sizeof(PatchBank)={}", sizeof(synth::PatchBank));
  fmt::println("sizeof(ParameterValue)={}", sizeof(synth::ParameterValue));
}

TEST(PatchTest, Save)
{
  using synth::Patch;
  Patch patch;
  fmt::println("sizeof(Patch.parameters)={}", sizeof(patch.parameters));
  fmt::println("Patch::kStorageSize={}", Patch::kStorageSize);
  fmt::println("Wavetable::kStorageSize={}", synth::WaveTable::kStorageSize);

  static constexpr size_t kBufferSize = 64 * 1024;
  std::vector<uint8_t> buf(kBufferSize);

  util::StreamBufferWriter writer{buf.data(), buf.size()};
  EXPECT_EQ(writer.available(), buf.size());

  patch.Save(writer);
  EXPECT_FALSE(writer.overflow());
  EXPECT_EQ(Patch::kStorageSize, writer.written());
  fmt::println("writer.written={}", writer.written());

  Hexdump(buf.data(), writer.written());

  util::StreamBufferReader reader{buf.data(), buf.size()};
  EXPECT_EQ(reader.available(), buf.size());

  bool loaded = patch.Load(reader);
  EXPECT_TRUE(loaded);
  EXPECT_EQ(reader.read(), writer.written());
  fmt::println("reader.read={}", reader.read());
}

}  // namespace pfm2sid::test
