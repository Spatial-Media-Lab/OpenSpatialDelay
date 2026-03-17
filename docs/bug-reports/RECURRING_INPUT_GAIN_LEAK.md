# RECURRING BUG: INPUT Knob Affecting Dry/Source Signal

**Status:** Fixed (v0.9) — RECURRING, has regressed before
**Severity:** High — audible level change on source signal when INPUT knob is adjusted
**Occurrences:** First fixed in earlier version, regressed in v0.9

## Symptom
When the INPUT knob is turned up/down, the dry (source) signal in the output mix changes level. The INPUT knob should ONLY control the signal entering the delay line, not the dry pass-through.

## Root Cause
In the per-sample output mix, `rawInput` (the dry signal mixed with wet) gets incorrectly multiplied by `inGain`. This makes the dry signal track the INPUT knob instead of remaining at unity.

**Incorrect pattern (causes bug):**
```cpp
float rawInput = monoInputBuffer[static_cast<size_t>(s)] * inGain;  // WRONG for dry mix
```

**Correct pattern (from HRTF binaural path, the reference implementation):**
```cpp
float rawInput = monoInputBuffer[static_cast<size_t>(s)];  // CORRECT — no inGain on dry path
```

## Why It Recurs
There are 5 render methods that each have their own per-sample loop with a `rawInput` variable:
1. `renderBinauralHRTF()` — Pass 3 (line ~3785) — **CORRECT reference**
2. `renderSimpleBinauralWoodworth()` — line ~3828
3. `renderStereoVariant()` — line ~3950
4. `renderAmbisonicsOutput()` — line ~4068
5. `renderDiscreteSurround()` — line ~4148

When new render methods are added or existing ones are modified, it's easy to copy a line that includes `* inGain` without realizing it should only apply to `rawL`/`rawR` (the delay input), not `rawInput` (the dry mix signal).

## Fix
Remove `* inGain` from the `rawInput` assignment in all render methods. The `rawL`/`rawR` lines that feed `writeDelayLine()` correctly keep `* inGain` — those are the delay input and should be scaled.

## Prevention Checklist
- [ ] When adding/modifying render methods, verify `rawInput` does NOT include `* inGain`
- [ ] When copying code between render methods, check all gain multipliers
- [ ] Reference `renderBinauralHRTF()` Pass 3 as the canonical correct pattern
- [ ] After any render method change, test: set INPUT to min, DRY/WET to 100% dry → output should be at unity
