# Spatial Media Library — Plugin Proposals

13 plugin proposals for the Spatial Media Library suite, all built on [SpatialCore](../SpatialCore/).

## Prioritized Order

| Priority | Plugin | Complexity | Type | Status |
|----------|--------|-----------|------|--------|
| **Tier 0** | [SpatialCore extraction](../SpatialCore/) | High | Framework | Pre-requisite |
| **Tier 1** | [OpenSpatialPanner](03-OpenSpatialPanner.md) | Low-Med | Effect | Validates extraction |
| **Tier 2** | [OpenSpatialChorus](04-OpenSpatialChorus.md) | Medium | Effect | Quick win |
| **Tier 2** | [OpenSpatialTremolo](11-OpenSpatialTremolo.md) | Low-Med | Effect | Quick win |
| **Tier 3** | [OpenSpatialReverb](01-OpenSpatialReverb.md) | High | Effect | Flagship |
| **Tier 3** | [OpenSpatialDistortion](05-OpenSpatialDistortion.md) | Medium | Effect | Novel |
| **Tier 4** | [OpenSpatialGranular](02-OpenSpatialGranular.md) | High | Effect | Ambitious |
| **Tier 4** | [OpenSpatialSynthesizer](13-OpenSpatialSynthesizer.md) | Very High | Instrument | Flagship |
| — | [OpenSpatialFilter](06-OpenSpatialFilter.md) | Medium | Effect | Novel |
| — | [OpenSpatialHarmonizer](07-OpenSpatialHarmonizer.md) | High | Effect | Novel |
| — | [OpenSpatialLooper](08-OpenSpatialLooper.md) | Medium | Effect | Novel |
| — | [OpenSpatialRingMod](09-OpenSpatialRingMod.md) | Medium | Effect | Novel |
| — | [OpenSpatialVocoder](10-OpenSpatialVocoder.md) | High | Effect | Novel |
| — | [OpenSpatialSampler](12-OpenSpatialSampler.md) | Medium | Instrument | Novel |

## What Each Proposal Contains

Each proposal brief includes:
- One-line concept
- What SpatialCore provides (reused framework)
- What's plugin-specific (new DSP to build)
- Market reference (what exists today)
- Complexity estimate
- Whether it's already in the project specification

## How to Start a New Plugin

1. Pick a proposal from above
2. Use the template from `spatial-media-skills/templates/spatialcore-effect/` or `spatialcore-instrument/`
3. Follow the [SpatialCore Integration Guide](../SpatialCore/docs/integration-guide.md)
4. See the [Workflow Tutorials](../SpatialCore/docs/workflow-tutorials.md) for multi-plugin development patterns
