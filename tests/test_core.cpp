#include "DeepLink.h"
#include "metadata/Metadata.h"
#include "metadata/BrowseCategories.h"
#include "models/Package.h"
#include "sources/SourceCatalog.h"
#include "sources/SourceRegistration.h"
#include <QFile>
#include <QSet>
#include <QTemporaryDir>
#include <QtTest>
class CoreTest:public QObject{Q_OBJECT
private slots:
 void sizes(){QCOMPARE(humanSize(1024),QString("1.0 KiB"));QCOMPARE(humanSize(1024*1024),QString("1.0 MiB"));}
 void critical(){QVERIFY(isCriticalPackage("pacman"));QVERIFY(isCriticalPackage("linux-zen"));QVERIFY(!isCriticalPackage("vlc"));}
 void sources(){QCOMPARE(friendlyRepository("aero7"),QString("Aero7"));QCOMPARE(friendlyRepository("extra"),QString("Arch Linux"));}
 void sourceClassification(){QCOMPARE(SourcePolicy::fromRepository("core-testing"),SoftwareSource::Arch);QCOMPARE(SourcePolicy::fromRepository("vendor-testing"),SoftwareSource::Unknown);QVERIFY(SourcePolicy::isThirdParty(SoftwareSource::Flathub));QVERIFY(!SourcePolicy::isThirdParty(SoftwareSource::Aero7));}
 void sourceRegistration(){QVERIFY(SourceRegistration::validName("myrepo"));QVERIFY(!SourceRegistration::validName("aero7"));QVERIFY(!SourceRegistration::validName("chaotic-aur-copy"));QVERIFY(SourceRegistration::validFlatpakRepo("myremote","https://example.org/my.flatpakrepo","[Flatpak Repo]\nUrl=https://example.org/repo/\nGPGKey=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\n"));QVERIFY(!SourceRegistration::validFlatpakRepo("arch","https://example.org/my.flatpakrepo","[Flatpak Repo]\nUrl=https://example.org/repo/\nGPGKey=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\n"));QVERIFY(!SourceRegistration::validFlatpakRepo("myremote","http://example.org/my.flatpakrepo","[Flatpak Repo]\nUrl=https://example.org/repo/\nGPGKey=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\n"));QVERIFY(!SourceRegistration::validFlatpakRepo("myremote","https://example.org/my.flatpakrepo","[Flatpak Repo]\nUrl=https://example.org/repo/\n"));}
 void categories(){
  QCOMPARE(Metadata::mapCategory({"Network","WebBrowser"}),QString("Internet"));
  QCOMPARE(Metadata::mapCategory({"AudioVideo"}),QString("Multimedia"));
  QCOMPARE(Metadata::mapCategory({"Network","Chat"}),QString("Communication"));
  QCOMPARE(Metadata::mapCategory({"AudioVideo","Audio","Midi"}),QString("Music & Audio"));
  QCOMPARE(Metadata::mapCategory({"AudioVideo","Video"}),QString("Video & Movies"));
  QCOMPARE(Metadata::mapCategory({"System","FileManager"}),QString("File Management"));
  QCOMPARE(Metadata::mapCategory({"System","Security"}),QString("Security & Privacy"));
  QCOMPARE(Metadata::mapCategory({"Education","Science","Math"}),QString("Science & Math"));
  QCOMPARE(Metadata::mapCategory({"System","Settings"}),QString("Settings"));
  QCOMPARE(Metadata::mapCategory({"Utility","UnregisteredTag"}),QString("Utilities"));
  QCOMPARE(BrowseCategories::find("communication")->label,QString("Communication"));
  QCOMPARE(BrowseCategories::find("Communication")->id,QString("communication"));
  QSet<QString> ids;
  for(const auto &category:BrowseCategories::all()){
   QVERIFY(!category.id.isEmpty());
   QVERIFY(!category.icon.isEmpty());
   QVERIFY(!ids.contains(category.id));
   if(!category.parentId.isEmpty()) QVERIFY(BrowseCategories::find(category.parentId));
   ids.insert(category.id);
  }
 }
 void search(){Package p;p.displayName="LibreOffice";p.name="libreoffice-fresh";p.summary="Office suite";QVERIFY(p.searchableText().contains("office"));}
 void deepLinks(){auto d=parseDeepLink({"app","--package","vlc"});QCOMPARE(d.page,DeepLink::PackageDetails);QCOMPARE(d.value,QString("vlc"));QCOMPARE(parseDeepLink({"app","--sources"}).page,DeepLink::Sources);}
 void catalog(){QTemporaryDir d;QFile f(d.filePath("x.json"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("[{\"package\":\"vlc\"},{\"package\":\"bad name\"},{\"package\":\"vlc\"}]");f.close();QString e;QCOMPARE(Metadata::loadFeatured(f.fileName(),&e),QStringList{"vlc"});}
 void badCatalog(){QTemporaryDir d;QFile f(d.filePath("x.json"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("nope");f.close();QString e;QVERIFY(Metadata::loadFeatured(f.fileName(),&e).isEmpty());QVERIFY(!e.isEmpty());}
 void sourceCatalogGroupsVariants(){SourceCatalog c;QString e;QVERIFY(c.loadBytes(R"({"schema":1,"approved_cachyos_packages":["approved-app"],"applications":[{"id":"org.test.App","variants":[{"source":"arch","package":"native-app"},{"source":"flathub","package":"org.test.App"}]}]})",&e));Package native;native.name="native-app";native.displayName="Test";native.source=SoftwareSource::Arch;Package flatpak;flatpak.name="org.test.App";flatpak.displayName="Test";flatpak.source=SoftwareSource::Flathub;flatpak.provider=PackageProvider::Flatpak;Package blocked;blocked.name="not-approved";blocked.source=SoftwareSource::CachyOs;const auto grouped=c.groupProducts({flatpak,native,blocked});QCOMPARE(grouped.size(),1);QCOMPARE(grouped.first().name,QString("native-app"));QCOMPARE(grouped.first().variants.size(),2);QCOMPARE(grouped.first().variants.first().source,SoftwareSource::Arch);}
};QTEST_MAIN(CoreTest)
#include "test_core.moc"
