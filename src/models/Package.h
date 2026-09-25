#pragma once
#include "sources/SourcePolicy.h"
#include <QString>
#include <QStringList>

enum class PackageState { Available, Installed, UpdateAvailable };

struct PackageVariant {
    QString packageId;
    QString version;
    QString installedVersion;
    QString repository;
    SoftwareSource source = SoftwareSource::Unknown;
    PackageProvider provider = PackageProvider::Pacman;
    PackageState state = PackageState::Available;
    int priority = 0;
    bool critical = false;
    bool approvedSystemReplacement = false;
    QString blockedReason;
};

struct Package {
    QString name, displayName, summary, description, version, installedVersion;
    QString repository, homepage, license, developer, iconName, category;
    QString productId;
    QStringList desktopIds, keywords;
    QList<PackageVariant> variants;
    qint64 downloadSize = 0, installedSize = 0;
    PackageState state = PackageState::Available;
    SoftwareSource source = SoftwareSource::Unknown;
    PackageProvider provider = PackageProvider::Pacman;
    int sourcePriority = 0;
    bool application = false, critical = false;
    bool approvedSystemReplacement = false;
    QString blockedReason;
    QString searchableText() const;
};

QString humanSize(qint64 bytes);
bool isCriticalPackage(const QString &name);
QString friendlyRepository(const QString &repository);
PackageVariant packageVariant(const Package &package);
