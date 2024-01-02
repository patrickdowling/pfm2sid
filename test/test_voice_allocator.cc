#include "fmt/core.h"
#include "gtest/gtest.h"
#include "synth/voice_allocator.h"

namespace pfm2sid::test {

using synth::VoiceAllocatorMono;

template <typename T>
void Dump(const T &container)
{
  fmt::print("size={} [ ", container.size());
  for (auto &v : container) fmt::print("{} ", v);
  fmt::println("]");
}

template <typename T>
void DumpVoices(const T &container)
{
  fmt::print("\nsize={} [ ", container.size());
  for (auto &v : container) fmt::print("(note={}, vel={}) ", v.note, v.velocity);
  fmt::println("]");
}

TEST(VoiceAllocatorTest, Mono)
{
  VoiceAllocatorMono<3> voice_allocator;
  EXPECT_EQ(0, voice_allocator.size());

  EXPECT_FALSE(voice_allocator.NoteOff(0));
  EXPECT_EQ(0, voice_allocator.size());

  voice_allocator.NoteOn(63, 127);
  EXPECT_EQ(63, voice_allocator.active_note().note);
  EXPECT_EQ(1, voice_allocator.size());

  EXPECT_TRUE(voice_allocator.NoteOff(63));
  EXPECT_EQ(0, voice_allocator.size());

  voice_allocator.NoteOn(63, 1);
  voice_allocator.NoteOn(64, 2);
  voice_allocator.NoteOn(65, 3);
  voice_allocator.NoteOn(66, 4);
  EXPECT_EQ(3, voice_allocator.size());

  EXPECT_EQ(66, voice_allocator.active_note().note);
  EXPECT_EQ(4, voice_allocator.active_note().velocity);
  EXPECT_TRUE(voice_allocator.NoteOff(66));
  EXPECT_EQ(65, voice_allocator.active_note().note);
  EXPECT_EQ(3, voice_allocator.active_note().velocity);
  EXPECT_TRUE(voice_allocator.NoteOff(65));
  EXPECT_EQ(64, voice_allocator.active_note().note);
  EXPECT_EQ(2, voice_allocator.active_note().velocity);

  EXPECT_TRUE(voice_allocator.NoteOff(64));
  EXPECT_EQ(0, voice_allocator.size());
}

TEST(VoiceAllocatorTest, Poly)
{
  synth::VoiceAllocator<3, synth::STEAL_STRATEGY::LRU> voice_allocator;

  auto v = voice_allocator.Find(63);
  EXPECT_FALSE(v);

  v = voice_allocator.NoteOn(63, 127);
  ASSERT_TRUE(v);

  v = voice_allocator.NoteOn(64, 123);
  ASSERT_TRUE(v);

  v = voice_allocator.NoteOn(65, 123);
  ASSERT_TRUE(v);
  DumpVoices(voice_allocator.voices());
  Dump(voice_allocator.lru());

  v = voice_allocator.NoteOff(65);
  DumpVoices(voice_allocator.voices());
  Dump(voice_allocator.lru());

  v = voice_allocator.NoteOn(65, 123);
  ASSERT_TRUE(v);
  DumpVoices(voice_allocator.voices());
  Dump(voice_allocator.lru());
}

}  // namespace pfm2sid::test
