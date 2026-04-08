# OpenSpatialDelay Release Test Checklist

Reusable manual listening/visual test checklist for validating builds before release.
Automated tests (291 Catch2 tests, 110,205 assertions) cover DSP correctness; this checklist covers
perceptual quality, DAW integration, and UI behavior that require human evaluation.

---

**Tester:** ________________  **Date:** ________________
**DAW:** REAPER __________  **Version:** _____________
**Build:** v______  **Commit:** ________________________

---

## A. Output Format Verification (23 formats)

**Method:** Load a preset with active taps. Switch output format via header dropdown.
Verify signal appears on correct channels via REAPER routing matrix or channel meters.

- [ ] A1.  Binaural (HRTF) -- 2ch, spatialized L+R
- [ ] A2.  Stereo -- 2ch, panned L+R
- [ ] A3.  Quad -- 4ch (FL/FR/RL/RR)
- [ ] A4.  5.0 -- 5ch (L/C/R/Ls/Rs)
- [ ] A5.  5.1 -- 6ch (5.0 + LFE on ch4)
- [ ] A6.  7.0 -- 7ch (L/C/R/Lss/Rss/Lrs/Rrs)
- [ ] A7.  7.1 -- 8ch (7.0 + LFE on ch4)
- [ ] A8.  9.1 -- 10ch (ITU-R BS.2051 System H, ear-level only)
- [ ] A9.  Octaphonic -- 8ch (45-degree spaced ring)
- [ ] A10. 5.1.2 -- 8ch (5.1 + Lts/Rts height)
- [ ] A11. 5.1.4 -- 10ch (5.1 + 4 height)
- [ ] A12. 7.1.2 -- 10ch (7.1 + 2 height)
- [ ] A13. 7.1.4 Atmos -- 12ch (7.1 + 4 height)
- [ ] A14. 7.1.6 -- 14ch (7.1 + 6 height)
- [ ] A15. 9.1.4 -- 14ch (9.1 + 4 height)
- [ ] A16. 9.1.6 -- 16ch (9.1 + 6 height)
- [ ] A17. SML 13.1 -- 14ch (SML multi-use room)
- [ ] A18. FOA (1st order ambi) -- 4ch ACN/SN3D
- [ ] A19. SOA (2nd order ambi) -- 9ch ACN/SN3D
- [ ] A20. HOA (3rd order ambi) -- 16ch ACN/SN3D
- [ ] A21. 4OA (4th order ambi) -- 25ch ACN/SN3D
- [ ] A22. 5OA (5th order ambi) -- 36ch ACN/SN3D
- [ ] A23. 6OA (6th order ambi) -- 49ch ACN/SN3D

**Notes:** ________________________________________________________

---

## B. HRTF Profile Switching (6 profiles)

**Method:** Set Binaural output. Play pink noise or a drum loop.
Switch profiles via header dropdown. Listen for artifacts on switch.

- [ ] B1.  Simple (Low CPU) -- Woodworth ITD+ILD, no convolution
- [ ] B2.  Studio Reference -- MIT KEMAR, neutral classic standard
- [ ] B3.  Immersive -- SADIE II D2 KU100, rich spatial detail
- [ ] B4.  Natural -- CIPIC Subject 003, organic rendering
- [ ] B5.  Precise -- HUTUBS PP2, analytical accuracy
- [ ] B6.  Spatial -- Bernschuetz KU100, widest coverage

Per-profile verification:
- [ ] B7.  No click/pop on profile switch (crossfade functional)
- [ ] B8.  Front/back distinction audible (move tap from 0 to 180 deg)
- [ ] B9.  Elevation changes audible (move tap from 0 to +45 deg)

**Notes:** ________________________________________________________

---

## C. Trajectory Shapes (14 shapes)

**Method:** Enable 1 tap, set trajectory speed ~1.0 Hz, Doppler ~50%.
Visually confirm spatial map animation. Audibly confirm smooth movement.

- [ ] C1.  None -- dot stays stationary
- [ ] C2.  Bounce -- vertical bouncing motion
- [ ] C3.  Circle -- circular orbit in azimuth plane
- [ ] C4.  Cross -- X-shaped crossing pattern
- [ ] C5.  Figure-8 -- two tangent circles
- [ ] C6.  Heart -- cardioid path
- [ ] C7.  Helix -- spiral with elevation change
- [ ] C8.  Infinity -- horizontal lemniscate
- [ ] C9.  Line -- back-and-forth linear motion
- [ ] C10. Orbit -- circular azimuth only
- [ ] C11. Random -- randomized position changes
- [ ] C12. Spiral -- expanding/contracting spiral
- [ ] C13. Square -- rectangular path
- [ ] C14. Triangle -- triangular path

Additional checks:
- [ ] C15. Forward direction plays correctly
- [ ] C16. Reverse direction reverses the motion path
- [ ] C17. Speed 0.1 Hz = very slow, 5.0 Hz = fast
- [ ] C18. Global Tap Speed drawer knob offsets all trajectories

**Notes:** ________________________________________________________

---

