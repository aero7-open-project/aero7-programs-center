# Transactions

The GUI validates package identifiers, shows source and trust information,
obtains explicit confirmation, and then selects the provider transaction.

Native packages start one fixed polkit helper. The helper validates again and
directly execs pacman with `--noconfirm`, because graphical confirmation has
already been collected and no terminal input is available. It never invokes a
shell, never deletes `db.lck`, never stores credentials, and rejects partial
updates. `update-all` maps to a noninteractive `pacman --sync --refresh
--sysupgrade` after graphical confirmation.

Flatpak applications are installed or removed for the current user. When the
user has no Flathub remote, the coordinator runs `flatpak remote-add --user
--if-not-exists` for the official Flathub repository and only then continues
the requested install. Both commands use fixed argument arrays.
