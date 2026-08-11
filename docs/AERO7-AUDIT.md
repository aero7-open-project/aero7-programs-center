# Aero7 project audit (2026-08-11)

The `memegeko` account currently has six directly relevant repositories: `aero7` (`beta`, Qt/Python installer and ISO), `aero7-shell` (`beta`, shell-driven Plasma integration), `aero7-repo` (`beta`, signed package repository plus header-only Aero7Qt), `aero7-control-panel-` (`main`, Qt 6 Widgets), `aero7-computer-management` (`main`, C++/Qt 6), and `aerothemeplasma` (`Plasma/6.7`, QML/C++/theme assets). No newer Programs Center repository existed before this work.

Reusable findings:

- Aero7Qt supplies glass frame properties, navigation buttons, shared stylesheet loading and event integration without imposing a page architecture.
- Control Panel's `Win7Ui.h` and `IconHelper.h` establish 9pt typography, `#1F4E99` links, white detail pages, thin gray/blue rules, native Qt buttons, themed-icon fallback chains and asynchronous population.
- `LinuxUpdatePage` uses direct argument arrays, never `sh -c`, performs `pacman -Syu`, exposes the pacman lock as a user-facing busy state and persists timestamps in `QSettings`.
- `ProgramsFeaturesPage` maps installed packages to `.desktop` and AppStream metadata, hides installer/system foundations by default, parses pacman history for useful install dates and protects package-manager/kernel foundations.
- Computer Management uses C++20/Qt 6, backend/UI separation, modelled navigation history, CTest and local PKGBUILD/desktop integration.
- The repository structure is a signed pacman repository. Programs Center must not change its `SigLevel`, package list or ISO manifest during alpha development.
- Stable Control Panel deep links are `control --setting <catalog-key>`; `updates` and `default-apps` are current keys.
- Aero7 and Aero7-shell document disposable Plasma Wayland VM testing, but this workstation currently exposes no libvirt guests and no SSH test host.

Asset decision: runtime software/category icons come from the active system icon theme with Aero7-style fallback names. The only copied branding asset is `aero7-shell/docs/assets/aero7-logo.png`, whose provenance explicitly permits modification/reuse when preserved. AeroThemePlasma screenshots and Microsoft-derived resources are not copied.
