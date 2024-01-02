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
#ifndef PFM2SID_MIDI_PARSER_H_
#define PFM2SID_MIDI_PARSER_H_

#include <array>

#include "midi_types.h"

namespace pfm2sid::midi {

// Yes, there exist lots of MIDI parsers. I've long avoided the low-level bits though so here we
// are. None of this is supposed to be production-ready anyway, right?
//
// Q Do we filter the channel messages in the parser, or in the handler? I've heard it both ways.
// Q Related: filter when message complete, or on initial status byte? The latter discards things
// earlier...
//
// TODO Return values for MidiSysex handler
// TODO Filter out classes of messages?

enum STATUS : uint8_t {
  CH_NOTE_OFF = 0x80,
  CH_NOTE_ON = 0x90,
  CH_POLY_AT = 0xA0,
  CH_CTRL_CHG = 0xB0,
  CH_PROG_CHG = 0xC0,
  CH_CHAN_AT = 0xD0,
  CH_PITCHBEND = 0xE0,
  SYS_SYSEX = 0xF0,
  SYS_TIME_CODE = 0xF1,
  SYS_SONG_POS = 0xF2,
  SYS_SONG_SEL = 0xF3,
  UNDEFINED_F4 = 0xF4,
  UNDEFINED_F5 = 0xF5,
  SYS_TUNE_REQ = 0xF6,
  SYS_EOX = 0xF7,
  RT_CLOCK = 0xF8,
  UNDEFINED_F9 = 0xF9,
  RT_START = 0xFA,
  RT_CONTINUE = 0xFB,
  RT_STOP = 0xFC,
  UNDEFINED_FD = 0xFD,
  RT_ACT_SENS = 0xFE,
  RT_SYS_RESET = 0xFF,
};

enum struct SYSEX_STATUS { IDLE, START, DATA, EOX, ABORT };
const char *to_string(SYSEX_STATUS status);

// TODO Make distinct message types instead? Seems like a lot of work...
// TODO But also a separate message-based parser might be useful

class MidiHandlerRT {
public:
  virtual ~MidiHandlerRT() = default;
  virtual void MidiClock() {}
  virtual void MidiStart() {}
  virtual void MidiContinue() {}
  virtual void MidiStop() {}
  virtual void MidiActiveSensing() {}
  virtual void MidiReset() {}
};

class MidiHandlerSys {
public:
  virtual ~MidiHandlerSys() = default;
  virtual void MidiTimeCode(uint8_t) {}
  virtual void MidiSongPos(uint8_t, uint8_t) {}
  virtual void MidiSongSel(uint8_t) {}
  virtual void MidiTuneRequest() {}
};

class MidiHandler {
public:
  virtual ~MidiHandler() = default;
  MidiHandler(ChannelMask enabled_channels = ALL_CHANNELS) : enabled_channels_(enabled_channels) {}

  virtual void MidiNoteOff(Channel /*channel*/, Note /*note*/, Velocity /*velocity*/) {}
  virtual void MidiNoteOn(Channel /*channel*/, Note /*note*/, Velocity /*velocity*/) {}
  virtual void MidiAftertouch(Channel /*channel*/, Note /*note*/, uint8_t /*pressure*/) {}
  virtual void MidiControlChange(Channel /*channel*/, uint8_t /*control*/, uint8_t /*value*/) {}
  virtual void MidiProgramChange(Channel /*channel*/, uint8_t /*program*/) {}
  virtual void MidiAftertouch(Channel /*channel*/, uint8_t /*pressure*/) {}
  virtual void MidiPitchbend(Channel /*channel*/, int16_t /*value*/) {}

  // The sysex handler should receive
  // SYSEX_STATUS::START when the first 3 bytes are received.
  // SYSEX_STATUS::DATA on-going data when the buffer is full.
  // SYSEX_STATUS::EOX when transfer complete
  // SYSEX_STATUS::ABORT when transfer aborted.
  //
  // Returning false for START or DATA will stop the transfer and further data will be ignored.
  // TODO this is wasteful for small messages (since they get callbacked twice) so perhaps this
  // pre-filter can be optional.
  [[nodiscard]] virtual bool MidiSysex(const uint8_t * /*data*/, unsigned /*len*/,
                                       SYSEX_STATUS /*status*/)
  {
    return false;
  }

  bool channel_enabled(unsigned channel) const { return enabled_channels_[channel]; }

  auto channel_mask() const { return enabled_channels_; }

  void set_channel_mask(uint16_t mask) { enabled_channels_ = mask; }

protected:
  MidiHandler() = delete;

  ChannelMask enabled_channels_ = ALL_CHANNELS;
};

#ifndef MIDI_PARSER_RX_BUFFER_SIZE
#define MIDI_PARSER_RX_BUFFER_SIZE 128
#endif

class MidiParser {
public:
  struct Handlers {
    MidiHandler *midi_handler = nullptr;
    MidiHandlerSys *sys_handler = nullptr;
    MidiHandlerRT *rt_handler = nullptr;
  };

  void Init(const Handlers &handlers);

  void Reset();

  void Parse(uint8_t byte);

  void Parse(const uint8_t *data, size_t len)
  {
    while (len--) Parse(*data++);
  }

  auto current_state() const { return state_; }

private:
  static constexpr unsigned kRxBufferSize = MIDI_PARSER_RX_BUFFER_SIZE;
  static constexpr unsigned kSysexMinLen = 3;

  struct {
    uint8_t running_status = 0;
    unsigned msg_len = 0;
    unsigned expected_msg_len = 0;
    SYSEX_STATUS sysex_status = SYSEX_STATUS::IDLE;
  } state_;

  std::array<uint8_t, kRxBufferSize> data_ = {};

  MidiHandler *midi_handler_ = nullptr;
  MidiHandlerSys *sys_handler_ = nullptr;
  MidiHandlerRT *rt_handler_ = nullptr;

  void DispatchMessage(uint8_t running_status) const;

  constexpr static unsigned expected_msg_len(uint8_t status_byte);
};

}  // namespace pfm2sid::midi

#endif  // PFM2SID_MIDI_PARSER_H_
