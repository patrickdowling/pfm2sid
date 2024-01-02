// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2023-2024 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef PFM2SID_SYNTH_VOICE_ALLOCATOR_H_
#define PFM2SID_SYNTH_VOICE_ALLOCATOR_H_

#include <array>
#include <memory>
#include <optional>

#include "midi/midi_types.h"
#include "misc/sorted_array.h"
#include "misc/static_stack.h"

namespace pfm2sid::synth {

using VoiceIndex = unsigned;

struct Note {
  midi::Note note = midi::INVALID_NOTE;
  midi::Velocity velocity = 0;

  bool is_free() const { return note == midi::INVALID_NOTE; }
};

constexpr bool operator==(const Note &lhs, midi::Note rhs)
{
  return lhs.note == rhs;
}

constexpr bool operator<(const Note &lhs, const Note &rhs)
{
  return lhs.note < rhs.note;
}

enum struct STEAL_STRATEGY { NONE, LRU };

// This weird template trickery is to allow the NoteStack to optionally maintain the array of sorted
// notes, or not.
enum struct SORT_NOTES { NO, YES };

template <SORT_NOTES, size_t>
struct SortedNotesImpl {
  static constexpr bool is_sorted = false;
};
template <size_t N>
struct SortedNotesImpl<SORT_NOTES::YES, N> {
  static constexpr bool is_sorted = true;
  util::StaticSortedArray<midi::Note, N> sorted_notes_;
};

// NOTE find, erase etc. can all be implemented in terms of iterators and std::find_if etc.

// This maintains a stack of the most recent notes. On it's own it's good for a monophonic
// allocation scheme. Contains no "strategy" and always deletes the least recently used note if
// full, so if a different strategy is required (e.g. highest has priority) then it may require a
// wrapper.
template <size_t stack_depth, SORT_NOTES sorted = SORT_NOTES::NO>
class NoteStack : public SortedNotesImpl<sorted, stack_depth> {
public:
  using SortedNotes = SortedNotesImpl<sorted, stack_depth>;

  void Init() {}

  void Clear()
  {
    note_stack_.clear();
    if constexpr (SortedNotes::is_sorted) SortedNotes::sorted_notes_.clear();
  }

  void NoteOn(midi::Note note, midi::Velocity velocity) noexcept
  {
    // It's not really clear how a note gets played twice (except if there are multiple sources,
    // like sequencer + keys) but if we remove it, it gets put back on top of the stack...
    if (!note_stack_.erase(Find(note))) {
      if (note_stack_.full()) {  // can still be full if note wasn't found...
        note_stack_.erase(note_stack_.begin());
      }
    }
    note_stack_.emplace_back(note, velocity);
    if constexpr (SortedNotes::is_sorted) SortedNotes::sorted_notes_.insert(note);
  }

  bool NoteOff(midi::Note note) noexcept
  {
    if (note_stack_.erase(Find(note))) {
      if constexpr (SortedNotes::is_sorted) SortedNotes::sorted_notes_.erase(note);
      return true;
    } else {
      return false;
    }
  }

  auto size() const noexcept { return note_stack_.size(); }

  auto active_note() const noexcept { return note_stack_.back(); }

  auto &sorted_notes() const
  {
    if constexpr (SortedNotes::is_sorted)
      return SortedNotes::sorted_notes_;
    else {};
  }

private:
  util::StaticStack<Note, stack_depth> note_stack_;

  auto Find(midi::Note note)
  {
    auto i = note_stack_.begin();
    auto e = note_stack_.end();
    for (; i != e; ++i)
      if (i->note == note) break;
    return i;
  }
};

template <size_t NUM_VOICES, STEAL_STRATEGY strategy = STEAL_STRATEGY::NONE>
class VoiceAllocator {
public:
  static_assert(NUM_VOICES > 0);

  void Clear()
  {
    for (auto &v : voice_pool_) v = {};
    lru_.clear();
    next_ = 0;
  }

  [[nodiscard]] std::optional<VoiceIndex> NoteOn(midi::Note note, midi::Velocity velocity) noexcept
  {
    // Find voice already playing requested note
    auto v = Find(note);
    if (v) {
      lru_.erase(std::find(lru_.begin(), lru_.end(), v.value()));
      AssignVoice(v.value(), note, velocity);
      return v;
    }

    // Try and find free slot instead
    v = FindFreeVoice();
    if (v) {
      AssignVoice(v.value(), note, velocity);
      return v;
    }

    // All voices assigned. What next depends on provided strategy
    if constexpr (STEAL_STRATEGY::NONE == strategy) {
      return std::nullopt;
    } else if constexpr (STEAL_STRATEGY::LRU == strategy) {
      auto voice = lru_.front();
      lru_.erase(lru_.begin());
      AssignVoice(voice, note, velocity);
      return voice;
    } else {
      // Doh
    }
  }

  [[nodiscard]] std::optional<VoiceIndex> NoteOff(midi::Note note) noexcept
  {
    auto v = Find(note);
    if (v) {
      voice_pool_[v.value()] = {};
      lru_.erase(std::find(lru_.begin(), lru_.end(), v.value()));
    }
    return v;
  }

  std::optional<VoiceIndex> Find(midi::Note note) const noexcept
  {
    for (VoiceIndex i = 0; i < NUM_VOICES; ++i)
      if (voice_pool_[i].note == note) return i;
    return std::nullopt;
  }

private:
  std::array<Note, NUM_VOICES> voice_pool_;
  util::StaticStack<VoiceIndex, NUM_VOICES> lru_;
  VoiceIndex next_ = 0;

  static constexpr auto next(VoiceIndex i) { return i < NUM_VOICES - 1 ? ++i : 0; }

  std::optional<VoiceIndex> FindFreeVoice() const
  {
    auto slot = next_;
    do {
      if (voice_pool_[slot].is_free()) return slot;
      slot = next(slot);
    } while (slot != next_);

    return std::nullopt;
  }

  void AssignVoice(VoiceIndex voice_index, midi::Note note, midi::Velocity velocity)
  {
    voice_pool_[voice_index] = {note, velocity};
    next_ = next(voice_index);
    lru_.emplace_back(voice_index);
  }

public:
  auto &lru() const { return lru_; }
  auto &voices() const { return voice_pool_; }
};

template <size_t STACK_DEPTH>
using VoiceAllocatorMono = NoteStack<STACK_DEPTH>;

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_VOICE_ALLOCATOR_H_
