#include "Metadata.h"
#include <AppStreamQt/bundle.h>
#include <AppStreamQt/component.h>
#include <AppStreamQt/developer.h>
#include <AppStreamQt/icon.h>
#include <AppStreamQt/launchable.h>
#include <AppStreamQt/pool.h>
#include <QDirIterator>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

namespace {
QString flatpakApplicationId(QString bundleId)
{
 const QStringList parts=bundleId.split('/',Qt::SkipEmptyParts);
 if(parts.size()>=2&&parts.first()==QStringLiteral("app")) return parts.at(1);
 return bundleId;
}

QString appStreamIcon(const AppStream::Component &component)
{
 auto icon=component.icon(QSize(64,64));
 if(icon.isEmpty()) icon=component.iconStock();
 for(const QString &candidate:{icon.filename(),icon.name()}) {
  if(candidate.isEmpty())continue;
  if(QFileInfo(candidate).isAbsolute() && QFileInfo::exists(candidate))return candidate;
  if(icon.kind()==AppStream::Icon::KindCached && !component.origin().isEmpty()) {
   for(const QString &root:QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
    for(const int size:{64,128,48}) {
     const QString path=QDir(root).filePath(QStringLiteral("swcatalog/icons/%1/%2x%2/%3")
                                           .arg(component.origin()).arg(size).arg(candidate));
     if(QFileInfo::exists(path))return path;
    }
  }
 }
 if(!icon.filename().isEmpty()) return icon.filename();
 if(!icon.name().isEmpty()) return icon.name();
 return {};
}

QHash<QString,QPair<QString,QString>> installedFlatpaks()
{
 QHash<QString,QPair<QString,QString>> installed;
 if(QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty()) return installed;
 QProcess process;
 process.start(QStringLiteral("flatpak"),{QStringLiteral("list"),QStringLiteral("--app"),
               QStringLiteral("--columns=application,version,origin")});
 if(!process.waitForFinished(10000)||process.exitStatus()!=QProcess::NormalExit
    ||process.exitCode()!=0) return installed;
 for(const auto &line:QString::fromUtf8(process.readAllStandardOutput()).split('\n',Qt::SkipEmptyParts)) {
  const auto columns=line.split('\t');
  if(!columns.isEmpty()) installed.insert(columns.at(0),
      {columns.value(1),columns.value(2)});
 }
 return installed;
}
}

QString Metadata::mapCategory(const QStringList &c){
 const QString x=c.join(' ').toLower();
 if(x.contains("network")||x.contains("webbrowser"))return "Internet";
 if(x.contains("audio")||x.contains("video"))return "Multimedia";
 if(x.contains("graphics")||x.contains("photography"))return "Graphics & Photos";
 if(x.contains("office"))return "Office"; if(x.contains("game"))return "Games";
 if(x.contains("education")||x.contains("science"))return "Education";
 if(x.contains("development"))return "Developer Tools"; if(x.contains("settings")||x.contains("system"))return "System Tools";
 return "Utilities";
}
void Metadata::load(const QStringList &roots,bool includeAppStream){
 m_entries.clear();
 m_flatpaks.clear();
 m_appStreamError.clear();
 m_nativeCatalogAvailable=false;
 for(const QString &root:roots){
  QDirIterator it(root,{"*.desktop"},QDir::Files); while(it.hasNext()){
   QSettings f(it.next(),QSettings::IniFormat); f.beginGroup("Desktop Entry"); if(f.value("NoDisplay").toBool()||f.value("Hidden").toBool())continue;
   QStringList pkgs=f.value("X-Pacman-Package").toStringList();
   // Arch desktop files rarely carry X-Pacman-Package. The unqualified
   // desktop-file basename is a useful, truthful fallback when it matches a
   // real repository package later during enrich() (firefox.desktop ->
   // firefox, org.kde.krita.desktop remains only a harmless unmatched key).
   const QString desktopId=QFileInfo(f.fileName()).completeBaseName();
   if(!desktopId.isEmpty()&&!pkgs.contains(desktopId))pkgs<<desktopId;
   Entry e;
   e.name=f.value("Name").toString();e.summary=f.value("Comment").toString();
   e.description=e.summary;e.icon=f.value("Icon").toString();
   e.category=mapCategory(f.value("Categories").toString().split(';',Qt::SkipEmptyParts));
   e.productId=desktopId;e.keywords=f.value("Keywords").toString().split(';',Qt::SkipEmptyParts);
   for(const auto&p:pkgs)m_entries.insert(p,e);
  }
 }
 if(!includeAppStream)return;

 for(const QString &root:QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
  const QDir catalog(QDir(root).filePath(QStringLiteral("swcatalog/xml")));
  if(catalog.exists(QStringLiteral("extra.xml.gz")) || catalog.exists(QStringLiteral("extra.xml"))
     || catalog.exists(QStringLiteral("core.xml.gz")) || catalog.exists(QStringLiteral("core.xml"))) {
   m_nativeCatalogAvailable=true;
   break;
  }
 }

 AppStream::Pool pool;
 pool.setLocale(QStringLiteral("en_US"));
 for(const QString &root:QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
  if(QDir(root).canonicalPath()==QStringLiteral("/usr/share"))continue;
  const QDir catalog(QDir(root).filePath(QStringLiteral("swcatalog/xml")));
  if(catalog.exists(QStringLiteral("extra.xml.gz")) || catalog.exists(QStringLiteral("extra.xml")))
   pool.addExtraDataLocation(catalog.absolutePath(),AppStream::Metadata::FormatStyleCatalog);
 }
 AppStream::Pool::Flags flags(AppStream::Pool::FlagLoadOsCatalog);
 flags.setFlag(AppStream::Pool::FlagLoadOsMetainfo);
 flags.setFlag(AppStream::Pool::FlagLoadOsDesktopFiles);
 flags.setFlag(AppStream::Pool::FlagLoadFlatpak);
 pool.setFlags(flags);
 if(!pool.load()) {m_appStreamError=pool.lastError();return;}
 const auto installed=installedFlatpaks();
 QSet<QString> seenFlatpaks;
 for(const auto &component:pool.components()) {
  if(component.kind()!=AppStream::Component::KindDesktopApp)continue;
  Entry entry;
  entry.name=component.name();entry.summary=component.summary();
  entry.description=component.description();entry.icon=appStreamIcon(component);
  entry.category=mapCategory(component.categories());entry.productId=component.id();
  entry.homepage=component.url(AppStream::Component::UrlKindHomepage).toString();
  entry.license=component.projectLicense();entry.developer=component.developer().name();
  entry.keywords=component.keywords();
  for(const auto &packageName:component.packageNames())
   if(!packageName.isEmpty())m_entries.insert(packageName,entry);
  for(const auto &launchable:component.launchables())
   if(launchable.kind()==AppStream::Launchable::KindDesktopId)
    for(QString desktopId:launchable.entries()) {
     desktopId.remove(QRegularExpression(QStringLiteral("\\.desktop$")));
     if(!desktopId.isEmpty())m_entries.insert(desktopId,entry);
    }

  const auto bundle=component.bundle(AppStream::Bundle::KindFlatpak);
  if(bundle.isEmpty())continue;
  const QString applicationId=flatpakApplicationId(bundle.id());
  if(applicationId.isEmpty()||seenFlatpaks.contains(applicationId))continue;
  const QString origin=component.origin().toLower();
  const auto installedEntry=installed.constFind(applicationId);
  const bool isInstalled=installedEntry!=installed.cend();
  if(!origin.contains(QStringLiteral("flathub"))
     &&(!isInstalled||!installedEntry->second.contains(QStringLiteral("flathub"),Qt::CaseInsensitive)))continue;
  Package package;
  package.name=applicationId;package.productId=component.id();
  package.displayName=entry.name;package.summary=entry.summary;package.description=entry.description;
  package.iconName=entry.icon;package.category=entry.category;package.homepage=entry.homepage;
  package.license=entry.license;package.developer=entry.developer;package.keywords=entry.keywords;
  package.repository=QStringLiteral("flathub");package.source=SoftwareSource::Flathub;
  package.sourcePriority=SourcePolicy::descriptor(package.source).priority;
  package.provider=PackageProvider::Flatpak;package.application=true;
  package.state=isInstalled?PackageState::Installed:PackageState::Available;
  if(isInstalled){package.installedVersion=installedEntry->first;package.version=installedEntry->first;}
  seenFlatpaks.insert(applicationId);m_flatpaks.append(package);
 }
}
void Metadata::enrich(Package &p)const{
 auto i=m_entries.constFind(p.name);
 if(i==m_entries.cend()) for(const auto &id:p.desktopIds) {
  i=m_entries.constFind(id);
  if(i!=m_entries.cend()) break;
 }
 if(i==m_entries.cend())return;
 p.displayName=i->name;p.summary=i->summary;p.description=i->description;
 p.iconName=i->icon;p.category=i->category;p.productId=i->productId;
 p.homepage=i->homepage;p.license=i->license;p.developer=i->developer;
 p.keywords=i->keywords;p.application=true;
}
QStringList Metadata::loadFeatured(const QString &path,QString *error){
 QFile f(path);if(!f.open(QIODevice::ReadOnly)){if(error)*error="Catalog is unavailable.";return{};}QJsonParseError e;auto d=QJsonDocument::fromJson(f.readAll(),&e);if(e.error!=QJsonParseError::NoError||!d.isArray()){if(error)*error="Catalog is invalid.";return{};}QStringList out;static const QRegularExpression valid("^[a-zA-Z0-9@._+:-]+$");for(auto v:d.array()){auto n=v.toObject().value("package").toString();if(valid.match(n).hasMatch()&&!out.contains(n))out<<n;}return out;
}
