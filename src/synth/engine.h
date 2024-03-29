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
#ifndef PFM2SID_ENGINE_H_
#define PFM2SID_ENGINE_H_

#include "synth/parameter_listener.h"
#include "synth/parameter_structs.h"
#include "synth/sid_instance.h"
#include "synth/synth.h"
#include "util/util_macros.h"

namespace pfm2sid::synth {

// Obligatory abstraction of sound generation. Engine? Processor? Generator?
//
// For now it's just a wrapper around a SID instance, and the actual "fun" happens in a wrapper
// (ASID player, synth, etc.) that can set a suitable register map...
//
class Engine : public ParameterListener {
public:
  Engine() = default;
  DELETE_COPY_MOVE(Engine);

  void Init(Parameters *parameters);
  void Reset();

  void RenderBlock(SampleBuffer::MutableBlock block, const sidbits::RegisterMap &register_map);

  const sidbits::RegisterMap &register_map() const { return sid_instance_.register_map(); }

  // parameter hooks
  void SystemParameterChanged(SYSTEM parameter) final;
  void GlobalParameterChanged(GLOBAL parameter) final;

protected:
  Parameters *parameters_ = nullptr;

  SIDInstance sid_instance_;
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_ENGINE_H_
