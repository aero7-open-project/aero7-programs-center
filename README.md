# Aero7 Programs Center

> **TESTING / ALPHA** — Programs Center is developed independently and is not yet part of the Aero7 ISO or signed Aero7 package repository.

Programs Center is a Windows 7-inspired graphical software manager for Aero7 Linux. It provides an approachable Qt 6 interface for discovering applications, searching locally indexed package metadata, viewing package details, managing installed applications, and performing complete Arch Linux updates.

## Safety model

The GUI always runs as the signed-in user. Read-only package discovery uses libalpm. A narrow polkit-authorized helper accepts only four validated operations and executes `/usr/bin/pacman` directly with argument arrays—never through a shell. Pacman's configured signature policy is left untouched, its database lock is respected, and `Update All` uses `pacman -Syu`. Partial upgrades are deliberately rejected.

Supported package sources are the configured official Arch repositories and the signed `[aero7]` repository. AUR and Flatpak are future provider work.

## Build and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/aero7-programs-center
```

Install transactions require installing the helper and policy through the local PKGBUILD inside a disposable VM. Never install this alpha package on a production workstation.

## Relationship to Control Panel

Control Panel remains Aero7's administrative settings interface and keeps its existing Programs and Features and Linux Update pages. Programs Center is an additional user-friendly view over the same pacman databases, repositories, lock, and signature policy. Quick Tasks deep-link back to `control` and `control --setting updates`.

Known limitations: AppStream enrichment is currently supplemented by desktop-entry metadata, per-package update selection is rejected to preserve Arch's full-upgrade rule, transaction progress is phase/log based, and no AUR or Flatpak provider is enabled.
