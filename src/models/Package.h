#pragma once
#include <QString>
#include <QStringList>

enum class PackageState { Available, Installed, UpdateAvailable };

struct Package {
    QString name, displayName, summary, description, version, installedVersion;
    QString repository, homepage, license, developer, iconName, category;
    qint64 downloadSize = 0, installedSize = 0;
    PackageState state = PackageState::Available;
    bool application = false, critical = false;
    QString searchableText() const;
};

QString humanSize(qint64 bytes);
bool isCriticalPackage(const QString &name);
QString friendlyRepository(const QString &repository);
