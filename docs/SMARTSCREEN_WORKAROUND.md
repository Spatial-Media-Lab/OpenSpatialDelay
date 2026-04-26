# Windows SmartScreen Workaround

> **Until OpenSpatialDelay's Windows VST3 is signed via the SignPath OSS Program (application in flight as of v1.0.0), Windows will show a "Windows protected your PC" warning the first time you run the installer or unzip the plugin.** The plugin is safe; SmartScreen is just being cautious about software it hasn't seen many copies of yet.

## What you'll see

When you double-click the downloaded ZIP or try to drag the `.vst3` into your VST3 plug-ins folder, Windows may show a blue dialog:

> Windows protected your PC
> Microsoft Defender SmartScreen prevented an unrecognized app from starting.
> Running this app might put your PC at risk.

## How to bypass it once

1. Click **More info** on the SmartScreen dialog.
2. A new button appears at the bottom: **Run anyway** (or **Extract anyway** for the ZIP).
3. Click it. Windows remembers your choice for that file.

That's it. The plugin will install normally and run in your DAW.

## Why this happens

Microsoft's SmartScreen learns to trust an executable based on:
1. Whether it's signed with an **Extended Validation (EV)** code-signing certificate ($300–700/yr from a CA), and
2. How many other Windows users have downloaded and run the same file without flagging it.

OpenSpatialDelay is a community-funded GPL-3.0 project — paying for an EV cert isn't sustainable. Instead, we're applying for **SignPath's OSS Program**, which signs open-source Windows binaries for free. Once approved (typically 1–4 weeks), v1.0.x patch releases will be signed and you'll never see this warning again.

If you'd rather wait for the signed build, sign up for the mailing list at <https://andrewrahman.com/get-osd> and you'll be notified when the signed build ships.

## Verifying the download is genuine

If you'd like to confirm the ZIP you downloaded matches what we shipped, the SHA-256 hash is published on the GitHub release page at:

<https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0>

Compute the local hash with:

```powershell
Get-FileHash .\OpenSpatialDelay-v1.0.0-Windows-x64.zip -Algorithm SHA256
```

…and compare to the published value. If they match, you have the unmodified release.
