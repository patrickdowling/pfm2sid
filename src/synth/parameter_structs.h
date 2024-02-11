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
#ifndef PFM2SID_SYNTH_PARAMETER_STRUCTS_H_
#define PFM2SID_SYNTH_PARAMETER_STRUCTS_H_

#include <algorithm>
#include <array>

#include "parameter_detail.h"
#include "parameters.h"

namespace pfm2sid::synth {

class ParameterRef {
public:
  const PARAMETER_SCOPE type;
  union {
    const SYSTEM system_param;
    const GLOBAL global_param;
    const VOICE voice_param;
    const LFO lfo_param;
  };
  constexpr ParameterRef() : type{PARAMETER_SCOPE::NONE}, global_param{GLOBAL::LAST} {}

  template <typename T>
  constexpr ParameterRef(T t);

  constexpr bool is_system() const { return PARAMETER_SCOPE::SYSTEM == type; }
  constexpr bool is_global() const { return PARAMETER_SCOPE::GLOBAL == type; }
  constexpr bool is_voice() const { return PARAMETER_SCOPE::VOICE == type; }
  constexpr bool is_lfo() const { return PARAMETER_SCOPE::LFO == type; }
};

template <>
constexpr ParameterRef::ParameterRef(SYSTEM p) : type{PARAMETER_SCOPE::SYSTEM}, system_param{p}
{}
template <>
constexpr ParameterRef::ParameterRef(GLOBAL p) : type{PARAMETER_SCOPE::GLOBAL}, global_param{p}
{}
template <>
constexpr ParameterRef::ParameterRef(VOICE p) : type{PARAMETER_SCOPE::VOICE}, voice_param{p}
{}
template <>
constexpr ParameterRef::ParameterRef(LFO p) : type{PARAMETER_SCOPE::LFO}, lfo_param{p}
{}

// We're just editing everything as an "int" (or eventually as a float) and always keep the stored
// value within the correct range. This simplifies some things, but does require a mapping later to
// "native" values (i.e. in this case, SID registers). There also needs to be a MIDI CC -> parameter
// mapping of some kind.
//
// I seem to re-use this pattern a lot, perhaps it's time to find a better model...
enum struct FORMATTER { DEFAULT, ZEROISOFF };
struct ParameterDesc {
  const char name[5] = {0};
  const parameter_value_type min_value = 0;
  const parameter_value_type max_value = 0;
  const ParameterRef parameter;
  const parameter_value_type default_value = 0;
  const char* const* label_strings = nullptr;
  const FORMATTER formatter = FORMATTER::DEFAULT;

  constexpr auto clamp(parameter_value_type value) const
  {
    return std::clamp(value, min_value, max_value);
  }

  ParameterDesc() = delete;
  DELETE_COPY_MOVE(ParameterDesc);

  static const ParameterDesc* Find(const ParameterRef parameter);
};

// Generic parameter value storage
// Contains a pointer to the ParameterDesc, which in turn contains a ParameterRef.
// TODO include voice/lfo index?

class ParameterValue {
public:
  /*constexpr*/ ParameterValue(const ParameterRef parameter_ref)
      : desc_(ParameterDesc::Find(parameter_ref)), ivalue(desc_->default_value)
  {}
  ParameterValue() = delete;
  // DELETE_COPY_MOVE(ParameterValue);
  //  -> This might be overkill; we want to avoid copies from ::get() type functions, but it does
  //  inhibit keeping parameters in a Patch (which might be copyable).

  constexpr const char* name() const { return desc_->name; }
  constexpr auto ref() const { return desc_->parameter; }
  constexpr auto desc() const { return desc_; }
  constexpr auto value() const { return ivalue; }

  void Reset() { ivalue = desc_->default_value; }

  ParameterValue& operator=(parameter_value_type value)
  {
    ivalue = desc_->clamp(value);
    return *this;
  }

  // For editor use
  auto change_value(int delta)
  {
    auto value = desc_->clamp(ivalue + delta);
    if (ivalue != value) {
      ivalue = value;
      return true;
    } else {
      return false;
    }
  }

  // Helpers for modulated values
  auto modulate_value(int delta) const { return desc_->clamp(ivalue + delta); }

