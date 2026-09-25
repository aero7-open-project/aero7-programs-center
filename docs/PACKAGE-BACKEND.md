# Package backend

libalpm reads the local and configured sync databases. Repository records remain
authoritative for native package names, versions, sizes and sources. Desktop and
AppStream metadata enrich application records; missing metadata does not hide a
native package from an explicit search. `[aero7]` is displayed as “Aero7”; no
signature option is overridden.

The source policy assigns Aero7, Arch, approved native, Chaotic-AUR, allowlisted
CachyOS Applications and Flathub a deterministic priority. The signed source
catalog maps variants to product IDs, so one application is shown once while
the details dialog retains source alternatives. Unknown repositories are
third-party rather than being mistaken for an official Arch testing repository.

Flatpak discovery comes from the local AppStream pool and is restricted to
Flathub components. Direct AUR build helpers are intentionally unsupported.
