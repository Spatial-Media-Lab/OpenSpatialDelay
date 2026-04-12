---
name: cross-platform-builds
description: Use when building JUCE plugins for macOS/Windows/Linux with CMake, setting up CI/CD, code signing, notarization, or creating installers. Covers AU validation, AAX signing, and GitHub Actions workflows.
---

# Cross-Platform JUCE Plugin Builds

## CMake Quick Setup

```cmake
cmake_minimum_required(VERSION 3.22)
project(MyPlugin VERSION 1.0.0)

add_subdirectory(JUCE)

juce_add_plugin(MyPlugin
    COMPANY_NAME "MyCompany"
    PLUGIN_MANUFACTURER_CODE Myco
    PLUGIN_CODE Mypl
    FORMATS AU VST3 Standalone    # Add AAX if licensed
    PRODUCT_NAME "My Plugin"
    BUNDLE_ID "com.mycompany.myplugin"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_compile_features(MyPlugin PRIVATE cxx_std_17)
target_link_libraries(MyPlugin
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
```

## Platform Checklist

| Task | macOS | Windows | Linux |
|------|-------|---------|-------|
| **Build** | `cmake --build . --config Release` | MSVC 2022 | `apt install` deps first |
| **Universal binary** | `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` | N/A | N/A |
| **Code sign** | `codesign --sign "Developer ID"` | `signtool sign` | N/A |
| **Notarize** | `notarytool submit` | N/A | N/A |
| **AU validate** | `auval -a` | N/A | N/A |
| **Installer** | `.pkg` via `pkgbuild` | `.msi` via WiX / Inno Setup | `.deb` via `dpkg-deb` |

## Code Signing Essentials (macOS)

```bash
# Sign the plugin
codesign --force --sign "Developer ID Application: Your Name (TEAMID)" \
    --options runtime --timestamp \
    "MyPlugin.component"

# Notarize
xcrun notarytool submit MyPlugin.zip \
    --apple-id "you@email.com" \
    --team-id "TEAMID" \
    --password "@keychain:notarize-password" \
    --wait

# Staple
xcrun stapler staple "MyPlugin.component"
```

## Common Mistakes

| Mistake | Fix |
|---|---|
| Wrong Bundle ID — AU validation fails silently | Match `BUNDLE_ID` in CMake to your signing cert; test with `auval -a` |
| Forgetting notarization — macOS Gatekeeper blocks plugin | Always notarize for distribution; test on clean macOS install |
| CI secrets not set — signing fails in GitHub Actions | Store certs as base64 in repo secrets; decode in CI step |
| Hardcoded paths in CMake | Use `${CMAKE_CURRENT_SOURCE_DIR}` and generator expressions |
| Not testing all formats | Build and test AU, VST3, and AAX separately — they can fail independently |
| Missing Linux dependencies | Install `libasound2-dev libfreetype-dev libx11-dev libxrandr-dev libxcursor-dev` first |

---

See **[REFERENCE.md](REFERENCE.md)** for full details: complete GitHub Actions CI/CD workflow with code signing secrets, per-platform build instructions, AAX/PACE signing, Windows code signing with signtool, Linux packaging (.deb), reproducible builds, conditional compilation, version management, troubleshooting guide, and pre-release platform checklist.
