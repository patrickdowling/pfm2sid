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
#include "pfm2sid.h"

#include <cmath>
#include <cstdio>

#include "drivers/core_timer.h"
#include "drivers/dac_4922.h"
#include "drivers/midi_serial.h"
#include "drivers/pfm2sid_gpio.h"
#include "menu/asid_player.h"
#include "menu/sid_player.h"
#include "menu/synth_editor.h"
#include "midi/midi_parser.h"
#include "pfm2sid_debug.h"
#include "sidbits/asid_parser.h"
#include "synth/engine.h"
#include "synth/parameters.h"
#include "synth/patch.h"
#include "synth/sid_synth.h"
#include "ui/display.h"
#include "ui/ui.h"

extern "C" {
extern const uint8_t _binary_test_dmp_start;
extern const uint8_t _binary_test_dmp_end;
}

STM32X_CORE_DEFINE(INCCMZ)
namespace pfm2sid {
GPIO gpio;
CoreTimer core_timer INCCM;
Dac dac INCCM;
MidiSerial midi_serial INCCM;
Ui ui INCCM;

namespace stats {
stm32x::AveragedCycles render_block_cycles;
stm32x::AveragedCycles sid_clock_cycles;
unsigned ui_event_counter = 0;
}  // namespace stats

static util::RingBuffer<uint8_t, kSerialMidiRxBufferSize> serial_midi_rx INCCM;
static midi::MidiParser serial_midi_parser INCCM;

static MODE current_mode = MODE::INVALID;
synth::SystemParameters system_parameters INCCM;

synth::Patch current_patch INCCM;

synth::Engine engine INCCM;
synth::SIDSynth sid_synth_ INCCM;
// TODO It's perhaps wasteful to allocate All The Menus even if only one is being used?
// Might depend on how switching works...
static SIDPlayer sid_player_;
static ASIDPlayer asid_player_;
static synth::SIDSynthEditor sid_synth_editor_;

void set_mode(MODE mode)
{
  if (mode != current_mode) {
    switch (mode) {
      case MODE::SID_SYNTH: ui.SetMenu(&sid_synth_editor_); break;
      case MODE::SID_PLAYER:
        // sid_player_.Init({&_binary_test_dmp_start, &_binary_test_dmp_end});
        ui.SetMenu(&sid_player_);
        break;
      case MODE::ASID_PLAYER:
        ui.SetMenu(&asid_player_);
        current_mode = MODE::ASID_PLAYER;
        break;
      default: break;
    }
    current_mode = mode;
    display.Clear();
  }
}

class MidiHandler : public midi::MidiHandler, public synth::ParameterListener {
public:
  MidiHandler() : midi::MidiHandler{1} {}

  void MidiNoteOff(midi::Channel channel, midi::Note note, midi::Velocity velocity) final
  {
    display.SetIcon<ICON_POS::MIDI>(ICON_MIDI_NOTE, kMidiActivityTicks);
    if (MODE::SID_SYNTH == current_mode) { sid_synth_.NoteOff(channel, note, velocity); }
  }

  void MidiNoteOn(midi::Channel channel, midi::Note note, midi::Velocity velocity) final
  {
    display.SetIcon<ICON_POS::MIDI>(ICON_MIDI_NOTE, kMidiActivityTicks);
    if (MODE::SID_SYNTH == current_mode) { sid_synth_.NoteOn(channel, note, velocity); }
  }

  void MidiPitchbend(midi::Channel channel, int16_t value) final
  {
    display.SetIcon<ICON_POS::MIDI>(ICON_MIDI_BEND, kMidiActivityTicks);
    sid_synth_.Pitchbend(channel, value);
  }

  bool MidiSysex(const uint8_t *data, unsigned len, midi::SYSEX_STATUS sysex_status) final
  {
    display.SetIcon<ICON_POS::MIDI>(ICON_MIDI_SYSEX, kMidiActivityTicks);
    if (!len) return false;
    bool accept = false;

    switch (sysex_status) {
      case midi::SYSEX_STATUS::START:
        if (sidbits::is_asid_sysex(data)) {
          if (MODE::ASID_PLAYER != current_mode) { set_mode(MODE::ASID_PLAYER); }
          accept = true;
        }
        break;
      case midi::SYSEX_STATUS::EOX:
        if (MODE::ASID_PLAYER == current_mode) { accept = !!asid_player_.ParseSysex(data, len); }
      default: break;
    }
    return accept;
  }

  void SystemParameterChanged(synth::SYSTEM parameter) final
  {
    using namespace synth;
    if (SYSTEM::MIDI_CHANNEL == parameter) {
      auto value = system_parameters.get<SYSTEM::MIDI_CHANNEL>().value();
      if (value > 0 && value <= 16) { set_rx_channel(value - 1); }
    }
  }

