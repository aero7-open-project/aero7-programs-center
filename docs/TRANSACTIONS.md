# Transactions

The GUI validates repository package identifiers, obtains explicit confirmation, and starts one fixed polkit helper. The helper validates again and directly execs pacman with `--noconfirm`, because the graphical confirmation has already been collected and no terminal input is available. It never invokes a shell, never deletes `db.lck`, never stores credentials, and rejects partial-update operations. `update-all` maps to a noninteractive `pacman --sync --refresh --sysupgrade` after graphical confirmation.
