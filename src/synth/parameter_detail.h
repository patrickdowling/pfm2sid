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
#ifndef PFM2SID_SYNTH_PARAMETER_DETAIL_H_
#define PFM2SID_SYNTH_PARAMETER_DETAIL_H_

#include <array>
#include <utility>

#include "util/util_macros.h"
#include "util/util_templates.h"

namespace pfm2sid::synth {

class ParameterValue;
class ParameterRef;

namespace detail {

// Building ParameterValue arrays, or arrays of arrays
template <typename parameter_enum, size_t... Is>
constexpr auto build_parameter_array_impl(std::index_sequence<Is...>)
{
  return std::array<ParameterValue, sizeof...(Is)>{
      ParameterRef{static_cast<parameter_enum>(Is)}...};
}

template <typename parameter_enum>
constexpr auto build_parameter_array([[maybe_unused]] size_t i = 0)
{
  return build_parameter_array_impl<parameter_enum>(
      std::make_index_sequence<util::enum_count<parameter_enum>()>());
}

template <typename parameter_enum, size_t... Is>
constexpr auto build_parameter_arrays_impl(std::index_sequence<Is...>)
    -> std::array<std::array<ParameterValue, util::enum_count<parameter_enum>()>, sizeof...(Is)>
{
  return {build_parameter_array<parameter_enum>(Is)...};
}

template <typename parameter_enum, size_t N>
constexpr auto build_parameter_arrays()
{
  return build_parameter_arrays_impl<parameter_enum>(std::make_index_sequence<N>());
}

template <typename T>
void ResetArray(T& arr)
{
  for (auto& v : arr) v.Reset();
}

template <typename T, size_t N>
void ResetArrays(std::array<T, N>& arr)
{
  for (auto& a : arr) ResetArray(a);
}

}  // namespace detail

}  // namespace pfm2sid::synth

#endif  // PFM2SID_SYNTH_PARAMETER_DETAIL_H_
