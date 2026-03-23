# OneMegaHertzPulse

An Arduino sketch that generates a precise **1 MHz square wave** on pins 9 and 10 using the ATmega328P's hardware Timer1. The CPU does zero work after setup — the waveform is produced entirely by the timer hardware, giving cycle-accurate output with no jitter.

---

## Hardware Required

- Arduino Uno, Nano, or any board based on the **ATmega328P** running at **16 MHz**
- Oscilloscope or frequency counter to observe the output (the signal is far too fast to see on an LED)

---

## Output

| Pin | Timer Channel | Frequency | Duty Cycle |
|-----|--------------|-----------|------------|
| 9   | OC1A         | 1 MHz     | 50%        |
| 10  | OC1B         | 1 MHz     | 50%        |

> **Note:** Pins 9 and 10 may be in phase or 180° out of phase with each other depending on the initial pin state at power-on. This is non-deterministic across power cycles. If a guaranteed phase relationship between the two outputs is required, `OCR1B` should be set explicitly.

---

## How It Works

The sketch configures Timer1 in **CTC mode** (Clear Timer on Compare Match) with the output compare pins set to toggle on each match.

```cpp
TCCR1A = _BV(COM1A0) | _BV(COM1B0); // Toggle OC1A (pin 9) and OC1B (pin 10) on compare match
TCCR1B = _BV(WGM12)  | _BV(CS10);  // CTC mode, no prescaler (timer runs at 16 MHz)
OCR1A  = 7;                         // Count 0–7 (8 steps), then reset and toggle
```

The timer counts from 0 up to the value in `OCR1A`, resets to 0, and toggles the output pins on every reset. Because one complete waveform cycle requires two toggles (LOW→HIGH and HIGH→LOW), the output frequency is:

```
f = F_CPU / (2 × prescaler × (OCR1A + 1))
f = 16,000,000 / (2 × 1 × (7 + 1))
f = 16,000,000 / 16
f = 1,000,000 Hz  →  1 MHz
```

### Why `OCR1A + 1`?

The counter is zero-indexed — it counts **0, 1, 2, 3, 4, 5, 6, 7** before resetting. That is 8 steps, which is why the divisor is `OCR1A + 1 = 8`.

---

## Changing the Frequency

To generate a different frequency, adjust `OCR1A` using the formula above rearranged:

```
OCR1A = (F_CPU / (2 × prescaler × desired_frequency)) - 1
```

### Common Frequencies (no prescaler, 16 MHz clock)

| Desired Frequency | OCR1A Value |
|-------------------|:-----------:|
| 8 MHz             | 0           |
| 4 MHz             | 1           |
| 2 MHz             | 3           |
| 1 MHz             | 7           |
| 500 kHz           | 15          |
| 100 kHz           | 79          |
| 10 kHz            | 799         |
| 1 kHz             | 7999        |

For frequencies below ~500 Hz, use a prescaler by replacing `CS10` in `TCCR1B`:

| Prescaler | Bit to set | Lowest frequency |
|-----------|-----------|-----------------|
| 1         | `CS10`    | ~122 Hz         |
| 8         | `CS11`    | ~15 Hz          |
| 64        | `CS11\|CS10` | ~1.9 Hz      |
| 256       | `CS12`    | ~0.47 Hz        |
| 1024      | `CS12\|CS10` | ~0.12 Hz     |

---

## Why Hardware Timer Instead of `digitalWrite()`?

A common alternative approach uses `digitalWrite()` and `micros()` in a busy-wait loop. This has serious limitations at high frequencies:

| | Busy-wait (`digitalWrite`) | Hardware Timer (this sketch) |
|--|:--:|:--:|
| CPU usage | 100% | 0% |
| Timing accuracy | ±4 µs + ISR jitter | ±62.5 ns (one clock cycle) |
| Max practical frequency | ~500 Hz | 8 MHz |
| Susceptible to interrupt jitter | Yes | No |

At 1 MHz, a half-period is only **500 ns**. `digitalWrite()` alone takes ~3,400 ns — more than six times the entire half-period. Hardware CTC mode has none of these limitations.

---

## Limitations

- **50% duty cycle only.** CTC toggle mode always produces an exactly symmetrical square wave. For variable duty cycle, use Fast PWM or Phase-Correct PWM mode with `OCR1B` as the compare threshold.
- **ATmega328P at 16 MHz only.** On a 3.3V/8 MHz board the same `OCR1A = 7` value will produce 500 kHz. Recalculate using the formula above for your clock speed.
- **Pins 9 and 10 only.** `OC1A` and `OC1B` are fixed hardware functions of Timer1 and are only available on these pins on the ATmega328P.
- **Timer1 is consumed.** The Arduino `Servo` library and some other libraries also use Timer1. Using this sketch will conflict with any library that depends on Timer1.

---

## License

GNU General Public License v2.0 — see [LICENSE](LICENSE) for details.
