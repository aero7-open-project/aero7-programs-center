#pragma once
#include "models/Package.h"
#include <QObject>

class PackageBackend : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual QList<Package> packages() = 0;
    virtual QList<Package> updates() = 0;
    virtual bool databaseLocked() const = 0;
signals:
    void refreshed();
    void error(const QString &friendly, const QString &details);
};
