# Aero7 Programs Center

Programs Center is the optional graphical software manager for Aero7. It
provides a Windows 7-inspired interface for discovering applications, searching
locally indexed package metadata, viewing package details, managing installed
applications, and performing complete Arch Linux updates.

> [!IMPORTANT]
> Programs Center remains Beta software. It is included in the Aero7 Beta 2
> package set as an optional feature and is not installed on a fresh desktop.
> Enable or remove it through **Control Panel > Programs and Features > Turn
> Aero7 features on or off**. Beta 2 retains a checksum-verified copy of the
> package locally, so this feature can be enabled again without internet.

## Features

- Browse, search, and inspect software from configured pacman repositories
- Install and remove applications through a narrowly scoped privileged helper
- Refresh repositories and perform complete Arch Linux system upgrades
- Show installed software, available updates, package source, size, and status
- Use Aero7-owned application and action icons regardless of the active theme
- Open the matching Control Panel pages from Quick Tasks

## Safety model

The GUI always runs as the signed-in user. Read-only package discovery uses libalpm. A narrow polkit-authorized helper accepts only four validated operations and executes `/usr/bin/pacman` directly with argument arrays—never through a shell. Pacman's configured signature policy is left untouched, its database lock is respected, and `Update All` uses `pacman -Syu`. Partial upgrades are deliberately rejected.

Supported package sources are the configured official Arch repositories and the
signed `[aero7]` repository. AUR and Flatpak are not enabled.

## Installation and updates

On Aero7 Beta 2, use **Turn Aero7 features on or off**. This keeps the feature
state, package verification, authentication, and retained-data explanation in
one place. Once installed, Programs Center receives normal updates from the
signed Aero7 package repository.

The package name is `aero7-programs-center-git`. It can also be managed with
pacman by experienced administrators, but the Control Panel route is the
supported desktop workflow.

## Relationship to Control Panel

Control Panel remains Aero7's administrative settings interface and keeps its existing Programs and Features and Linux Update pages. Programs Center is an additional user-friendly view over the same pacman databases, repositories, lock, and signature policy. Quick Tasks deep-link back to `control` and `control --setting updates`.

## Known limitations

AppStream enrichment is supplemented by desktop-entry metadata. Per-package
update selection is rejected to preserve Arch's full-upgrade rule. Transaction
progress is phase/log based, and no AUR or Flatpak provider is enabled.
