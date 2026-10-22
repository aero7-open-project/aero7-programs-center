#include "PackageTransaction.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

PackageTransaction::PackageTransaction(QObject *p):QObject(p){
 connect(&m_process,&QProcess::readyReadStandardOutput,this,[this]{
  const QString chunk=QString::fromUtf8(m_process.readAllStandardOutput());
  m_output=(m_output+chunk).right(16000);
  emit progress("Working…",-1,chunk.trimmed());
 });
 connect(&m_process,&QProcess::readyReadStandardError,this,[this]{
  const QString chunk=QString::fromUtf8(m_process.readAllStandardError());
  m_output=(m_output+chunk).right(16000);
  emit progress("Working…",-1,chunk.trimmed());
 });
 connect(&m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus s){
  const bool ok=s==QProcess::NormalExit&&code==0;
  const QString details=(m_output+QString::fromUtf8(m_process.readAll())).right(16000);
  if(ok&&!m_followUpProgram.isEmpty()){
   const QString program=m_followUpProgram;const QStringList arguments=m_followUpArguments;
   m_followUpProgram.clear();m_followUpArguments.clear();
   emit progress(QStringLiteral("Installing from Flathub…"),0,{});m_process.start(program,arguments);return;
  }
  m_followUpProgram.clear();m_followUpArguments.clear();m_output.clear();emit finished(ok,details);
 });
}
bool PackageTransaction::start(Operation op,const QStringList &packages){
 if(m_process.state()!=QProcess::NotRunning||QFileInfo::exists("/var/lib/pacman/db.lck"))return false;
 static const QRegularExpression valid("^[a-zA-Z0-9@._+:-]+$"); for(const auto&p:packages)if(!valid.match(p).hasMatch())return false;
 QString action=op==Install?"install":op==Remove?"remove":op==UpdateSelected?"update-selected":"update-all";
 QStringList args={"--disable-internal-agent","/usr/lib/aero7-programs-center/aero7-programs-center-helper",action}; args+=packages;
 emit progress("Authenticating…",0,{}); m_process.start("pkexec",args); return true;
}

bool PackageTransaction::startFlatpak(Operation op,const QString &applicationId,const QString &remote){
 if(m_process.state()!=QProcess::NotRunning||QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty())return false;
 static const QRegularExpression valid("^[A-Za-z0-9._-]+$");
 if(!valid.match(applicationId).hasMatch()||!valid.match(remote).hasMatch()||(op!=Install&&op!=Remove))return false;
 QStringList args;
 if(op==Install)args={QStringLiteral("install"),QStringLiteral("--user"),QStringLiteral("--noninteractive"),QStringLiteral("-y"),remote,applicationId};
 else args={QStringLiteral("uninstall"),QStringLiteral("--user"),QStringLiteral("--noninteractive"),QStringLiteral("-y"),applicationId};
 emit progress(QStringLiteral("Preparing Flatpak…"),0,{});
 if(op==Install && remote==QStringLiteral("flathub")){
  QProcess remotes;remotes.start(QStringLiteral("flatpak"),{QStringLiteral("remotes"),QStringLiteral("--user"),QStringLiteral("--columns=name")});
  const bool found=remotes.waitForFinished(5000)&&QString::fromUtf8(remotes.readAllStandardOutput()).split('\n',Qt::SkipEmptyParts).contains(QStringLiteral("flathub"));
  if(!found){
   m_followUpProgram=QStringLiteral("flatpak");m_followUpArguments=args;
   m_process.start(QStringLiteral("flatpak"),{QStringLiteral("remote-add"),QStringLiteral("--user"),QStringLiteral("--if-not-exists"),QStringLiteral("flathub"),QStringLiteral("https://dl.flathub.org/repo/flathub.flatpakrepo")});
   return m_process.waitForStarted(3000);
  }
 }
 m_process.start(QStringLiteral("flatpak"),args);
 return m_process.waitForStarted(3000);
}

bool PackageTransaction::startTrustedInstall(const QString &repository,const QString &package){
 if(m_process.state()!=QProcess::NotRunning||QFileInfo::exists(QStringLiteral("/var/lib/pacman/db.lck")))return false;
 static const QRegularExpression validPackage(QStringLiteral("^[A-Za-z0-9@._+-]+$"));
 if(!QStringList{QStringLiteral("aero7"),QStringLiteral("core"),QStringLiteral("extra"),QStringLiteral("multilib")}.contains(repository)
    ||!validPackage.match(package).hasMatch())return false;
 emit progress(QStringLiteral("Checking trusted package…"),0,{});
 m_process.start(QStringLiteral("pkexec"),{QStringLiteral("--disable-internal-agent"),
  QStringLiteral("/usr/lib/aero7-programs-center/aero7-programs-center-trusted-installer"),
  repository,package});
 return m_process.waitForStarted(3000);
}
