# Transactions

The GUI validates package identifiers, shows source and trust information,
obtains explicit confirmation, and then selects the provider transaction.

Native packages start one fixed polkit helper. The helper validates again and
directly execs pacman with `--noconfirm`, because graphical confirmation has
already been collected and no terminal input is available. It never invokes a
shell, never deletes `db.lck`, never stores credentials, and rejects partial
updates. `update-all` maps to a noninteractive `pacman --sync --refresh
--sysupgrade` after graphical confirmation.

Password-free installs are a separate, opt-in polkit action. An administrator
must authenticate once to create the current account's root-owned rule. The
rule applies only while that account has an active local session. The dedicated
trusted installer accepts one explicitly qualified Aero7/Arch package, checks
the complete cached dependency plan stays in those trusted repositories, and
rejects any planned package that declares a conflict or replacement. It never
removes packages or updates the whole system. Disabling the option removes the
rule; normal package management still requires authentication. This is not a
general `sudo` exemption and does not store credentials.

Flatpak applications are installed or removed for the current user. When the
user has no Flathub remote, the coordinator runs `flatpak remote-add --user
--if-not-exists` for the official Flathub repository and only then continues
the requested install. Both commands use fixed argument arrays.
