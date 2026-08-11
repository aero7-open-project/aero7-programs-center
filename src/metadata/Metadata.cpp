#include "Metadata.h"
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

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
void Metadata::load(){
 for(const QString &root:{"/usr/share/applications","/usr/local/share/applications"}){
  QDirIterator it(root,{"*.desktop"},QDir::Files); while(it.hasNext()){
   QSettings f(it.next(),QSettings::IniFormat); f.beginGroup("Desktop Entry"); if(f.value("NoDisplay").toBool()||f.value("Hidden").toBool())continue;
   const QStringList pkgs=f.value("X-Pacman-Package").toStringList(); if(pkgs.isEmpty())continue;
   Entry e{f.value("Name").toString(),f.value("Comment").toString(),f.value("Comment").toString(),f.value("Icon").toString(),mapCategory(f.value("Categories").toString().split(';',Qt::SkipEmptyParts))}; for(const auto&p:pkgs)m_entries.insert(p,e);
  }
 }
}
void Metadata::enrich(Package &p)const{auto i=m_entries.constFind(p.name);if(i==m_entries.cend())return;p.displayName=i->name;p.summary=i->summary;p.description=i->description;p.iconName=i->icon;p.category=i->category;p.application=true;}
QStringList Metadata::loadFeatured(const QString &path,QString *error){
 QFile f(path);if(!f.open(QIODevice::ReadOnly)){if(error)*error="Catalog is unavailable.";return{};}QJsonParseError e;auto d=QJsonDocument::fromJson(f.readAll(),&e);if(e.error!=QJsonParseError::NoError||!d.isArray()){if(error)*error="Catalog is invalid.";return{};}QStringList out;static const QRegularExpression valid("^[a-zA-Z0-9@._+:-]+$");for(auto v:d.array()){auto n=v.toObject().value("package").toString();if(valid.match(n).hasMatch()&&!out.contains(n))out<<n;}return out;
}
