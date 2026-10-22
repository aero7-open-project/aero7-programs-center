# Asset provenance

| Asset | Source | Licence/provenance | Notes |
| --- | --- | --- | --- |
| `resources/icons/aero7-programs-center-source.png` | Artwork supplied directly by the Aero7 project owner on 2026-08-11 | Project-owner-supplied Programs Center artwork | Unmodified source image requested as the application icon. |
| `resources/icons/aero7-programs-center.png` | Background-isolated derivative of the supplied source | Same provenance as source | Subject preserved; external background isolated for desktop-icon use. |
| Rendered PNG icon sizes | Generated locally from the isolated PNG master by `scripts/render-app-icons.sh` | Same provenance as source | 16, 22, 24, 32, 48, 64, 128, 256 and 512 px. The artwork is trimmed and scaled to fill the hicolor tile like the other Aero7 app icons. |
| `resources/icons/pack/**` | `aerothemeplasma-icons`, commit `96950b8028a5d960cb683280fe5f1d9e33e6b8a2` | Upstream repository declares AGPL-3.0-or-later; see bundled `UPSTREAM-LICENSE` and `UPSTREAM-README.md` | Unmodified selected sizes from the icon pack already shipped by Aero7. |
| `resources/icons/pack/{16,24,32,48,256}/system-software-update.png` | Approved Aero7 replacements in `aero7-repo/assets/icon-branding/overrides/apps/` | Project-owner-approved handoff, 2026-10-22 | Embedded Windows emblems replaced with the approved plain Aero7 mark; resource paths and native sizes preserved. |
| `resources/icons/sources/aero7.png` | Approved `aero7-repo/assets/icon-branding/aero7-logo-plain.png` | Aero7 project artwork, SHA-256 `5632d386115028d42b07eaca5c345ed03203ebe61dcd7bd222f1a644ee93d1c6` | Exact approved plain mark. |
| `resources/icons/sources/arch.png` | Arch Linux logo supplied by the Aero7 project owner on 2026-10-22 | Arch Linux trademark policy applies | The supplied 600×600 PNG had its pure-white background made transparent; compositing on white is pixel-identical to the supplied file. ImageGen was tried but changed the outline, so its output was rejected. Identifies Arch packages only and implies no endorsement. |
| `resources/icons/sources/chaotic-aur.png` | Colorful orbital PNG supplied by the Aero7 project owner on 2026-10-22 | Project mark; referential use only | Byte-for-byte copy, SHA-256 `7fe86eb8abf04a6531a87bb1cb69514da1f10c0ada3ec0d854e71ea8c0397d2`. Used only for Chaotic-AUR entries. |
| `resources/icons/sources/cachyos.png` | CachyOS PNG supplied by the Aero7 project owner on 2026-10-22 | Project mark; referential use only | Byte-for-byte copy, SHA-256 `6804952cba7f2b0b980ce3cddd323cfa74e96252d340b549ef5cb175605842bc`. Used only for CachyOS Applications entries. |
| `resources/icons/sources/flathub.png` | [Flathub repository logo](https://dl.flathub.org/repo/logo.svg) | Project mark; referential use only | Scaled PNG; used only for Flathub entries. |

Custom external sources use the already embedded neutral warning icon, never a
brand logo or an official checkmark. Source marks identify their respective
projects; they do not mean those projects endorse Aero7.

Third-party application artwork is intentionally resolved from each installed
application's icon identity. When that artwork is unavailable, Programs Center
uses an embedded package icon from the pinned AeroThemePlasma pack. Every
Programs Center-owned UI icon is embedded at build time and is independent of
the selected Plasma icon theme.
