#include "backend/AlpmBackend.h"
#include "metadata/Metadata.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>

namespace {
bool writeFile(const QString &path, const QByteArray &data) {
    if(!QDir().mkpath(QFileInfo(path).absolutePath())) return false;
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data)==data.size();
}
const QByteArray description=
    "%NAME%\nsample-app\n\n%VERSION%\n1.0-1\n\n%DESC%\nSample application\n\n"
    "%ARCH%\nx86_64\n\n%SIZE%\n4096\n\n";
bool initializeLocal(const QString &db) {
    return writeFile(db+"/local/ALPM_DB_VERSION", "9\n")
        && writeFile(db+"/local/sample-app-1.0-1/desc", description)
        && writeFile(db+"/local/sample-app-1.0-1/files",
                     "%FILES%\nusr/share/applications/org.aero7.Sample.desktop\n\n");
}
}

class OfflineBackendTest:public QObject {
    Q_OBJECT
private slots:
    void installedWithoutSyncDatabases() {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString db=root.path()+"/var/lib/pacman";
        QVERIFY(initializeLocal(db));
        AlpmBackend backend(nullptr, root.path(), db, {"test-repo"});
        const auto packages=backend.packages();
        QCOMPARE(packages.size(),1);
        const auto &package=packages.first();
        QCOMPARE(package.name,QString("sample-app"));
        QCOMPARE(package.state,PackageState::Installed);
        QCOMPARE(package.installedVersion,QString("1.0-1"));
        QCOMPARE(package.repository,QString("local"));
        QCOMPARE(package.downloadSize,0);
        QCOMPARE(package.desktopIds,QStringList{"org.aero7.Sample"});
        QVERIFY(backend.lastError().isEmpty());
        QVERIFY(backend.repositoryWarning().contains("test-repo"));
        QVERIFY(backend.updates().isEmpty());
        QVERIFY(!backend.repositoryWarning().isEmpty());
        QCOMPARE(backend.packages().size(),1); // Repeated refresh must retain local data.
    }

    void syncAndLocalDoNotDuplicateInstalledPackage() {
        QTemporaryDir root;
        const QString db=root.path()+"/var/lib/pacman";
        QVERIFY(initializeLocal(db));
        const QString staging=root.path()+"/repo";
        QVERIFY(writeFile(staging+"/sample-app-1.0-1/desc",description+
                          "%FILENAME%\nsample-app-1.0-1-x86_64.pkg.tar.zst\n\n"));
        QVERIFY(QDir().mkpath(db+"/sync"));
        QCOMPARE(QProcess::execute("bsdtar",{"-cf",db+"/sync/test-repo.db",
                 "-C",staging,"sample-app-1.0-1"}),0);
        AlpmBackend backend(nullptr,root.path(),db,{"test-repo"});
        const auto packages=backend.packages();
        QCOMPARE(packages.size(),1);
        QCOMPARE(packages.first().repository,QString("test-repo"));
        QCOMPARE(packages.first().state,PackageState::Installed);
        QCOMPARE(packages.first().desktopIds,QStringList{"org.aero7.Sample"});
        QVERIFY2(backend.repositoryWarning().isEmpty(),qPrintable(backend.repositoryWarning()));
    }

    void namespacedDesktopUsesPackageOwnership() {
        QTemporaryDir root;
        const QString desktop=root.filePath("org.aero7.Sample.desktop");
        QVERIFY(writeFile(desktop,"[Desktop Entry]\nType=Application\nName=Sample App\n"
                                  "Comment=An installed application\nIcon=sample\nCategories=Office;\n"));
        Metadata metadata;
        metadata.load({root.path()},false);
        Package package;
        package.name="sample-app";
        package.desktopIds={"org.aero7.Sample"};
        metadata.enrich(package);
        QVERIFY(package.application);
        QCOMPARE(package.displayName,QString("Sample App"));
        QCOMPARE(package.category,QString("Office"));
        QVERIFY(QFile::remove(desktop));
        metadata.load({root.path()},false);
        Package removed;
        removed.name=package.name;
        removed.desktopIds=package.desktopIds;
        metadata.enrich(removed);
        QVERIFY(!removed.application);
    }
};
QTEST_GUILESS_MAIN(OfflineBackendTest)
#include "test_offline_backend.moc"
