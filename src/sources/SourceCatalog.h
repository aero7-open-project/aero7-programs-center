#pragma once

#include "models/Package.h"

#include <QHash>
#include <QSet>

class SourceCatalog {
public:
    bool load(const QString &path = QStringLiteral("/usr/share/aero7-programs-center/catalog/sources.json"));
    bool loadBytes(const QByteArray &data, QString *error = nullptr);
    QString lastError() const { return m_error; }

    QString productId(SoftwareSource source, const QString &packageId,
                      const QString &fallback = {}) const;
    int priority(SoftwareSource source, const QString &packageId) const;
    bool cachyPackageAllowed(const QString &packageId) const;
    bool systemReplacementApproved(SoftwareSource source, const QString &packageId) const;
    QList<Package> groupProducts(const QList<Package> &packages) const;

private:
    struct VariantRule {
        QString productId;
        int priority = 0;
        bool approvedSystemReplacement = false;
    };

    static QString key(SoftwareSource source, const QString &packageId);
    QHash<QString, VariantRule> m_rules;
    QSet<QString> m_cachyAllowlist;
    QString m_error;
};
