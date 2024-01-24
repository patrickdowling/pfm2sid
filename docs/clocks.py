#!/usr/bin/env python3
#
# Print some combinations of clocks per sample, sample rate, etc. based on the number of SID cycles per sample.
# This might give some insight into tradeoffs or which settings might be most useful.

import math

CLOCK_FREQ_PAL = 985248
block_size = 32

print(f"CLOCK_FREQ_PAL={CLOCK_FREQ_PAL}")

def eval(cycles, f_cpu):
    sample_freq = CLOCK_FREQ_PAL / cycles
    tim = f_cpu / sample_freq
    actual_sample_freq = f_cpu / math.floor(tim)
    err = (sample_freq - actual_sample_freq) / sample_freq * 100.
    dt = int(round((1000 * 1000) / sample_freq * block_size))

    print(f"cycles={cycles:.1f} {math.ceil(cycles * block_size)}\t{sample_freq:.2f}Khz\ttim={tim:.2f}\t{actual_sample_freq:.2f}Khz\terr={err:.3f}% dt={dt}")


for mhz in [168, 180, 192]:
    f_cpu = mhz * 1000 * 1000
    print(f"F_CPU={f_cpu}")
    eval(22, f_cpu)
    eval(CLOCK_FREQ_PAL / 44100., f_cpu)
    eval(23, f_cpu)
    eval(24, f_cpu)
    eval(25, f_cpu)
    print("")