  template <typename T>
  auto modulate_value(int delta) const
  {
    return detail::typed_value<T>(modulate_value(delta));
  }

  void Fmt(char* buf) const;

private:
  const ParameterDesc* /*const*/ desc_;
  parameter_value_type ivalue;
};

class Parameters {
public:
  void Reset();

  template <GLOBAL parameter>
  auto& get() const
  {
    static_assert(parameter < GLOBAL::LAST);
    return global_parameters_[util::enum_to_i(parameter)];
  }

  template <LFO_INDEX lfo_index, LFO parameter>
  auto& get() const
  {
    static_assert(parameter < LFO::LAST);
    return lfo_parameters_[lfo_index][util::enum_to_i(parameter)];
  }

  template <auto voice_index, VOICE parameter>
  auto& get() const
  {
    static_assert(parameter < VOICE::LAST);
    return voice_parameters_[voice_index][util::enum_to_i(parameter)];
  }

  template <auto parameter, typename T>
  auto get() const
  {
    return detail::typed_value<T>(get<parameter>().value());
  }

  template <auto index, auto parameter, typename T>
  auto get() const
  {
    return detail::typed_value<T>(get<index, parameter>().value());
  }

  template <VOICE parameter>
  auto& get(sidbits::VOICE_INDEX voice_index) const
  {
    return voice_parameters_[voice_index][util::enum_to_i(parameter)];
  }

  template <VOICE parameter, typename T>
  auto get(sidbits::VOICE_INDEX voice_index) const
  {
    return detail::typed_value<T>(get<parameter>(voice_index).value());
  }

  // These are for editor use
  ParameterValue* mutable_value(const ParameterRef& parameter_ref)
  {
    if (parameter_ref.is_global())
      return &global_parameters_[util::enum_to_i(parameter_ref.global_param)];
    return nullptr;
  }

  ParameterValue* mutable_value(const ParameterRef& parameter_ref, sidbits::VOICE_INDEX voice_index)
  {
    return parameter_ref.is_voice()
               ? &voice_parameters_[voice_index][util::enum_to_i(parameter_ref.voice_param)]
               : nullptr;
  }

  ParameterValue* mutable_value(const ParameterRef& parameter_ref, LFO_INDEX lfo_index)
  {
    return parameter_ref.is_lfo()
               ? &lfo_parameters_[lfo_index][util::enum_to_i(parameter_ref.lfo_param)]
               : nullptr;
  }

  void Save(util::StreamBufferWriter &sbw) const;
  bool Load(util::StreamBufferReader &sbr);

private:
  using global_parameter_array = std::array<ParameterValue, kNumGlobalParameters>;
  using lfo_parameter_array = std::array<ParameterValue, kNumLfoParameters>;
  using voice_parameter_array = std::array<ParameterValue, kNumVoiceParameters>;

  global_parameter_array global_parameters_ = detail::build_parameter_array<GLOBAL>();

  std::array<lfo_parameter_array, kNumLfos> lfo_parameters_ =
      detail::build_parameter_arrays<LFO, kNumLfos>();

  std::array<voice_parameter_array, 3> voice_parameters_ =
      detail::build_parameter_arrays<VOICE, 3>();
};

class SystemParameters {
public:
  using system_parameter_array = std::array<ParameterValue, kNumSystemParameters>;

  void Reset();

  template <SYSTEM parameter>
  auto& get() const
  {
    static_assert(parameter < SYSTEM::LAST);
    return system_parameters_[util::enum_to_i(parameter)];
  }

  template <SYSTEM parameter, typename T>
  auto get() const
  {
    return detail::typed_value<T>(get<parameter>().value());
  }

  ParameterValue* mutable_value(const ParameterRef& parameter_ref)
  {
    if (parameter_ref.is_system())
      return &system_parameters_[util::enum_to_i(parameter_ref.system_param)];
    return nullptr;
  }

private:
  system_parameter_array system_parameters_ = detail::build_parameter_array<SYSTEM>();

  template <auto parameter, typename T>
  auto get() const
  {
    return detail::typed_value<T>(get<parameter>().value());
  }
};

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PARAMETER_STRUCTS_H_
