#pragma once

#include "Aero7Icon.h"
#include "models/Package.h"

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QStandardPaths>

inline QIcon packageArtwork(const Package &package)
{
    if (!package.iconName.isEmpty()) {
        const QFileInfo direct(package.iconName);
        if (direct.isAbsolute() && direct.isReadable()) {
            const QIcon icon(direct.filePath());
            if (!icon.isNull()) return icon;
        }
        const QString base = QFileInfo(package.iconName).completeBaseName();
        const auto dataRoots = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &root : dataRoots) {
            for (const int size : {256, 128, 64, 48, 32, 24, 22, 16}) {
                for (const QString &extension : {QStringLiteral("png"), QStringLiteral("svg")}) {
                    const QString path = QDir(root).filePath(
                        QStringLiteral("icons/hicolor/%1x%1/apps/%2.%3").arg(size).arg(base).arg(extension));
                    if (QFileInfo::exists(path)) return QIcon(path);
                }
            }
            const QString pixmap = QDir(root).filePath(QStringLiteral("pixmaps/%1.png").arg(base));
            if (QFileInfo::exists(pixmap)) return QIcon(pixmap);
        }
    }
    return Aero7Icons::icon(QStringLiteral("package"));
}
