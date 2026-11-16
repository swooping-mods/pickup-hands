# Pickup Hands: behavior and implementation

Describes the exported implementation **0.1.2**. The source hashes and packaged module identify this version; verification remains scoped to the checks below.

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

Refresh the shared adapter and held-motion closure used by Squishy Plushie 0.2.1. Preserve the hands API and qualify actual pickup, held turning/walking and throw release in that closure. Native Windows gameplay, other mod combinations, long sessions and real co-op remain separate.

This is selection/presentation arbitration, not new carry physics or a general inventory system. Simultaneous native/provider claims, blocked drop and restored equipment visuals need matching game checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
