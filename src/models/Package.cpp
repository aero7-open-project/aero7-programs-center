#include "Package.h"

QString Package::searchableText() const {
    return (displayName + ' ' + name + ' ' + summary + ' ' + description + ' ' + category).toCaseFolded();
}

QString humanSize(qint64 bytes) {
    if (bytes < 0) return QStringLiteral("Unknown");
    constexpr double k = 1024.0;
    if (bytes < k) return QString::number(bytes) + " B";
    if (bytes < k*k) return QString::number(bytes/k, 'f', 1) + " KiB";
    if (bytes < k*k*k) return QString::number(bytes/(k*k), 'f', 1) + " MiB";
    return QString::number(bytes/(k*k*k), 'f', 2) + " GiB";
}

bool isCriticalPackage(const QString &n) {
    const QString x = n.toLower();
    return x == "base" || x == "pacman" || x == "glibc" || x == "systemd" ||
           x == "filesystem" || x == "linux" || x.startsWith("linux-") ||
           x == "archlinux-keyring" || x.startsWith("aero7-");
}

QString friendlyRepository(const QString &r) {
    return r.compare("aero7", Qt::CaseInsensitive) == 0 ? QStringLiteral("Aero7") : r;
}
