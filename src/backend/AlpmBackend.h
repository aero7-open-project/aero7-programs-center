#pragma once
#include "PackageBackend.h"
#include <alpm.h>

class AlpmBackend final : public PackageBackend {
    Q_OBJECT
public:
    explicit AlpmBackend(QObject *parent=nullptr);
    ~AlpmBackend() override;
    QList<Package> packages() override;
    QList<Package> updates() override;
    bool databaseLocked() const override;
    QString lastError() const { return m_error; }
private:
    alpm_handle_t *m_handle=nullptr;
    QString m_error;
    Package fromPackage(alpm_pkg_t *pkg, alpm_db_t *repo) const;
};
