# Misc Implemenation Notes

## reSID
- All the resampling methods are disabled since anything but `SAMPLE_FAST` takes "too long" (at least in a first test)
- Newer implementations (i.e. above 1.x) have a more streamlined output sample generation, but other drawbacks...

### Output value ranges
For one thing, this magic computation in `SID::output()` seems to be gone, or at least folded in to the rest of the improvements like the DAC modelling.
```
  int output_sample = extfilt.output()/((4095*255 >> 7)*3*15*2/range);
```
This works out as `extfilt.output()/11` for the default 16-bit range. My guess is that the full range of the source 20 (20.5?) bits are seldom used, so this is leaving more range than the simple `>>4` and just risks hard clipping some extremes.

![plot](./20_16.png)

In this use case we might prefer an 18-bit value anyway and have floats available, so it may be simplified to just output the _raw_ value and we'll post-process it later, e.g. soft clipping. This also avoids moving platform-specifics like `__SSAT` into the reSID code.

### Clocking
- By using a fixed (or compile-time) `cycles_per_sample` and `clock_delta_t` which could just skip all the sample tracking? 
### FreeRTOS/interrupts
Another "satisfy my curiosity" addition to the project: FreeRTOS. Turns out this is useful for putting USB IO "in the background" but requires some care around the interrupts.

- There's a separation between "safe" ISRs and ones of higher priority that cannot use any of the RTOS API.
- The main DACs are run off of such a high-priority timer ISR.
- Calculating sample blocks needs to be prioritised as well but can't starve other things.
- So the DAC ISR fires a secondary ISR within the safe range which wakes the rendering thread. Good times.
- Pre RTOS, the UI tick would update the display 1 character per tick to avoid blocking the "main loop". Now the main loop updates the display (but still in smaller chunks).
