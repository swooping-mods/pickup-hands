# Pickup Hands: behavior and implementation

Describes the retained payload **0.1.1**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.1.1** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.1.1** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Automatically switches to internal empty hands when carrying native coffee/capsules or a registered physical toy. No visible hand slot or new key is added. The temporary toolbelt selection is index -1 with a null prototype; normal presentation refresh hides equipped tools. The provider API admits one resident owner claim and blocks competing tool selection while hands are occupied. Native coffee pickup/sip/drop and capsule carry remain game-owned.

## State and lifecycle

A rooted snapshot restores the previous selection/prototype after final release. All six assigned slots, unlocked capacity and saved loadout remain unchanged; no saved-selection setter is used. World teardown releases provider claims/restoration roots. Physical consumers claim only after successful pickup admission and release on actual drop/throw.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/pickup-hands/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Provides `pickup-hands.hands` | 1 | 32 bytes |

## Implementation sources

- [mod.c](../source/mods/pickup-hands/mod.c)
- [claims.h](../source/mods/pickup-hands/claims.h)
- [selection.h](../source/mods/pickup-hands/selection.h)

## Verification boundary

This is selection/presentation arbitration, not new carry physics or a general inventory system. Simultaneous native/provider claims, blocked drop and restored equipment visuals need matching game checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
