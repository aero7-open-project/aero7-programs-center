"""Exercise source configuration edits without touching the host's /etc."""

from importlib.machinery import SourceFileLoader
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


helper = SourceFileLoader(
    "sources_helper",
    str(Path(__file__).resolve().parents[1] / "helper/aero7-programs-center-sources"),
).load_module()


class SourceHelperTest(unittest.TestCase):
    def test_signed_repository_add_and_remove(self):
        original = "[options]\nArchitecture = auto\n\n[core]\nInclude = /etc/pacman.d/mirrorlist\n"
        fingerprint = "A" * 40
        pacman, managed = helper.update_text(
            original, "", "add", "example", "https://example.org/$repo/os/$arch", fingerprint
        )
        self.assertEqual(pacman.count(helper.INCLUDE), 1)
        self.assertIn("SigLevel = PackageRequired DatabaseRequired TrustedOnly", managed)
        self.assertEqual(helper.managed_entries(managed)["example"][1], fingerprint)
        pacman_again, managed_again = helper.update_text(pacman, managed, "remove", "example")
        self.assertEqual(pacman_again.count(helper.INCLUDE), 1)
        self.assertEqual(managed_again, "")

    def test_rejects_spoofing_and_injection(self):
        fingerprint = "A" * 40
        for name, url in (
            ("arch", "https://example.org/$arch"),
            ("chaotic-aur-extra", "https://example.org/$arch"),
            ("demo", "http://example.org/$arch"),
            ("demo", "https://example.org/\nSigLevel = Never"),
            ("demo", "https://example.org/$evil"),
            ("demo", "https://user:pass@example.org/$arch"),
        ):
            with self.subTest(name=name, url=url), self.assertRaises(ValueError):
                helper.validate(name, url, fingerprint)

    def test_rejects_unmanaged_replacement(self):
        fingerprint = "A" * 40
        with self.assertRaises(ValueError):
            helper.update_text("[core]\n", "", "add", "core",
                               "https://example.org/$arch", fingerprint)
        with self.assertRaises(ValueError):
            helper.managed_entries("[demo]\nSigLevel = Never\nServer = https://example.org\n")

    def test_atomic_write_and_rollback_in_temporary_files(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            pacman, managed, backups = root / "pacman.conf", root / "managed.conf", root / "backups"
            original = "[options]\nArchitecture = auto\n\n[core]\n"
            pacman.write_text(original)
            fingerprint = "B" * 40
            backup = helper.apply_change(
                pacman, managed, backups, "add", "demo", "https://example.org/$arch",
                fingerprint, lambda action, name: True,
            )
            self.assertIn("[demo]", managed.read_text())
            self.assertEqual((backup / "pacman.conf").read_text(), original)
            before_pacman, before_managed = pacman.read_text(), managed.read_text()
            with self.assertRaises(RuntimeError):
                helper.apply_change(pacman, managed, backups, "remove", "demo", "", "",
                                    lambda action, name: False)
            self.assertEqual(pacman.read_text(), before_pacman)
            self.assertEqual(managed.read_text(), before_managed)

    def test_pacman_accepts_required_signature_policy(self):
        with TemporaryDirectory() as directory:
            config = Path(directory) / "pacman.conf"
            config.write_text("[options]\nArchitecture = auto\n\n" + helper.render_entries({
                "demo": ("https://example.org/$repo/os/$arch", "C" * 40)
            }))
            result = subprocess.run(["/usr/bin/pacman-conf", "--config", str(config), "--repo-list"],
                                    capture_output=True, text=True, timeout=5)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("demo", result.stdout.splitlines())


if __name__ == "__main__":
    unittest.main()
