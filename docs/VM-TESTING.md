# VM testing

## 2026-09-25 catalog and trusted-install run

- Build: local `testing` commit `57298d8`, packaged as
  `aero7-programs-center-git 1:0.2.0.r16.g57298d8-1`. The package build's
  five CTest cases passed. The same package and `archlinux-appstream-data`
  installed in a disposable QEMU overlay of the Beta 2 online acceptance VM;
  the saved base disk was not modified.
- Discovery: the installed guest app showed 1,227 available programs with
  AppStream names and artwork ([catalog screenshot](qa/2026-09-25/catalog-home.png)).
  On the current Aero7 host, the packaged app and catalog showed 1,234
  programs ([home](qa/2026-09-25/host-catalog-home.png)); search returned
  available GIMP and related packages ([search](qa/2026-09-25/host-search.png)).
- Password-free opt-in: from a process launched by the VM's local desktop,
  the checkbox requested administrator authentication
  ([prompt](qa/2026-09-25/admin-opt-in.png)) and then stayed checked
  ([setting](qa/2026-09-25/permission-enabled.png)). `pkcheck` authorized
  that local GUI process but denied the same user's SSH process.
- Real install: with the option enabled, the app installed `extra/cowsay`
  after the app's own confirmation and without another password prompt
  ([success](qa/2026-09-25/passwordless-install-success.png)). `pacman -Q`
  reported `cowsay 3.8.4-1`, and the package lock was clear. Disabling the
  checkbox required administrator authentication again; afterward `pkcheck`
  denied all open GUI instances.
- Guest setup caveat: SSH was absent in the base image. It was installed only
  in the disposable overlay for package transfer, and the overlay's firewall
  was temporarily stopped for that transport. The guest's cached mirror had
  two stale dependency URLs; copying the exact signed cache artifacts from
  the host allowed the test package to install. Neither workaround is a
  production configuration change.
- Scope: this validates discovery, packaging, opt-in/revocation, and one
  trusted install. It is not an update-all, removal, offline-download,
  authentication-cancellation, or physical-install acceptance pass.

## 2026-08-11 run

- VM: QEMU `Aero7-Installer-Test`, Arch Linux, Plasma 6.7.4 Wayland, kernel 7.1.7, 4 GiB RAM / 4 vCPU.
- Safety checkpoint: `work/qemu/snapshots/programs-center-pretest-20260811`, containing consistent paused copies of the qcow2 disk and writable UEFI state. `qemu-img check` reported no errors.
- Repository security: `[aero7]` retained `SigLevel = Required DatabaseRequired`.
- Commit built initially: `5ca698c`; fixes discovered during the run were rebuilt through `ca26a37`.
- CMake/Ninja VM build: passed.
- CTest in VM: 1/1 passed.
- Offscreen launch smoke test: passed.
- Local PKGBUILD: built and its check phase passed.
- Local package installation: passed; binary, desktop file, icon sizes, helper and polkit policy were present.
- Plasma Wayland launch: passed.
- Deep links: `--package`, `--installed`, `--updates`, and search exercised successfully.
- Package discovery: official `extra/firefox` and Aero7-sourced installed packages detected.
- Update comparison: Programs Center showed zero; `pacman -Qu` returned zero.
- Real transaction package: dynamically selected `extra/cowsay 3.8.4-1` (19.4 KiB download).
- GUI/polkit install: passed; `pacman -Q cowsay` confirmed installation and `db.lck` was clear.
- GUI/polkit removal: passed; `pacman -Q cowsay` returned not found and `db.lck` was clear.

## Defects found and fixed during the run

1. Ordinary Arch desktop files lacked `X-Pacman-Package`, leaving Installed Programs empty. Fixed by matching standard desktop basenames against real repository packages (`9be36c4`).
2. Pacman waited for terminal confirmation after GUI confirmation. Fixed by using `--noconfirm` only inside the validated, polkit-authorized helper (`398bc4a`).
3. The long-lived libalpm handle cached pre-transaction installed state. Fixed by reopening ALPM before catalog/update reads (`ca26a37`).

## Still outstanding

The full acceptance matrix is not complete. Update Selected and Update All controls are not yet exposed by the current Updates page, no pending-update system was available for a real upgrade test, authentication cancellation and offline-network tests remain, Control Panel launcher integration has not yet been installed in this VM, and fractional-scaling/alternate-size screenshots remain. The VM should remain disposable until those checks pass.
