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
#include "patch.h"

namespace pfm2sid::synth {

static_assert(Patch::kStorageSize < Patch::kBinaryPatchSize);

bool Patch::Save(util::StreamBufferWriter &sbw) const
{
  sbw.Write(kStorageID);
  sbw.Write(kVersionID);
  sbw.Write(name(), kPatchNameLength);

  parameters.Save(sbw);
  for (const auto &wt : wavetables) wt.Save(sbw);

  sbw.Write(kEndMarker);
  return !sbw.overflow();
}

bool Patch::Load(util::StreamBufferReader &sbr)
{
  if (kStorageID != sbr.Read<util::FOURCC>()) return false;
  if (kVersionID != sbr.Read<util::FOURCC>()) return false;
  sbr.Read(name_, kPatchNameLength);

  parameters.Load(sbr);
  for (auto &wt : wavetables) wt.Load(sbr);

  if (kEndMarker != sbr.Read<util::FOURCC>()) return false;

  return !sbr.underflow();
}

}  // namespace pfm2sid::synth
