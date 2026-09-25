# Architecture

The Qt Widgets presentation layer depends on modelled package and source data,
never on scattered package-manager process calls. `AlpmBackend` owns the
read-only libalpm handle. `Metadata` combines desktop entries with AppStream and
discovers Flathub components. `SourcePolicy` classifies trust and priority;
`SourceCatalog` validates the signed allowlist, deduplicates provider variants
and selects the recommended source.

`PackageTransaction` is the single transaction coordinator. Native package
changes go through the narrowly scoped privileged helper. Flatpak changes use
the signed-in user's Flatpak installation and can add only the official Flathub
remote before continuing an explicitly requested installation.

The Widgets shell deliberately mirrors the other Aero7 applications: Explorer
navigation and command rows, a Control Panel navigation pane, white content
pages, 9pt typography, blue task links, flat list headers and a details status
strip. Owned interface icons are embedded from the pinned AeroThemePlasma pack.
