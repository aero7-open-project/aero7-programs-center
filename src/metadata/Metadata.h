#pragma once
#include "models/Package.h"
#include <QHash>

class Metadata {
public:
    void load(const QStringList &roots={QStringLiteral("/usr/share/applications"),
                                       QStringLiteral("/usr/local/share/applications")},
              bool includeAppStream=true);
    void enrich(Package &package) const;
    QList<Package> flatpakPackages() const { return m_flatpaks; }
    QString appStreamError() const { return m_appStreamError; }
    bool nativeCatalogAvailable() const { return m_nativeCatalogAvailable; }
    static QString mapCategory(const QStringList &categories);
    static QStringList loadFeatured(const QString &path, QString *error=nullptr);
private:
    struct Entry {
        QString name, summary, description, icon, category, categoryId, productId;
        QString homepage, license, developer;
        QStringList keywords;
    };
    QHash<QString,Entry> m_entries;
    QList<Package> m_flatpaks;
    QString m_appStreamError;
    bool m_nativeCatalogAvailable = false;
};
