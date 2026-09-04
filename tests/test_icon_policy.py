#!/usr/bin/env python3
"""Reject theme-dependent icons in Programs Center-owned interface code."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ALLOWED = {
    "src/ui/MainWindow.cpp": ("QIcon::fromTheme(p.iconName)",),
    "src/ui/ProgramCard.cpp": ("QIcon::fromTheme(p.iconName)",),
}


def main() -> int:
    errors: list[str] = []
    for path in (ROOT / "src").rglob("*"):
        if path.suffix not in {".cpp", ".h", ".qml", ".ui"}:
            continue
        relative = path.relative_to(ROOT).as_posix()
        allowed = ALLOWED.get(relative, ())
        for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if "QIcon::fromTheme" not in line:
                continue
            if not any(fragment in line for fragment in allowed):
                errors.append(f"{relative}:{number}: unapproved theme icon: {line.strip()}")
    for desktop in (ROOT / "packaging").glob("*.desktop"):
        for number, line in enumerate(desktop.read_text(encoding="utf-8").splitlines(), 1):
            if line.startswith("Icon=") and not line.startswith("Icon=aero7-"):
                errors.append(f"{desktop.relative_to(ROOT)}:{number}: launcher icon is not namespaced")
    if errors:
        raise SystemExit("\n".join(errors))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
