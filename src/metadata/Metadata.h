#pragma once
#include "models/Package.h"
#include <QHash>

class Metadata {
public:
    void load();
    void enrich(Package &package) const;
    static QString mapCategory(const QStringList &categories);
    static QStringList loadFeatured(const QString &path, QString *error=nullptr);
private:
    struct Entry{QString name,summary,description,icon,category;};
    QHash<QString,Entry> m_entries;
};
