#include "AlpmBackend.h"
#include <QFileInfo>
#include <QSet>

AlpmBackend::AlpmBackend(QObject *parent):PackageBackend(parent) {
    alpm_errno_t err;
    m_handle=alpm_initialize("/", "/var/lib/pacman", &err);
    if (!m_handle) { m_error=QString::fromLocal8Bit(alpm_strerror(err)); return; }
    const QStringList repos={"core","extra","multilib","aero7"};
    for (const QString &repo:repos) {
        const QByteArray n=repo.toUtf8();
        alpm_register_syncdb(m_handle,n.constData(),ALPM_SIG_USE_DEFAULT);
    }
}
AlpmBackend::~AlpmBackend(){ if(m_handle) alpm_release(m_handle); }

Package AlpmBackend::fromPackage(alpm_pkg_t *p, alpm_db_t *db) const {
    Package x;
    x.name=QString::fromUtf8(alpm_pkg_get_name(p));
    x.displayName=x.name;
    x.summary=QString::fromUtf8(alpm_pkg_get_desc(p));
    x.description=x.summary;
    x.version=QString::fromUtf8(alpm_pkg_get_version(p));
    x.repository=db ? QString::fromUtf8(alpm_db_get_name(db)) : QStringLiteral("local");
    x.homepage=QString::fromUtf8(alpm_pkg_get_url(p));
    x.downloadSize=alpm_pkg_download_size(p);
    x.installedSize=alpm_pkg_get_isize(p);
    x.critical=isCriticalPackage(x.name);
    x.application=false;
    return x;
}

QList<Package> AlpmBackend::packages() {
    QList<Package> out; if(!m_handle) return out;
    QSet<QString> installed;
    alpm_db_t *local=alpm_get_localdb(m_handle);
    for(alpm_list_t *i=alpm_db_get_pkgcache(local);i;i=i->next) installed.insert(QString::fromUtf8(alpm_pkg_get_name(static_cast<alpm_pkg_t*>(i->data))));
    for(alpm_list_t *d=alpm_get_syncdbs(m_handle);d;d=d->next) {
        auto *db=static_cast<alpm_db_t*>(d->data);
        for(alpm_list_t *i=alpm_db_get_pkgcache(db);i;i=i->next) {
            auto x=fromPackage(static_cast<alpm_pkg_t*>(i->data),db);
            if(installed.contains(x.name)){ x.state=PackageState::Installed; auto *lp=alpm_db_get_pkg(local,x.name.toUtf8().constData()); if(lp)x.installedVersion=QString::fromUtf8(alpm_pkg_get_version(lp)); }
            out.push_back(x);
        }
    }
    return out;
}

QList<Package> AlpmBackend::updates(){
    QList<Package> out; if(!m_handle)return out; alpm_db_t *local=alpm_get_localdb(m_handle);
    for(alpm_list_t *i=alpm_db_get_pkgcache(local);i;i=i->next){
        auto *lp=static_cast<alpm_pkg_t*>(i->data); auto *sp=alpm_sync_get_new_version(lp,alpm_get_syncdbs(m_handle)); if(!sp)continue;
        Package x=fromPackage(sp,alpm_pkg_get_db(sp)); x.installedVersion=QString::fromUtf8(alpm_pkg_get_version(lp)); x.state=PackageState::UpdateAvailable; out.push_back(x);
    } return out;
}
bool AlpmBackend::databaseLocked() const { return QFileInfo::exists("/var/lib/pacman/db.lck"); }
