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
#ifndef PFM2SID_FIXED_POINT_H_
#define PFM2SID_FIXED_POINT_H_

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

// Basic fixed point stuff, mostly assuming int32_t/uint32_t
//
// TODO Rounding?
// TODO We can optimize for cases where used bits <= 16 but stored in an int32_t

namespace util {

// std::clamp seems to do the Right Thing but we reserve the option of overriding it
// e.g. by using #ifdef ARM_MATH_CM4
template <typename FP>
constexpr int32_t SSAT(int32_t value)
{
  return std::clamp(value, FP::kMin, FP::kMax);
}

template <typename FP>
constexpr uint32_t USAT(uint32_t value)
{
  return std::clamp(value, FP::kMin, FP::kMax);
}

struct FixedPointType {};
template <typename FP>
constexpr bool is_fixed_point()
{
  return std::is_base_of_v<FixedPointType, FP>;
}

template <typename T>
struct FixedPointTraits;

template <>
struct FixedPointTraits<int32_t> {
  using value_type = int32_t;
  using intermediate_type = int64_t;
};
template <>
struct FixedPointTraits<uint32_t> {
  using value_type = uint32_t;
  using intermediate_type = uint64_t;
};

template <typename T, T fractional_bits, T integral_bits = 32 - fractional_bits>
class FixedPoint : public FixedPointType {
public:
  using Traits = FixedPointTraits<T>;
  using value_type = typename Traits::value_type;
  using intermediate_type = typename Traits::intermediate_type;

  static_assert(fractional_bits > 0);
  static_assert(fractional_bits < 31);
  static_assert(sizeof(intermediate_type) > sizeof(value_type));
  static_assert(std::is_signed_v<value_type> == std::is_signed_v<intermediate_type>);

  static constexpr T kTotalBits = fractional_bits + integral_bits;

  static constexpr T kMin = std::numeric_limits<T>::min() >> (32 - kTotalBits);
  static constexpr T kMax = std::numeric_limits<T>::max() >> (32 - kTotalBits);

  static constexpr bool enable_saturate = integral_bits != 32 - fractional_bits;

  static constexpr intermediate_type kFracMult = intermediate_type(1) << fractional_bits;
  static constexpr value_type kFracMask = (1 << fractional_bits) - 1;

  constexpr T integral() const { return value_ >> fractional_bits; }
  constexpr T fractional() const { return value_ & kFracMask; }

  constexpr auto value() const { return value_; }

  constexpr bool operator!() const { return value_ == 0; }

  static constexpr auto from_raw(value_type value)
  {
    FixedPoint fp;
    fp.assign(value);
    return fp;
  }

  static constexpr auto from_integral(value_type value) { return FixedPoint(value); }

  static constexpr auto from_float(float f)
  {
    FixedPoint fp;
    auto value = static_cast<value_type>(static_cast<float>(kFracMult) * f);
    fp.assign(value);
    return fp;
  }

  static constexpr auto from_float_clamped(float f)
  {
    FixedPoint fp;
    auto value = static_cast<intermediate_type>(static_cast<float>(kFracMult) * f);
    fp.assign(std::clamp<intermediate_type>(value, kMin, kMax));
    return fp;
  }

  static constexpr auto from(value_type i, value_type f)
  {
    FixedPoint fp;
    fp.assign((i << fractional_bits) + f);
    if constexpr (enable_saturate) fp.saturate();
    return fp;
  }

  constexpr FixedPoint() = default;
  constexpr explicit FixedPoint(T value) : value_(value << fractional_bits)
  {
    if constexpr (enable_saturate) saturate();
  }
  FixedPoint& operator=(const FixedPoint& rhs) = default;

  void assign(int32_t value)
  {
    value_ = value;
    if constexpr (enable_saturate) saturate();
  }

  // Is this a Good Idea? Assigning a plain integer makes it into a FP
  auto operator=(value_type value)
  {
    assign(value << fractional_bits);
    return *this;
  }

  constexpr void saturate()
  {
    if constexpr (std::is_signed_v<value_type>) {
      value_ = SSAT<FixedPoint>(value_);
    } else {
      value_ = USAT<FixedPoint>(value_);
    }
  }

  constexpr auto scale(value_type value) const
  {
    return static_cast<value_type>((static_cast<intermediate_type>(value_) * value) >>
                                   fractional_bits);
  }

  constexpr auto add_integral(value_type value)
  {
    value_ += (value << fractional_bits);
    saturate();
  }

  constexpr auto add_fractional(value_type value)
  {
    value_ += value;
    saturate();
  }

  constexpr auto operator+=(value_type value)
  {
    value_ += value;
    saturate();
  }

private:
  value_type value_ = 0;
};

template <typename FP, typename = std::enable_if_t<is_fixed_point<FP>()>>
FP operator-(FP lhs, FP rhs)
{
  FP result;
  result.assign(lhs.value() - rhs.value());
  return result;
}

template <typename FP, typename = std::enable_if_t<is_fixed_point<FP>()>>
bool operator<(FP lhs, FP rhs)
{
  return lhs.value() < rhs.value();
}
template <typename FP, typename = std::enable_if_t<is_fixed_point<FP>()>>
bool operator<=(FP lhs, FP rhs)
{
  return lhs.value() <= rhs.value();
}
template <typename FP, typename = std::enable_if_t<is_fixed_point<FP>()>>
bool operator>=(FP lhs, FP rhs)
{
  return lhs.value() >= rhs.value();
}

using s824 = FixedPoint<int32_t, 24>;
using s1616 = FixedPoint<int32_t, 16>;
using s816 = FixedPoint<int32_t, 16, 8>;

using u824 = FixedPoint<uint32_t, 24>;
using u1616 = FixedPoint<uint32_t, 16>;

};  // namespace util

#endif  // PFM2SID_FIXED_POINT_H_
