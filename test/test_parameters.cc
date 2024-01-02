#include "fmt/core.h"
#include "gtest/gtest.h"
#include "synth/parameter_structs.h"
#include "synth/parameters.h"

namespace pfm2sid::test {

TEST(ParametersTest, basics)
{
  fmt::println("global_parameter_count={}", synth::kNumGlobalParameters);
  fmt::println("voice_parameter_count={}", synth::kNumVoiceParameters);
  fmt::println("lfo_parameter_count={}", synth::kNumLfoParameters);
}

static void Print(const synth::ParameterDesc *desc)
{
  fmt::println("{} = {{ min_value={}, max_value={}, default_value={} }}", desc->name,
               desc->min_value, desc->max_value, desc->default_value);
}

TEST(ParametersTest, ParameterRef)
{
  using synth::GLOBAL;
  using synth::PARAMETER_SCOPE;
  using synth::ParameterRef;

  ParameterRef ref;
  EXPECT_EQ(PARAMETER_SCOPE::NONE, ref.type);

  ParameterRef global_ref(GLOBAL::FILTER_MODE);
  EXPECT_EQ(PARAMETER_SCOPE::GLOBAL, global_ref.type);
  EXPECT_EQ(GLOBAL::FILTER_MODE, global_ref.global_param);
}

TEST(ParametersTest, global_parameter_array)
{
  using synth::GLOBAL;
  auto arr = synth::detail::build_parameter_array<GLOBAL>();

  for (auto &value : arr) {
    ASSERT_NE(value.desc(), nullptr);
    EXPECT_TRUE(value.desc()->parameter.is_global()) << value.desc()->name;
    Print(value.desc());
  }
}

TEST(ParametersTest, voice_parameter_array)
{
  using synth::VOICE;
  auto arr = synth::detail::build_parameter_array<VOICE>();
  for (auto &value : arr) {
    ASSERT_NE(value.desc(), nullptr);
    EXPECT_TRUE(value.desc()->parameter.is_voice());
    Print(value.desc());
  }
}

TEST(ParametersTest, lfo_parameter_array)
{
  using synth::LFO;
  auto arr = synth::detail::build_parameter_array<LFO>();
  for (auto &value : arr) {
    ASSERT_NE(value.desc(), nullptr);
    EXPECT_TRUE(value.desc()->parameter.is_lfo());
    Print(value.desc());
  }
}

}  // namespace pfm2sid::test
