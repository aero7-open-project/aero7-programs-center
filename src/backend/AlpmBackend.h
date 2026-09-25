#pragma once
#include "PackageBackend.h"
#include <alpm.h>

class AlpmBackend final : public PackageBackend {
    Q_OBJECT
public:
    explicit AlpmBackend(QObject *parent=nullptr,
                         const QString &root=QStringLiteral("/"),
                         const QString &databasePath=QStringLiteral("/var/lib/pacman/"),
                         const QStringList &repositories={});
    ~AlpmBackend() override;
    QList<Package> packages() override;
    QList<Package> updates() override;
    bool databaseLocked() const override;
    QString lastError() const { return m_error; }
    QString repositoryWarning() const { return m_repositoryWarning; }
    QStringList repositories() const { return m_configuredRepositories; }
private:
    alpm_handle_t *m_handle=nullptr;
    QString m_error;
    QString m_root, m_databasePath, m_repositoryWarning;
    QStringList m_repositories, m_configuredRepositories;
    bool reopen();
    Package fromPackage(alpm_pkg_t *pkg, alpm_db_t *repo) const;
};
