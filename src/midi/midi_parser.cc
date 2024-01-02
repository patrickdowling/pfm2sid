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
#include "midi_parser.h"

#include <tuple>

#ifdef MIDI_TRACE_FMT
#include <fmt/core.h>
#else
#define MIDI_TRACE_FMT(...) \
  do {                      \
  } while (0)
#endif

namespace pfm2sid::midi {

const char *to_string(SYSEX_STATUS sysex_status)
{
  switch (sysex_status) {
    case SYSEX_STATUS::IDLE: return "SYSEX_IDLE";
    case SYSEX_STATUS::START: return "SYSEX_START";
    case SYSEX_STATUS::DATA: return "SYSEX_DATA";
    case SYSEX_STATUS::EOX: return "SYSEX_EOX";
    case SYSEX_STATUS::ABORT: return "SYSEX_ABORT";
  }
  return "?";
}

// NOTES
// For sysex, we're taking a page out of miby implementation and requiring a length of 3 before the
// first callback. In theory we could then use a return code to abort the transfer if it's not for
// this device.

void MidiParser::Init(const Handlers &handlers)
{
  Reset();

  midi_handler_ = handlers.midi_handler;
  sys_handler_ = handlers.sys_handler;
  rt_handler_ = handlers.rt_handler;
}

void MidiParser::Reset()
{
  state_.running_status = 0;
  state_.msg_len = 0;
  state_.expected_msg_len = 0;
  state_.sysex_status = SYSEX_STATUS::IDLE;
}

#define MIDI_HANDLER()  \
  if (!midi_handler_) { \
  } else                \
    midi_handler_

/*static*/ constexpr unsigned MidiParser::expected_msg_len(uint8_t status_byte)
{
  switch (status_byte) {
    case CH_NOTE_OFF: return 2;
    case CH_NOTE_ON: return 2;
    case CH_POLY_AT: return 2;
    case CH_CTRL_CHG: return 2;
    case CH_PROG_CHG: return 1;
    case CH_CHAN_AT: return 1;
    case CH_PITCHBEND: return 2;
    case SYS_SYSEX: return kSysexMinLen;
    case SYS_TIME_CODE: return 1;
    case SYS_SONG_POS: return 2;
    case SYS_SONG_SEL: return 1;
    case SYS_TUNE_REQ: return 0;
    case SYS_EOX: return 0;
    default: return 0;
  }
}

constexpr auto split_status_byte(uint8_t byte)
{
  uint8_t hi = byte & 0xf0;
  uint8_t lo = byte & 0x0f;
  return std::tuple(hi, lo);
}

constexpr bool is_channel_message(uint8_t byte)
{
  return byte < STATUS::SYS_SYSEX;
}

constexpr bool is_data_byte(uint8_t byte)
{
  return byte < STATUS::CH_NOTE_OFF;
}

constexpr bool is_sys_message(uint8_t byte)
{
  return byte >= STATUS::SYS_SYSEX;
}

constexpr bool is_realtime_message(uint8_t byte)
{
  return byte >= STATUS::RT_CLOCK;
}

void MidiParser::Parse(uint8_t byte)
{
  // Short-circuit evaluation of realtime messages
  if (is_realtime_message(byte)) {
    if (rt_handler_) {
      switch (byte) {
        case STATUS::RT_CLOCK: rt_handler_->MidiClock(); break;
        case STATUS::RT_START: rt_handler_->MidiStart(); break;
        case STATUS::RT_CONTINUE: rt_handler_->MidiContinue(); break;
        case STATUS::RT_STOP: rt_handler_->MidiStop(); break;
        case STATUS::RT_ACT_SENS: rt_handler_->MidiActiveSensing(); break;
        case STATUS::RT_SYS_RESET: rt_handler_->MidiReset(); break;
      }
    }
    return;
  }

  if (is_data_byte(byte)) {
    if (!state_.running_status) return;  // Orphan data or noise
    data_[state_.msg_len++] = byte;
    if (state_.msg_len < state_.expected_msg_len) return;

    if (SYSEX_STATUS::IDLE != state_.sysex_status) {
      // This handler can use the 3 bytes to decide whether to continue at all by returning false.
      // TODO Make this optional
      if (midi_handler_ &&
          midi_handler_->MidiSysex(data_.data(), state_.msg_len, state_.sysex_status)) {
        if (state_.sysex_status == SYSEX_STATUS::DATA) {
          state_.msg_len = 0;
        } else {
          state_.sysex_status = SYSEX_STATUS::DATA;
        }
        state_.expected_msg_len = kRxBufferSize;
      } else {
        // End transfer, further data will be ignored.
        state_.sysex_status = SYSEX_STATUS::IDLE;
        state_.running_status = 0;
      }
    } else {
      DispatchMessage(state_.running_status);
      if (!is_channel_message(state_.running_status)) state_.running_status = 0;
      state_.msg_len = 0;
    }
  } else {
    state_.running_status = 0;

    // End or abort current sysex transfer, if any
    if (SYSEX_STATUS::IDLE != state_.sysex_status) {
      bool eox = byte == SYS_EOX;
      if (midi_handler_)
        (void)midi_handler_->MidiSysex(data_.data(), state_.msg_len,
                                       eox ? SYSEX_STATUS::EOX : SYSEX_STATUS::ABORT);
      state_.sysex_status = SYSEX_STATUS::IDLE;
      if (eox) return;
    }

    // Handler can filter out unwanted channel messages
    if (is_channel_message(byte)) {
      auto [hi, lo] = split_status_byte(byte);
      MIDI_TRACE_FMT("channel={} enabled={:16}", lo, midi_handler_->channel_mask().to_string());
      if (!midi_handler_ || !midi_handler_->channel_enabled(lo)) return;

      state_.expected_msg_len = expected_msg_len(hi);
    } else {
      state_.expected_msg_len = expected_msg_len(byte);
    }

    // New status, get expected data length.
    // Commands with no data get dispatched immediately and don't set the running status.
    if (state_.expected_msg_len == 0) {
      DispatchMessage(byte);
    } else {
      if (SYS_SYSEX == byte) state_.sysex_status = SYSEX_STATUS::START;
      state_.running_status = byte;
    }
    state_.msg_len = 0;
  }
}

void MidiParser::DispatchMessage(uint8_t running_status) const
{
  MIDI_TRACE_FMT("Dispatch {:02x}", running_status);
  if (is_sys_message(running_status)) {
    if (sys_handler_) {
      switch (running_status) {
        case STATUS::SYS_TIME_CODE: sys_handler_->MidiTimeCode(data_[0]); break;
        case STATUS::SYS_SONG_POS: sys_handler_->MidiSongPos(data_[0], data_[1]); break;
        case STATUS::SYS_SONG_SEL: sys_handler_->MidiSongSel(data_[0]); break;
        case STATUS::SYS_TUNE_REQ: sys_handler_->MidiTuneRequest(); break;
      }
    }
    return;
  }

  if (midi_handler_) {
    auto [hi, lo] = split_status_byte(running_status);
    switch (hi) {
      case STATUS::CH_NOTE_OFF: midi_handler_->MidiNoteOff(lo, data_[0], data_[1]); break;
      case STATUS::CH_NOTE_ON:
        if (data_[1]) {
          midi_handler_->MidiNoteOn(lo, data_[0], data_[1]);
        } else {
          midi_handler_->MidiNoteOff(lo, data_[0], 0);
        }
        break;
      case STATUS::CH_POLY_AT: midi_handler_->MidiAftertouch(lo, data_[0], data_[1]); break;
      case STATUS::CH_CTRL_CHG: midi_handler_->MidiControlChange(lo, data_[0], data_[1]); break;
      case STATUS::CH_PROG_CHG: midi_handler_->MidiProgramChange(lo, data_[0]); break;
      case STATUS::CH_CHAN_AT: midi_handler_->MidiAftertouch(lo, data_[0]); break;
      case STATUS::CH_PITCHBEND:
        midi_handler_->MidiPitchbend(lo, static_cast<int16_t>(-8192 + (data_[1] << 7) + data_[0]));
        break;
    }
  }
}

}  // namespace pfm2sid::midi
