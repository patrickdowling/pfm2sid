project('pfm2sid', 'cpp', default_options: ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

inc = [
  '../src',
  '../resources',
  '../stm32x/include',
  '../extern'
  ]

test_src = [
  'pfm2sid_test.cc',
  'test_fp.cc',
  'test_midi.cc',
  'test_midi_sysex.cc',
  'test_parameters.cc',
  'test_patch.cc',
  'test_sidbits.cc',
  'test_glide.cc',
  'test_lfo.cc',
  'test_asid_parser.cc',
  'test_sorted_array.cc',
  'test_static_stack.cc',
  'test_wavetable.cc',
  'test_voice_allocator.cc',
  'test_resid_constexpr.cc',
  ]

src = [
  '../src/midi/midi_parser.cc',
  '../src/synth/glide.cc',
  '../src/synth/lfo.cc',
  '../src/synth/parameters.cc',
  '../src/synth/patch.cc',
  '../src/synth/wavetable.cc',
  '../src/sidbits/sidbits.cc',
  '../src/sidbits/asid_parser.cc',
  ]

extern_src = [
  '../extern/reSID/src/filter.cc'
  ]

gtest_dep = dependency('gtest', main : true, required: true)
fmt_dep = dependency('fmt', required: true)

pfm2sid_test = executable(
  'pfm2sid_test',
  cpp_args : [ '-DMIDI_TRACE_FMT=fmt::println' ],
  sources : [ test_src, src, extern_src ],
  include_directories : inc,
  dependencies : [ gtest_dep, fmt_dep ])

test('pfm2sid_test', pfm2sid_test)
