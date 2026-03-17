# IndicatorToggle — Reusable Toggle Button Component

## Context

The plugin has 5 toggle buttons (MOD, FLT, AIR, RECEIVE, SEND) that should look identical but are implemented inconsistently:

- **AIR, RECEIVE, SEND** — `juce::TextButton` instances with external `paintStyledToggle()` rendering, 10px font, dynamic widths
- **FLT, MOD** — Manually painted in `paint()` with manual `mouseDown`/`mouseMove`/`mouseExit` hit-testing, **9.5px font**, **hardcoded widths** (38px/42px)

This causes MOD/FLT to appear visually smaller than the other buttons. The fix creates a single reusable `IndicatorToggle` class that replaces all 5 implementations with consistent sizing, font, and interaction.

## Design

### New Class: `IndicatorToggle : public juce::Button`

**Location:** Declared in `PluginEditor.h` (line ~269, between `FilterGraphComponent` and `OpenSpatialDelayEditor`). Implemented in `PluginEditor.cpp` (after `makeFont` helper, before `Ableton12Look` constructor).

**Constructor:** `IndicatorToggle(const juce::String& label, const juce::Colour& accentColour, juce::Typeface::Ptr typeface)`
- Calls `setClickingTogglesState(true)` internally — callers override to `false` for timer-driven buttons (FLT, MOD)
- No LookAndFeel setup needed — `paintButton()` handles all rendering

**Static method:** `static int getPreferredWidth(juce::Typeface::Ptr tf, const juce::String& text)`
- Formula: `6px left + 5px dot + 4px gap + textWidth + 7px right`
- Replaces the `measureToggle` lambda in `resized()`

**Visual constants (all `static constexpr`):**
| Constant | Value | Purpose |
|----------|-------|---------|
| `kFontSize` | 10.0f | JetBrains Mono Medium size (fixes 9.5f bug) |
| `kKerning` | 0.08f | Letter spacing |
| `kDotRadius` | 2.5f | Indicator dot radius (5px diameter) |
| `kLeftPad` | 6.0f | Left padding before dot |
| `kDotGap` | 4.0f | Gap between dot and text |
| `kRightPad` | 7.0f | Right padding after text |
| `kCornerR` | 4.0f | Border radius |
| `kHeight` | 18 | Standard height |

**`paintButton()` override** — pixel-exact transplant of existing `paintStyledToggle()` logic:
- Background: ON = accent 8% alpha, HOVER adds +6%, OFF = bgRecessed
- Border: ON = accent 60% alpha, HOVER bumps to 85%, OFF hover = 30%
- Dot: ON = accent + glow ring (accent 25% alpha), OFF = textDim 40%
- Text: ON = accent, HOVER = textSecondary, OFF = textDim
- Font: `makeFont(typeface, kFontSize, kKerning)` — JetBrains Mono Medium 10px

### Member Replacements in `OpenSpatialDelayEditor`

| Before | After |
|--------|-------|
| `juce::TextButton oscToggleButton` | `std::unique_ptr<IndicatorToggle> oscToggleButton` |
| `juce::TextButton oscSendToggleButton` | `std::unique_ptr<IndicatorToggle> oscSendToggleButton` |
| `juce::TextButton airAbsorptionButton` | `std::unique_ptr<IndicatorToggle> airAbsorptionButton` |
| *(no member)* — painted tag | `std::unique_ptr<IndicatorToggle> fltToggle` |
| *(no member)* — painted tag | `std::unique_ptr<IndicatorToggle> modToggle` |
| `bool fltTagHovered` | **REMOVE** — IndicatorToggle handles hover |
| `bool modTagHovered` | **REMOVE** — IndicatorToggle handles hover |

**Kept:** `filterIsActive` (line 460), `modIsActive` (line 292) — still needed in `timerCallback()` for dimming sibling components.

**`unique_ptr` rationale:** Constructor needs `ableton12Look.jetbrainsMedium` which is only available after `Ableton12Look` construction. Stack members can't take constructor args that depend on other members.

### Wiring Per Button

**AIR** (APVTS-driven, auto-toggle):
```cpp
airAbsorptionButton = std::make_unique<IndicatorToggle> ("AIR", Colours_OSD::accentStellar,
                                                          ableton12Look.jetbrainsMedium);
addAndMakeVisible (*airAbsorptionButton);
airAbsorptionAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, "airAbsorption",
                                                          *airAbsorptionButton);
```

**RECEIVE** (APVTS-driven, auto-toggle):
```cpp
oscToggleButton = std::make_unique<IndicatorToggle> ("RECEIVE", Colours_OSD::accentGreen,
                                                      ableton12Look.jetbrainsMedium);
addAndMakeVisible (*oscToggleButton);
oscToggleAttach = std::make_unique<ButtonAttachment> (processorRef.apvts, "admOscEnabled",
                                                      *oscToggleButton);
```

**SEND** (callback-driven, auto-toggle):
```cpp
oscSendToggleButton = std::make_unique<IndicatorToggle> ("SEND", Colours_OSD::accentGreen,
                                                          ableton12Look.jetbrainsMedium);
oscSendToggleButton->onClick = [this] {
    processorRef.setOscSendEnabled (oscSendToggleButton->getToggleState());
};
addAndMakeVisible (*oscSendToggleButton);
```

