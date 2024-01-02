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
#include "parameter_structs.h"

#include <cstdio>

namespace pfm2sid::synth {

void ParameterValue::Fmt(char* buf) const
{
  if (desc_->parameter.type == synth::PARAMETER_SCOPE::NONE)
    memset(buf, ' ', 5);
  else if (desc_->label_strings) {
    snprintf(buf, 6, "%5s", desc_->label_strings[ivalue]);
  } else {
    if (desc_->formatter == FORMATTER::ZEROISOFF && ivalue == 0)
      snprintf(buf, 6, "%5s", "off");
    else
      snprintf(buf, 6, "%5ld", (long)ivalue);
  }
}
}  // namespace pfm2sid::synth
