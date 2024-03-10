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

#include "drivers/core_timer.h"
#include "drivers/dac_4922.h"
#include "drivers/hires_timer.h"
#include "drivers/midi_serial.h"
#include "drivers/pfm2sid_gpio.h"
#include "menu/asid_player.h"
#include "menu/sid_player.h"
#include "menu/synth_editor.h"
#include "midi/midi_parser.h"
#include "pfm2sid_debug.h"
#include "platform/platform.h"
#include "platform/platform_config.h"
#include "sidbits/asid_parser.h"
#include "synth/engine.h"
#include "synth/parameters.h"
#include "synth/patch_bank.h"
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
stm32x::AveragedCycles ui_tick_cycles;
unsigned ui_event_counter = 0;
}  // namespace stats

static util::RingBuffer<uint8_t, kSerialMidiRxBufferSize> serial_midi_rx INCCM;
static midi::MidiParser serial_midi_parser INCCM;
static MODE current_mode = MODE::INVALID;
synth::SystemParameters system_parameters INCCM;
synth::PatchBank current_bank;
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
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  PFM2SID_DEBUG_INIT();

  GPIO::EnableClocks(true);
  core_timer.Init((F_CPU / synth::kDacUpdateRateHz / 4) - 1);
  dac.Init();
  midi_serial.Init();
  ui.Init();

  synth::PatchBank::default_bank(current_bank);
  current_bank.Load(0, current_patch);

  engine.Init(&current_patch.parameters);
  sid_synth_.Init(&current_patch.parameters, current_patch.wavetables);

  sid_synth_editor_.MenuInit();
  sid_synth_editor_.register_listener(&engine);
  sid_synth_editor_.register_listener(&sid_synth_);
  sid_synth_editor_.register_listener(&midi_handler);

  // STM32X_CORE_INIT(F_CPU / kSysTickUpdateHz); -> FreeRTOS timer
  serial_midi_parser.Init({&midi_handler, nullptr, nullptr});
  midi_handler.set_rx_channel(0);
}

static void RenderSampleBlock()
{
  while (sample_buffer.writeable()) {
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

// TODO Critical sections for reset and other "complex" editor operations
extern "C" void vRenderTask(void *pvParameters)
{
  (void)pvParameters;

  uint32_t ulStatus = 0;
  for (;;) {
    xTaskNotifyWaitIndexed(TASK_RENDER_IRQ_NOTIFY_IDX, 0, 0, &ulStatus, portMAX_DELAY);
    RenderSampleBlock();
  }
}

extern "C" void vMainTask(void *pvParameters)
{
  auto mode = static_cast<pfm2sid::MODE>((uint32_t)pvParameters);
  using namespace pfm2sid;

  set_mode(mode);

  RenderSampleBlock();
  core_timer.Start();

  auto ticks = HiresTimer::now();
  for (;;) {
    ui.DispatchEvents();

    if (sid_synth_.voice_active(0))
      display.SetIcon<ICON_POS::VOICE1>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);
    if (sid_synth_.voice_active(1))
      display.SetIcon<ICON_POS::VOICE2>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);
    if (sid_synth_.voice_active(2))
      display.SetIcon<ICON_POS::VOICE3>(ICON_VOICE_ACTIVE, kVoiceActivityTicks);

    // This isn't a super-precise method since we're polling
    // TODO Handling the player/asid update via UI is sketchy, but we have RTOS now...
    auto now = HiresTimer::now();
    if ((now - ticks) > HiresTimer::ms_to_timer(20)) {
      ui.Step();
      ui.UpdateDisplay();
      ticks = now;
    }
    {
      PFM2SID_DEBUG_TRACE(3);
      display.Tick();
    }

    xTaskNotifyWaitIndexed(TASK_MAIN_TICK_NOTIFY_IDX, 0, 0, nullptr, pdMS_TO_TICKS(1));
  }
}

// Use a timer for the UI/display tick. This could also hook the vApplicationTickHook
// (it's perhaps not quite lightweight enough)
extern "C" void vTickTimerCallback(TimerHandle_t /* xTimer */)
{
  // TODO we might get rid of STM32X_CORE_TICK post-FreeRTOS
  STM32X_CORE_TICK();
  stm32x::ScopedCycleMeasurement scm{stats::ui_tick_cycles};
  PFM2SID_DEBUG_TRACE(2);
  ui.Tick();

  // wake main task (TODO if events?)
  xTaskNotifyIndexed(xTaskHandles[TASK_MAIN], TASK_MAIN_TICK_NOTIFY_IDX, 0, eNoAction);
}

}  // namespace pfm2sid

// Trampoline interrupt from "unsafe but high priority" into "ISR safe API"
extern "C" void PFM2SID_RENDER_IRQ_HANDLER()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyIndexedFromISR(xTaskHandles[TASK_RENDER], TASK_RENDER_IRQ_NOTIFY_IDX, 0, eIncrement,
                            &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// DAC Update
// We have two DACs with two channels each. To keep the time in the interrupt short (I assume...)
// the original code updates one channel per call -- i.e. the timer runs at sample rate x 4 -- which
// avoids having to waste time waiting for the SPI transfer.
//
// I didn't see a way to build a timer + DMA chain to somehow automate this, and the SPI NSS lines
// won't help either since we need two chip selects; similarly the I2S interface won't support it
// either (citation required).
//
// Ideally the timing is fairly consistent (since the dac.Load drives the sample and thus jitter.
extern "C" void PFM2SID_CORE_TIMER_IRQ_HANDLER()
{
  using namespace pfm2sid;
  if (!core_timer.Ticked()) return;

  PFM2SID_DEBUG_TRACE(1);
  if (Dac::FRAME_COMPLETE == dac.Update()) {
    // Do we need the 40ns setup time after CS here?
    const auto next_sample = sample_buffer.ReadableBlock<1>()[0];
    dac.Load();
    if (sample_buffer.Consume<1>()) {
      // Trigger the next block -- which we only want to happen when required, otherwise the
      // superfluous interrupts will bog things down...
      NVIC->STIR = PFM2SID_RENDER_IRQn;
    }
    dac.BeginFrame(next_sample.left, next_sample.right);
  }

  // Poll MIDI serial input here, it should way faster than we can receive bytes anyway.
  // If we add a TX option perhaps an interrupt might be more useful.
  // Also this may be a better place to handle clock messages since we have "hires" time (or,
  // timestamp the incoming bytes).
  auto midi_rx = midi_serial.Receive();
  if (midi_rx) { serial_midi_rx.Write(midi_rx.value()); }
}

int main()
{
  STM32X_DEBUG_INIT();
  pfm2sid::Init();

  TimerCreate(TIMER_UI_TICK, pdMS_TO_TICKS(1), pfm2sid::vTickTimerCallback);
  TaskCreate(TASK_MAIN, pfm2sid::vMainTask, (void *)pfm2sid::MODE::SID_SYNTH);
  TaskCreate(TASK_RENDER, pfm2sid::vRenderTask, nullptr);

  xTimerStart(xTimerHandles[TIMER_UI_TICK], pdMS_TO_TICKS(1));

  vTaskStartScheduler();
  for (;;) {}
}