**FLT** (timer-driven, NO auto-toggle — visual state set by timerCallback):
```cpp
fltToggle = std::make_unique<IndicatorToggle> ("FLT", Colours_OSD::accentViolet,
                                                ableton12Look.jetbrainsMedium);
fltToggle->setClickingTogglesState (false);  // timer drives visual state
fltToggle->onClick = [this] {
    auto* hpParam = processorRef.apvts.getParameter ("filterHP");
    auto* lpParam = processorRef.apvts.getParameter ("filterLP");
    if (hpParam != nullptr && lpParam != nullptr)
    {
        float hpHz = processorRef.apvts.getRawParameterValue ("filterHP")->load();
        float lpHz = processorRef.apvts.getRawParameterValue ("filterLP")->load();
        bool isAtDefaults = (hpHz < 21.0f && lpHz > 19999.0f);
        if (isAtDefaults)
        {
            hpParam->setValueNotifyingHost (hpParam->convertTo0to1 (200.0f));
            lpParam->setValueNotifyingHost (lpParam->convertTo0to1 (8000.0f));
        }
        else
        {
            hpParam->setValueNotifyingHost (hpParam->convertTo0to1 (20.0f));
            lpParam->setValueNotifyingHost (lpParam->convertTo0to1 (20000.0f));
        }
    }
};
addAndMakeVisible (*fltToggle);
// Set initial visual state from current parameter values
{
    float hp = processorRef.apvts.getRawParameterValue ("filterHP")->load();
    float lp = processorRef.apvts.getRawParameterValue ("filterLP")->load();
    filterIsActive = (hp > 21.0f || lp < 19900.0f);
    fltToggle->setToggleState (filterIsActive, juce::dontSendNotification);
}
```

**MOD** (timer-driven, NO auto-toggle — visual state set by timerCallback):
```cpp
modToggle = std::make_unique<IndicatorToggle> ("MOD", Colours_OSD::accentRose,
                                                ableton12Look.jetbrainsMedium);
modToggle->setClickingTogglesState (false);  // timer drives visual state
modToggle->onClick = [this] {
    auto* param = processorRef.apvts.getParameter ("wobbleEnabled");
    if (param != nullptr)
    {
        float cur = processorRef.apvts.getRawParameterValue ("wobbleEnabled")->load();
        param->setValueNotifyingHost (cur > 0.5f ? 0.0f : 1.0f);
    }
};
addAndMakeVisible (*modToggle);
// Set initial visual state
modIsActive = processorRef.apvts.getRawParameterValue ("wobbleEnabled")->load() > 0.5f;
modToggle->setToggleState (modIsActive, juce::dontSendNotification);
```

**Key design decision:** FLT and MOD use `setClickingTogglesState(false)` because their visual state is driven by `timerCallback()` (which reads actual parameter values and updates sibling component states). The onClick just flips the parameter; the timer picks it up on the next tick (~33ms at 30Hz, imperceptible). AIR, RECEIVE, and SEND use `setClickingTogglesState(true)` (the constructor default) because their state is managed by APVTS ButtonAttachment or direct toggle.

### Colour Namespace Update

Add `accentRose` to `Colours_OSD` namespace (currently a `static const` local at PluginEditor.cpp:2267 inside the MOD paint block). Place after `accentGreenDim` at line ~47:
```cpp
static const juce::Colour accentRose       (0xffe467a6);  // oklch(70% 0.14 350) — mod section
```
Remove the `static const juce::Colour accentRose (0xffe467a6);` local at line 2267.

### unique_ptr Dereference Sites

All `.` access on the 3 renamed members must become `->` (or `*` for reference params). Exhaustive list:

| Call site | Change |
|-----------|--------|
| `addAndMakeVisible(oscToggleButton)` | `addAndMakeVisible(*oscToggleButton)` |
| `addAndMakeVisible(oscSendToggleButton)` | `addAndMakeVisible(*oscSendToggleButton)` |
| `addAndMakeVisible(airAbsorptionButton)` | `addAndMakeVisible(*airAbsorptionButton)` |
| `ButtonAttachment(..., oscToggleButton)` | `ButtonAttachment(..., *oscToggleButton)` |
| `ButtonAttachment(..., airAbsorptionButton)` | `ButtonAttachment(..., *airAbsorptionButton)` |
| `oscSendToggleButton.getToggleState()` in onClick lambda | `oscSendToggleButton->getToggleState()` |
| `oscSendToggleButton.onClick = ...` | `oscSendToggleButton->onClick = ...` |
| `airAbsorptionButton.setBounds(...)` in resized() | `airAbsorptionButton->setBounds(...)` |
| `oscToggleButton.setBounds(...)` in resized() | `oscToggleButton->setBounds(...)` |
| `oscSendToggleButton.setBounds(...)` in resized() | `oscSendToggleButton->setBounds(...)` |
| `oscToggleButton.getBounds()` in paint() (~line 2309) | `oscToggleButton->getBounds()` |
| `oscSendToggleButton.getBounds()` in paint() (~line 2321) | `oscSendToggleButton->getBounds()` |