## D. Preset Cycling (70 factory presets)

**Method:** Feed a drum loop or pink noise. Cycle through all presets using
Next/Prev buttons. Listen for clicks/pops on transitions and verify each
preset sounds distinct and intentional.

- [ ] D1.  Classic Delays category -- all presets load, sound appropriate
- [ ] D2.  Spatial Movement category -- moving sources audible
- [ ] D3.  Ambient + Texture category -- lush/washy character
- [ ] D4.  Height + 3D category -- elevation movement audible (binaural/Atmos)
- [ ] D5.  Surround Production category -- multichannel routing correct
- [ ] D6.  Wobble + Modulated category -- modulation audible
- [ ] D7.  Creative + Experimental category -- unusual/extreme settings
- [ ] D8.  Rhythmic category -- rhythmic patterns audible
- [ ] D9.  Transitions between adjacent presets are click-free
- [ ] D10. Category Next/Prev stays within category
- [ ] D11. Preset index wraps at boundaries (last -> first, first -> last)
- [ ] D12. User preset save: name input, category selection, save/cancel
- [ ] D13. User preset load: saved preset restores correctly
- [ ] D14. User preset overwrite: overwriting works without crash

**Notes:** ________________________________________________________

---

## E. DAW Integration (REAPER)

### Bus Negotiation
- [ ] E1.  Stereo track -> plugin reports 2 output channels
- [ ] E2.  Multichannel track (16ch) -> correct channel count per format
- [ ] E3.  Switching output format changes bus layout live (no crash)
- [ ] E4.  Ambisonics (16ch HOA) bus negotiation correct

### Plugin Delay Compensation
- [ ] E5.  PDC reports 2048 samples (check via REAPER plugin info)
- [ ] E6.  Dry signal at 0% wet is time-aligned with bypassed plugin
- [ ] E7.  PDC still correct after sample rate change (44.1k -> 48k -> 96k)

### Automation
- [ ] E8.  All 144 automatable parameters appear in REAPER automation
- [ ] E9.  Config params (algorithm, HRTF, output format) do NOT appear
- [ ] E10. Automating Dry/Wet produces smooth, click-free sweep
- [ ] E11. Automating azimuth produces smooth panning
- [ ] E12. Automating delay time produces smooth change (no clicks)

### State Save/Load
- [ ] E13. Save REAPER project -> close -> reopen -> plugin state restored
- [ ] E14. Config params (algorithm, HRTF profile) restored correctly
- [ ] E15. Preset selection index restored
- [ ] E16. OSC settings (port, IP, enabled) restored
- [ ] E17. Undo after parameter change restores previous value

**Notes:** ________________________________________________________

---

## F. Regression Checks (closed issues)

Spot-check key bugs from the v1.0 development cycle to prevent regressions.

- [ ] F1.  #53 -- WSOLA clicks: pitch shift sounds clean, no metallic artifacts
- [ ] F2.  #60 -- Phase vocoder: ±12 semitone range works, transients preserved
- [ ] F3.  #62 -- Multiple instances: open 2+ instances, no crash
- [ ] F4.  #63 -- Dry path latency: dry signal at 0% wet matches bypassed timing
- [ ] F5.  #65 -- No PV buzz after transport stop/start/seek
- [ ] F6.  #67 -- No feedback chirp when enabling/disabling taps
- [ ] F7.  #68 -- Config params hidden from DAW automation list
- [ ] F8.  #73 -- Stereo dry signal preserved (not collapsed to mono)
- [ ] F9.  #73 -- Equal-power crossfade: no volume dip at 50% dry/wet
- [ ] F10. #76 -- MS Encode: no center collapse at ±90 deg azimuth
- [ ] F11. #77 -- No Doppler buzzing on moving objects
- [ ] F12. #84 -- No chirping on preset changes

**Notes:** ________________________________________________________

---

## G. Edge Cases and Stress Tests

- [ ] G1.  All 12 taps enabled simultaneously -- no CPU spike or crash
- [ ] G2.  All 12 taps with trajectories at max speed (5 Hz)
- [ ] G3.  Maximum feedback (0.99) with pitch shift (+12) -- no runaway
- [ ] G4.  Minimum delay time (1ms) with high feedback -- stable
- [ ] G5.  Maximum delay time (2000ms) -- no memory error
- [ ] G6.  Rapid preset switching (click Next 20 times quickly)
- [ ] G7.  Sample rate change mid-session (48k -> 96k -> 44.1k)
- [ ] G8.  Buffer size change mid-session (256 -> 1024 -> 64)
- [ ] G9.  Mono input track -> plugin functions correctly
- [ ] G10. Open plugin UI -> close -> reopen -> state preserved
- [ ] G11. Two instances of plugin on same track -- no conflict
- [ ] G12. NaN recovery: set feedback to max, verify soft clip prevents runaway

**Notes:** ________________________________________________________

---

## Sign-Off

| | |
|---|---|
| All sections passed | [ ] YES  [ ] NO (see notes) |
| Tester signature | ________________ |
| Date | ________________ |
