# EVE Emulator binaries

The Windows emulator files in this directory are vendored from Bridgetek's
official EVE-Emulator release package:

- Repository: https://github.com/Bridgetek/EVE-Emulator
- x64 version: 5.3.35
- Upstream commit: `516f4d511b205f9cc620814d8e7387b975599d3c`
- Upstream commit date: 2026-10-20
- x64 public header/API: API 19
- Legacy x86 version: 4.8.2
- Legacy x86 header/API: API 12

The API 19 headers are selected only for x64 emulator builds. EveApps retains
its API 12 headers for legacy x86 builds. The x64 runtime set is:

| File | SHA-256 |
| --- | --- |
| `x64/bt8xxemu.dll` | `FAAA77891A3417A40263874397EB22BF33377DB53CD2EFE1C3F0A8039498FAAB` |
| `x64/bt8xxemu.lib` | `A45B29AD8E20525600BE2E98F367E73023A46060D3484D4CAB81E5EF95AFC4C4` |
| `x64/btflashemu.dll` | `CA5AD462A55DBD69176A678813AB0B32DDD5E10DAD85416879179E6508F7ED2D` |
| `x64/zlib.dll` | `999EB98A1916A098C6C88F726A12B1702FEA88BDAF04D3C0F281EC2D2630FF6D` |

Keep the x64 files together. `btflashemu.dll` replaces the older standalone
flash-emulator library, and `zlib.dll` is a runtime dependency. Do not copy the
x64 files into the `x86` directory. See `LICENSE.EVE-Emulator.md` for the
upstream redistribution terms.
