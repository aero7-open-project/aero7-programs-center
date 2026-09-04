#pragma once

#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QSize>
#include <QString>

// Aero7 application chrome is sourced from the pinned Windows 7 Aero icon
// pack and embedded at build time. The active desktop theme is never queried.
namespace Aero7Icons {

inline QString canonicalName(QString name)
{
    name = name.toLower();
    name.replace(QLatin1Char('_'), QLatin1Char('-'));
    if (name.contains(QStringLiteral("back"))) return QStringLiteral("go-previous");
    if (name.contains(QStringLiteral("forward"))) return QStringLiteral("go-next");
    if (name.contains(QStringLiteral("home"))) return QStringLiteral("go-home");
    if (name.contains(QStringLiteral("search")) || name.contains(QStringLiteral("discover"))) return QStringLiteral("edit-find");
    if (name.contains(QStringLiteral("uninstall")) || name.contains(QStringLiteral("remove"))) return QStringLiteral("edit-delete");
    if (name.contains(QStringLiteral("install"))) return QStringLiteral("system-software-install");
    if (name.contains(QStringLiteral("update"))) return QStringLiteral("system-software-update");
    if (name.contains(QStringLiteral("package"))) return QStringLiteral("package-x-generic");
    if (name.contains(QStringLiteral("internet"))) return QStringLiteral("applications-internet");
    if (name.contains(QStringLiteral("multimedia")) || name.contains(QStringLiteral("audio"))) return QStringLiteral("gnome-multimedia");
    if (name.contains(QStringLiteral("graphic")) || name.contains(QStringLiteral("photo"))) return QStringLiteral("applications-graphics");
    if (name.contains(QStringLiteral("office")) || name.contains(QStringLiteral("document"))) return QStringLiteral("applications-office");
    if (name.contains(QStringLiteral("game"))) return QStringLiteral("applications-games");
    if (name.contains(QStringLiteral("education"))) return QStringLiteral("applications-science");
    if (name.contains(QStringLiteral("developer"))) return QStringLiteral("applications-development");
    if (name.contains(QStringLiteral("utilit"))) return QStringLiteral("applications-utilities");
    if (name.contains(QStringLiteral("success"))) return QStringLiteral("dialog-ok-apply");
    if (name.contains(QStringLiteral("warning")) || name.contains(QStringLiteral("pending"))) return QStringLiteral("dialog-warning");
    if (name.contains(QStringLiteral("error"))) return QStringLiteral("dialog-error");
    if (name.contains(QStringLiteral("information"))) return QStringLiteral("dialog-information");
    if (name.contains(QStringLiteral("cancel"))) return QStringLiteral("dialog-cancel");
    if (name.contains(QStringLiteral("setting"))) return QStringLiteral("preferences-system");
    if (name.contains(QStringLiteral("download"))) return QStringLiteral("go-download");
    return QStringLiteral("applications-system");
}

inline QIcon icon(const QString &name)
{
    const QString canonical = canonicalName(name);
    QIcon result;
    for (const int size : {16, 22, 24, 32, 36, 48, 64, 72, 96, 128, 192, 256}) {
        const QString path = QStringLiteral(":/aero7/pack/%1/%2.png").arg(size).arg(canonical);
        if (QFile::exists(path)) result.addFile(path, QSize(size, size));
    }
    if (!result.isNull()) return result;
    qWarning().noquote() << "[Aero7 Icons] Missing pack icon:" << canonical;
    const QString fallback = QStringLiteral(":/aero7/pack/48/applications-system.png");
    return QFile::exists(fallback) ? QIcon(fallback) : QIcon();
}

inline QIcon resource(const QString &path, const QString &fallback)
{
    const QIcon result(path);
    if (!result.isNull()) return result;
    qWarning().noquote() << "[Aero7 Icons] Missing resource:" << path;
    return icon(fallback);
}

} // namespace Aero7Icons