  void set_rx_channel(midi::Channel channel)
  {
    enabled_channels_.reset();
    enabled_channels_[channel] = true;
    if (MODE::SID_SYNTH == current_mode) { sid_synth_.set_midi_channel(channel); }
  }
};

static MidiHandler midi_handler;

static synth::SampleBuffer sample_buffer INCCM;

static void Init()
{
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, FLASH_ORIGIN - NVIC_VectTab_FLASH);  // expects an offset
  PFM2SID_DEBUG_INIT();

  GPIO::EnableClocks(true);
  core_timer.Init((F_CPU / synth::kDacUpdateRateHz / 4) - 1);
  dac.Init();
  midi_serial.Init();
  ui.Init();

  engine.Init(&current_patch.parameters);
  sid_synth_.Init(&current_patch.parameters);

  sid_synth_editor_.MenuInit();
  sid_synth_editor_.register_listener(&engine);
  sid_synth_editor_.register_listener(&sid_synth_);
  sid_synth_editor_.register_listener(&midi_handler);

  core_timer.Start();
  STM32X_CORE_INIT(F_CPU / kSysTickUpdateHz);

  serial_midi_parser.Init({&midi_handler, nullptr, nullptr});
  midi_handler.set_rx_channel(0);

  synth::InitWaveTables();  // TODO
}

static void RenderSampleBlock()
{
  while (sample_buffer.writeable() >= sample_buffer.block_size()) {
    while (serial_midi_rx.readable()) { serial_midi_parser.Parse(serial_midi_rx.Read()); }

    stm32x::ScopedCycleMeasurement scm{stats::render_block_cycles};
    switch (current_mode) {
      case MODE::SID_SYNTH:
        sid_synth_.Update();
        engine.RenderBlock(sample_buffer.WriteableBlock(), sid_synth_.register_map());
        break;
      case MODE::SID_PLAYER:
        engine.RenderBlock(sample_buffer.WriteableBlock(), sid_player_.register_map());
        break;
      case MODE::ASID_PLAYER:
        engine.RenderBlock(sample_buffer.WriteableBlock(), asid_player_.register_map());
        break;
      default: break;
    }
    sample_buffer.Commit<sample_buffer.block_size()>();
  }
}

[[noreturn]] void Run(MODE mode)
{
  using namespace pfm2sid;

  set_mode(mode);

  auto ticks = core_timer.now();
  for (;;) {
    RenderSampleBlock();
    ui.DispatchEvents();

    if (sid_synth_.voice_active(0))
      display.SetIcon<ICON_POS::VOICE1>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);
    if (sid_synth_.voice_active(1))
      display.SetIcon<ICON_POS::VOICE2>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);
    if (sid_synth_.voice_active(2))
      display.SetIcon<ICON_POS::VOICE3>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);

    // This isn't a super-precise method since we're polling
    // TODO Handling the player/asid update via UI is sketchy
    auto now = core_timer.now();
    if (now - ticks > CoreTimer::ms_to_timer(20)) {
      ui.Step();
      ui.UpdateDisplay();
      ticks = now;
    }
  }
}

}  // namespace pfm2sid

// DAC Update
// We have two DACs with two channels each. To keep the time in the interrupt short (I assume...)
// the original code updates one channel per call -- i.e. the timer runs at sample rate x 4 -- which
// avoids having to waste time waiting for the SPI transfer.
//
// I didn't see a way to build a timer + DMA chain to somehow automate this, and the SPI NSS lines
// won't help either since we need two chip selects; similarly the I2S interface won't support it
// either (citation required).
extern "C" void PFM2SID_CORE_TIMER_HANDLER()
{
  using namespace pfm2sid;
  if (!core_timer.Ticked()) return;

  PFM2SID_DEBUG_TRACE(1);
  if (Dac::FRAME_COMPLETE == dac.Update()) {
    // Do we need the 40ns setup time after CS here?
    const auto next_sample = sample_buffer.ReadableBlock<1>()[0];
    dac.Load();
    sample_buffer.Consume<1>();
    dac.BeginFrame(next_sample.left, next_sample.right);
  }

  // Poll MIDI serial input here, it should way faster than we can receive bytes anyway.
  // If we add a TX option perhaps an interrupt might be more useful.
  // Also this may be a better place to handle clock messages since we have "hires" time (or,
  // timestamp the incoming bytes).
  auto midi_rx = midi_serial.Receive();
  if (midi_rx) { serial_midi_rx.Write(midi_rx.value()); }
}

extern "C" void SysTick_Handler()
{
  STM32X_CORE_TICK();
  using namespace pfm2sid;
  PFM2SID_DEBUG_TRACE(2);
  ui.Tick();
  PFM2SID_DEBUG_TRACE(3);
  display.Tick();
}

int main()
{
  STM32X_DEBUG_INIT();
  pfm2sid::Init();
  pfm2sid::Run(pfm2sid::MODE::SID_SYNTH);
}
