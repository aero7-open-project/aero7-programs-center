# VM testing

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
