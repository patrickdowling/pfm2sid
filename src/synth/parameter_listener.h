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
#ifndef PFM2SID_SYNTH_PARAMETER_LISTENER_H_
#define PFM2SID_SYNTH_PARAMETER_LISTENER_H_

#include "synth/parameters.h"

namespace pfm2sid::synth {

// In some places we need a notification when a parameter has changed.
// This solution isn't super scaleable.
class ParameterListener {
public:
  virtual ~ParameterListener() = default;

  virtual void SystemParameterChanged(SYSTEM /*parameter*/) {}
  virtual void GlobalParameterChanged(GLOBAL /*parameter*/) {}
};

};  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PARAMETER_LISTENER_H_
