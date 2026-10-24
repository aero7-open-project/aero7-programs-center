#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct TrustedInstallDeclaration {
    QString package;
    QString field;
    QString expression;
    QString namedPackage;
    bool namedPackageInstalled = false;
};

struct TrustedInstallPlan {
    QString target;
    QStringList newPackages;
    QStringList installedPackagesInPlan;
    QVector<TrustedInstallDeclaration> declarations;
    QString error;

    bool complete() const { return error.isEmpty(); }
    QString detailsText() const;
    static TrustedInstallPlan inspect(const QString &target,
        const QString &pacmanExecutable = QStringLiteral("/usr/bin/pacman"));
};
