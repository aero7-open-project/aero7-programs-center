# Transactions

The GUI validates repository package identifiers and starts one fixed polkit helper. The helper validates again and directly execs pacman. It never invokes a shell, never deletes `db.lck`, never stores credentials, and rejects partial-update operations. `update-all` maps to `pacman --sync --refresh --sysupgrade`.