### Section Header Width Updates

Currently hardcoded widths in `paint()` must use dynamic toggle widths (since FLT/MOD widths change from 9.5px→10px):
```cpp
// Before (hardcoded):
drawSectionHeader (g, rpX, toneHeaderY, rpW - 42, "TONE");
drawSectionHeader (g, rpX, modHeaderY,  rpW - 46, "MOD");
drawSectionHeader (g, rpX, mixHeaderY,  rpW - 42, "MIX");

// After (dynamic from component width):
drawSectionHeader (g, rpX, toneHeaderY, rpW - fltToggle->getWidth() - 4, "TONE");
drawSectionHeader (g, rpX, modHeaderY,  rpW - modToggle->getWidth() - 4, "MOD");
drawSectionHeader (g, rpX, mixHeaderY,  rpW - airAbsorptionButton->getWidth() - 4, "MIX");
```

### measureToggle Capitalization Fix

The existing `measureToggle` lambda uses mixed case ("Receive", "Send") but `paintStyledToggle` renders uppercase ("RECEIVE", "SEND"). Since JetBrains Mono is monospace this has no practical width impact, but `getPreferredWidth` calls should use the exact label strings for correctness:
```cpp
int rcvW = IndicatorToggle::getPreferredWidth (jbm, "RECEIVE");
int sndW = IndicatorToggle::getPreferredWidth (jbm, "SEND");
```

### Code Removed

| What | Location |
|------|----------|
| `paintStyledToggle()` declaration | PluginEditor.h:309-311 |
| `paintStyledToggle()` implementation | PluginEditor.cpp:1905-1947 |
| 3x `paintStyledToggle()` calls in `paint()` | PluginEditor.cpp:2096-2098 |
| FLT painted tag block in `paint()` | PluginEditor.cpp:2224-2263 |
| MOD painted tag block in `paint()` | PluginEditor.cpp:2265-2305 |
| FLT mouseDown handler | PluginEditor.cpp:2627-2658 |
| MOD mouseDown handler | PluginEditor.cpp:2661-2678 |
| FLT mouseMove hover | PluginEditor.cpp:2721-2731 |
| MOD mouseMove hover | PluginEditor.cpp:2733-2745 |
| FLT/MOD mouseExit reset | PluginEditor.cpp:2755-2764 |
| `measureToggle` lambda + usage | PluginEditor.cpp:2491-2499 |
| TextButton setup (oscToggle: ~1233-1241, air: ~1271-1279, oscSend: ~1282-1293) | ~30 lines of button-specific code (not the oscPortLabel setup lines 1243-1268) |

### Code Added

| What | Lines |
|------|-------|
| `IndicatorToggle` class declaration | ~25 |
| `IndicatorToggle` implementation (constructor, getPreferredWidth, paintButton) | ~55 |
| 5 button constructions + onClick lambdas + initial state | ~55 |
| `getPreferredWidth` calls + setBounds in `resized()` | ~12 |
| `setToggleState` calls in `timerCallback()` | ~2 |
| `accentRose` in Colours_OSD | ~1 |
| **Total added** | **~150** |

### Files Modified

- `Project/OpenSpatialDelay/Source/PluginEditor.h` — Add IndicatorToggle class (~25 lines, between FilterGraphComponent and OpenSpatialDelayEditor), replace 3 TextButton members with unique_ptr<IndicatorToggle>, add fltToggle + modToggle members, remove `fltTagHovered`/`modTagHovered` bools, remove `paintStyledToggle` declaration
- `Project/OpenSpatialDelay/Source/PluginEditor.cpp` — Add IndicatorToggle impl (~55 lines after makeFont), add accentRose to Colours_OSD, rewrite constructor button setup (5 buttons), update resized() with getPreferredWidth + setBounds for all 5, remove paintStyledToggle impl + 3 paint calls + FLT/MOD paint blocks + FLT/MOD mouseDown/mouseMove/mouseExit handlers, update timerCallback with setToggleState, update section header widths, update all `.` to `->` for unique_ptr members (see dereference table above)

### Verification

1. **Build:** `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release` — 0 warnings
2. **Visual:** Load in DAW, verify all 5 toggles show consistent size/font, MOD and FLT match AIR/SEND/RECEIVE
3. **States:** For each toggle, verify: OFF (dim dot, dim text, dark bg), ON (accent dot+glow, accent text, tinted bg+border), HOVER OFF (brighter dot/border), HOVER ON (brighter border)
4. **Functionality:**
   - AIR: toggle air absorption on/off, verify DSP responds
   - RECEIVE: toggle OSC receive, verify port connection
   - SEND: toggle OSC send, verify sender connects
   - FLT: click to toggle filters between active (HP 200/LP 8k) and bypass (HP 20/LP 20k), verify filter graph updates
   - MOD: click to enable/disable wobble, verify wobble knobs dim when off
5. **State persistence:** Save and reload preset/session, verify all toggle states restore correctly
