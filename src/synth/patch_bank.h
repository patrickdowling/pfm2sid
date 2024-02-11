// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2024 Patrick Dowling (pld@gurkenkiste.com)
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
#include "synth/patch.h"

namespace pfm2sid::synth {

class PatchBank {
public:
  PatchBank() : name_{"Default"} {}

  const char *name() const { return name_; }

  const char *patch_name(size_t i) const { return patch_data_[i].name; }

  bool Load(size_t i, Patch &patch) const
  {
    util::StreamBufferReader sbr{patch_data(i), Patch::kBinaryPatchSize};
    return patch.Load(sbr);
  }

  bool Save(size_t i, const Patch &patch)
  {
    util::StreamBufferWriter sbw{patch_data(i), Patch::kBinaryPatchSize};
    memcpy(patch_data_[i].name, patch.name(), kPatchNameLength);
    return patch.Save(sbw);
  }

  constexpr int size() { return patch_data_.size(); }

  static void default_bank(PatchBank &patch_bank);

private:
  char name_[kBankNameLength] = {0};

  struct PatchData {
    char name[kPatchNameLength] = {};
    uint8_t binary_data[Patch::kBinaryPatchSize];
  };

  std::array<PatchData, kNumPatchesPerBank> patch_data_;

  auto patch_data(size_t i) -> uint8_t * { return patch_data_[i].binary_data; }
  auto patch_data(size_t i) const -> const uint8_t * { return patch_data_[i].binary_data; }
};

}  // namespace pfm2sid::synth
