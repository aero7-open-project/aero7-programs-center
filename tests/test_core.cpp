#include "DeepLink.h"
#include "metadata/Metadata.h"
#include "models/Package.h"
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>
class CoreTest:public QObject{Q_OBJECT
private slots:
 void sizes(){QCOMPARE(humanSize(1024),QString("1.0 KiB"));QCOMPARE(humanSize(1024*1024),QString("1.0 MiB"));}
 void critical(){QVERIFY(isCriticalPackage("pacman"));QVERIFY(isCriticalPackage("linux-zen"));QVERIFY(!isCriticalPackage("vlc"));}
 void sources(){QCOMPARE(friendlyRepository("aero7"),QString("Aero7"));QCOMPARE(friendlyRepository("extra"),QString("extra"));}
 void categories(){QCOMPARE(Metadata::mapCategory({"Network","WebBrowser"}),QString("Internet"));QCOMPARE(Metadata::mapCategory({"AudioVideo"}),QString("Multimedia"));}
 void search(){Package p;p.displayName="LibreOffice";p.name="libreoffice-fresh";p.summary="Office suite";QVERIFY(p.searchableText().contains("office"));}
 void deepLinks(){auto d=parseDeepLink({"app","--package","vlc"});QCOMPARE(d.page,DeepLink::PackageDetails);QCOMPARE(d.value,QString("vlc"));}
 void catalog(){QTemporaryDir d;QFile f(d.filePath("x.json"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("[{\"package\":\"vlc\"},{\"package\":\"bad name\"},{\"package\":\"vlc\"}]");f.close();QString e;QCOMPARE(Metadata::loadFeatured(f.fileName(),&e),QStringList{"vlc"});}
 void badCatalog(){QTemporaryDir d;QFile f(d.filePath("x.json"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("nope");f.close();QString e;QVERIFY(Metadata::loadFeatured(f.fileName(),&e).isEmpty());QVERIFY(!e.isEmpty());}
};QTEST_MAIN(CoreTest)
#include "test_core.moc"
