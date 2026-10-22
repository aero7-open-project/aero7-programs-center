# Real Aero7 VM screenshots

Captured on 2026-08-11 from the disposable Aero7 QEMU VM at 1024×768 under Plasma 6.7.4 Wayland:

- `programs-center-home-vm.png` — Home layout and real package metadata.
- `programs-center-search-vm.png` — local offline-capable search for `cowsay`.
- `programs-center-details-vm.png` — Firefox package details.
- `programs-center-installed-vm.png` — installed application detection after the desktop-file metadata fix.
- `programs-center-updates-vm.png` — zero-update state, agreeing with `pacman -Qu`.
- `programs-center-auth-vm.png` — Aero7 UAC-style polkit authentication.
- `programs-center-install-result-vm.png` — successful GUI installation result.
- `programs-center-remove-result-vm.png` — successful GUI removal result.

These are test evidence, not concept images. They also show that the 1024×768 layout needs further horizontal-density refinement to match the supplied mockup more closely.

## Aero7 host review — 2026-10-22

The current local `testing` build was launched directly on the user's Aero7
Wayland desktop. These active-window captures show the actual running UI:

- [`qa/2026-10-22/home-host.png`](qa/2026-10-22/home-host.png) — Home with the
  single command bar below the breadcrumb/search row and no right task pane.
- [`qa/2026-10-22/settings-host.png`](qa/2026-10-22/settings-host.png) — separate
  Settings window with the approved embedded source logos, official checkmarks
  and three optional built-in sources off.

No repository was added to pacman or Flatpak during this visual check.
