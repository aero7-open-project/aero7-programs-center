# Asset provenance

| Asset | Source | Licence/provenance | Notes |
| --- | --- | --- | --- |
| `resources/icons/aero7-programs-center-source.png` | Artwork supplied directly by the Aero7 project owner on 2026-08-11 | Project-owner-supplied Programs Center artwork | Unmodified source image requested as the application icon. |
| `resources/icons/aero7-programs-center.png` | Background-isolated derivative of the supplied source | Same provenance as source | Subject preserved; external background isolated for desktop-icon use. |
| Rendered PNG icon sizes | Generated locally from the isolated PNG master by `scripts/render-app-icons.sh` | Same provenance as source | 16, 22, 24, 32, 48, 64, 128, 256 and 512 px. The artwork is trimmed and scaled to fill the hicolor tile like the other Aero7 app icons. |
| `resources/icons/pack/**` | `aerothemeplasma-icons`, commit `96950b8028a5d960cb683280fe5f1d9e33e6b8a2` | Upstream repository declares AGPL-3.0-or-later; see bundled `UPSTREAM-LICENSE` and `UPSTREAM-README.md` | Unmodified selected sizes from the icon pack already shipped by Aero7. |

Third-party application artwork is intentionally resolved from each installed
application's icon identity. When that artwork is unavailable, Programs Center
uses an embedded package icon from the pinned AeroThemePlasma pack. Every
Programs Center-owned UI icon is embedded at build time and is independent of
the selected Plasma icon theme.
