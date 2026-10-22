# Aero7 Programs Center

Programs Center is Aero7's optional graphical software manager. It combines a
Windows 7-style Control Panel and Explorer interface with the native Arch Linux
package system and a guarded multi-source application catalog.

> [!IMPORTANT]
> Programs Center is Beta software. On Aero7 Beta 2 it is an optional feature
> under **Control Panel > Programs and Features > Turn Aero7 features on or
> off**. A checksum-verified package is retained by the offline image so the
> feature can be enabled again without downloading it.

## What it provides

- Windows 7-style navigation, a single command bar below the breadcrumb/search
  row, list views and dialogs
- Application discovery from Aero7, official Arch, Chaotic-AUR, the reviewed
  CachyOS Applications catalog and Flathub
- One product entry when the same application exists in more than one source
- Clear source labels, recommended-source ordering and advanced alternatives
- Installed-program management and System, Application and Third-party update
  groups
- A separate Settings window with source management, install permissions, browsing options
  and a reset for dismissed safety notices
- AppStream names, descriptions, publishers, licenses and real application
  artwork when the catalog supplies it
- A searchable Arch application catalog; if catalog data is missing, Programs
  Center explains the problem and offers an authenticated repair action
- An optional per-account setting for password-free installs from trusted
  Aero7 and official Arch repositories, enabled only after administrator approval
- Aero7-owned interface icons embedded from the pinned AeroThemePlasma pack, so
  the app chrome does not change with the desktop icon theme

## Software sources and trust

Programs Center separates where software comes from from whether the package is
considered part of the trusted system base.

| Source | Default trust | Behavior |
| --- | --- | --- |
| Aero7 | Trusted | Official Aero7 integration packages and updates; highest recommendation priority |
| Arch Linux | Trusted | Official `core`, `extra`, `multilib` and their official testing repositories |
| Aero7 approved native | Reviewed third-party | Native packages explicitly approved in the signed Aero7 catalog |
| Chaotic-AUR | Third-party, off by default | Precompiled community packages; Programs Center never invokes `yay` or `paru` |
| CachyOS Applications | Third-party, off by default | Only application IDs in Aero7's maintained allowlist are exposed; a full CachyOS repository is never enabled |
| Flathub | Third-party, off by default | Flatpak applications; the per-user Flathub remote is added only after the user opts in and requests an install |
| Custom native/Flatpak | External, off until added | User-added signed sources are always labelled External and use a generic icon |

Aero7 and Arch Linux cannot be disabled in Programs Center. Their status shows
whether the repository is actually configured. Optional sources remain off
until the user accepts an enablement warning. Installed applications remain
visible and manageable even when their source is hidden. The first install from
each third-party source shows a separate warning; the setting to dismiss that
warning can be reset under **Settings > Safety notices**. Details always retain
the source warning and label.

**Settings > Software sources > Add source** supports two paths:

- **Native pacman repository:** enter a unique name, HTTPS `Server` URL, and
  the full fingerprint of an already fully trusted key in pacman's keyring.
  Review the source and authenticate as an administrator. The dedicated helper
  adds a small managed include file to `/etc/pacman.conf`, requires package
  **and database** signatures, checks the key, validates the resulting pacman
  configuration, and saves a root-owned backup with rollback on failure. It
  never imports, signs, or trusts a new key. Pacman can accept any key already
  trusted by its system keyring; it cannot pin one signer per repository. A
  full system refresh is needed before new package listings appear. The app
  can remove repositories that this helper added without removing installed
  packages.
- **Flatpak remote:** enter a unique name and publisher HTTPS `.flatpakrepo`
  URL. The app checks for a GPG key, displays its fingerprint for review, then
  adds the remote to the user's Flatpak installation. Its catalog entries are
  shown with a generic external-source icon and can be hidden or removed from
  Settings. Flatpak's signature checks remain enabled.

Neither path grants a custom source an Aero7 or Arch logo/checkmark. A source
name alone never makes it official. Adding a new signing key to the system
keyring remains an administrator task outside Programs Center.

The signed `sources.json` catalog maps package variants to one product ID,
defines source priority and carries the CachyOS allowlist. Updating the normal
Programs Center package therefore updates the allowlist without granting a
remote service permission to rewrite pacman configuration.

## Safety model

The GUI runs as the signed-in user. Native package discovery uses libalpm. A
narrow polkit helper accepts only validated package operations and executes
`pacman` directly with argument arrays, never through a shell. Pacman's lock,
signature policy and full-upgrade rule remain authoritative.

Programs Center blocks native candidates that declare a replacement or conflict
against protected foundation packages. An exception can only be made by an
explicit reviewed entry in the signed Aero7 catalog. Direct AUR builds are not
supported.

Flatpak operations run in the current user's Flatpak installation. After a
Flathub opt-in, Programs Center may add only the official Flathub remote for a
requested install. Custom Flatpak installs target their reviewed remote.

The **Settings** page can enable password-free trusted application
installs for the current account. Enabling or disabling it requires an
administrator password. Its separate polkit action is available only in an
active local session, and its helper accepts one package from Aero7, `core`,
`extra`, or `multilib` after checking the entire cached dependency plan.
Conflicting/replacing packages, packages from other repositories, removal, and
system upgrades do not use this permission. The setting stores a root-owned
per-user polkit rule, not a password; disabling it removes the rule.

## Installation and updates

The supported desktop route is **Turn Aero7 features on or off**. Once enabled,
the `aero7-programs-center-git` package receives normal updates from the signed
Aero7 package repository. Experienced administrators can manage that package
with pacman, but the Control Panel feature flow remains the recommended route.

## Relationship to Control Panel

Control Panel remains Aero7's administrative settings interface. Programs
Center is a focused application catalog over the same package databases and
signature rules. Its visual structure deliberately reuses the established
Aero7 Control Panel and File Explorer language rather than introducing a
separate modern store design.

## Current limitations

- The full Arch application catalog requires `archlinux-appstream-data`, which
  is a package dependency. Existing installations missing it can use the
  **Install the application catalog** action on the Home page. Without it,
  featured apps and package-name search remain available.
- Update installation remains a complete Arch upgrade; partial system upgrades
  are deliberately not offered.
- Chaotic-AUR and CachyOS entries are shown only when the corresponding package
  metadata is available to libalpm. Their built-in switches do not silently
  configure pacman repositories.
- Native Add source supports HTTPS repositories with a signing key that is
  already fully trusted and a signed repository database. Repositories with
  unsigned databases or new/untrusted keys require separate administrator
  trust setup; the app does not weaken signature policy to accommodate them.
