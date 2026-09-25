#include "AlpmBackend.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QSet>

AlpmBackend::AlpmBackend(QObject *parent, const QString &root,
                         const QString &databasePath, const QStringList &repositories)
    :PackageBackend(parent),m_root(root),m_databasePath(databasePath),
      m_repositories(repositories) {
    reopen();
}
bool AlpmBackend::reopen() {
    if(m_handle){alpm_release(m_handle);m_handle=nullptr;}
    m_error.clear();
    m_repositoryWarning.clear();
    alpm_errno_t err;
    m_handle=alpm_initialize(m_root.toUtf8().constData(), m_databasePath.toUtf8().constData(), &err);
    if (!m_handle) { m_error=QString::fromLocal8Bit(alpm_strerror(err)); return false; }
    QStringList repos=m_repositories;
    if(repos.isEmpty()) {
        QProcess query;
        query.start(QStringLiteral("pacman-conf"), {QStringLiteral("--repo-list")});
        if(query.waitForFinished(5000) && query.exitStatus()==QProcess::NormalExit
           && query.exitCode()==0) {
            for(const auto &line:QString::fromUtf8(query.readAllStandardOutput()).split('\n'))
                if(!line.trimmed().isEmpty()) repos.append(line.trimmed());
        } else {
            query.kill(); query.waitForFinished(1000);
        }
    }
    m_configuredRepositories = repos;
    QStringList unavailable;
    for (const QString &repo:repos) {
        const QByteArray n=repo.toUtf8();
        auto *db=alpm_register_syncdb(m_handle,n.constData(),ALPM_SIG_USE_DEFAULT);
        const QFileInfo cache(QDir(m_databasePath).filePath("sync/"+repo+".db"));
        if(!db || !cache.isFile() || !cache.isReadable() || alpm_db_get_valid(db)!=0)
            unavailable.append(repo);
    }
    if(repos.isEmpty())
        m_repositoryWarning=QStringLiteral("Repository configuration is unavailable. Installed programs are still shown.");
    else if(!unavailable.isEmpty())
        m_repositoryWarning=QStringLiteral("Repository information is missing or unreadable (%1). Installed programs are still shown; connect to the internet and perform a full system update to refresh it.").arg(unavailable.join(QStringLiteral(", ")));
    return true;
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
    x.source=SourcePolicy::fromRepository(x.repository);
    x.sourcePriority=SourcePolicy::descriptor(x.source).priority;
    x.provider=PackageProvider::Pacman;
    x.homepage=QString::fromUtf8(alpm_pkg_get_url(p));
    x.downloadSize=alpm_pkg_download_size(p);
    x.installedSize=alpm_pkg_get_isize(p);
    x.critical=isCriticalPackage(x.name);
    x.application=false;
    auto blockProtectedDependency = [&x](alpm_list_t *entries, const QString &verb) {
     for(alpm_list_t *entry=entries;entry;entry=entry->next) {
        const auto *dependency=static_cast<alpm_depend_t *>(entry->data);
        const QString replaced=dependency&&dependency->name
            ? QString::fromUtf8(dependency->name) : QString();
        if(isCriticalPackage(replaced)) {
            x.blockedReason=QStringLiteral("Installing this package would %1 protected system package %2.").arg(verb,replaced);
            break;
        }
     }
    };
    blockProtectedDependency(alpm_pkg_get_replaces(p),QStringLiteral("replace"));
    if(x.blockedReason.isEmpty())blockProtectedDependency(alpm_pkg_get_conflicts(p),QStringLiteral("conflict with"));
    const auto *files=alpm_pkg_get_files(p);
    if(files) for(size_t i=0;i<files->count;++i) {
        const QString path=QString::fromUtf8(files->files[i].name);
        if(path.startsWith(QStringLiteral("usr/share/applications/")) && path.endsWith(QStringLiteral(".desktop")))
            x.desktopIds.append(QFileInfo(path).completeBaseName());
    }
    return x;
}

QList<Package> AlpmBackend::packages() {
    QList<Package> out; if(!reopen()) return out;
    QSet<QString> installed;
    QSet<QString> seen;
    alpm_db_t *local=alpm_get_localdb(m_handle);
    for(alpm_list_t *i=alpm_db_get_pkgcache(local);i;i=i->next) installed.insert(QString::fromUtf8(alpm_pkg_get_name(static_cast<alpm_pkg_t*>(i->data))));
    for(alpm_list_t *d=alpm_get_syncdbs(m_handle);d;d=d->next) {
        auto *db=static_cast<alpm_db_t*>(d->data);
        for(alpm_list_t *i=alpm_db_get_pkgcache(db);i;i=i->next) {
            auto x=fromPackage(static_cast<alpm_pkg_t*>(i->data),db);
            if(installed.contains(x.name)){ x.state=PackageState::Installed; auto *lp=alpm_db_get_pkg(local,x.name.toUtf8().constData()); if(lp){x.installedVersion=QString::fromUtf8(alpm_pkg_get_version(lp));x.desktopIds=fromPackage(lp,local).desktopIds;x.installedSize=alpm_pkg_get_isize(lp);} }
            seen.insert(x.name);
            out.push_back(x);
        }
    }
    // The local database is authoritative even without any downloaded sync DBs,
    // and includes foreign/local packages absent from configured repositories.
    for(alpm_list_t *i=alpm_db_get_pkgcache(local);i;i=i->next) {
        auto x=fromPackage(static_cast<alpm_pkg_t*>(i->data),local);
        if(seen.contains(x.name)) continue;
        x.state=PackageState::Installed;
        x.installedVersion=x.version;
        x.downloadSize=0;
        out.push_back(x);
    }
    return out;
}

QList<Package> AlpmBackend::updates(){
    QList<Package> out; if(!reopen())return out; alpm_db_t *local=alpm_get_localdb(m_handle);
    for(alpm_list_t *i=alpm_db_get_pkgcache(local);i;i=i->next){
        auto *lp=static_cast<alpm_pkg_t*>(i->data); auto *sp=alpm_sync_get_new_version(lp,alpm_get_syncdbs(m_handle)); if(!sp)continue;
        Package x=fromPackage(sp,alpm_pkg_get_db(sp)); x.installedVersion=QString::fromUtf8(alpm_pkg_get_version(lp)); x.state=PackageState::UpdateAvailable; out.push_back(x);
    } return out;
}
bool AlpmBackend::databaseLocked() const { return QFileInfo::exists(QDir(m_databasePath).filePath("db.lck")); }
