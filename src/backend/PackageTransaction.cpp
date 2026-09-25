#include "PackageTransaction.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

PackageTransaction::PackageTransaction(QObject *p):QObject(p){
 connect(&m_process,&QProcess::readyReadStandardOutput,this,[this]{emit progress("Working…",-1,QString::fromUtf8(m_process.readAllStandardOutput()).trimmed());});
 connect(&m_process,&QProcess::readyReadStandardError,this,[this]{emit progress("Working…",-1,QString::fromUtf8(m_process.readAllStandardError()).trimmed());});
 connect(&m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus s){
  const bool ok=s==QProcess::NormalExit&&code==0;
  const QString details=QString::fromUtf8(m_process.readAll());
  if(ok&&!m_followUpProgram.isEmpty()){
   const QString program=m_followUpProgram;const QStringList arguments=m_followUpArguments;
   m_followUpProgram.clear();m_followUpArguments.clear();
   emit progress(QStringLiteral("Installing from Flathub…"),0,{});m_process.start(program,arguments);return;
  }
  m_followUpProgram.clear();m_followUpArguments.clear();emit finished(ok,details);
 });
}
bool PackageTransaction::start(Operation op,const QStringList &packages){
 if(m_process.state()!=QProcess::NotRunning||QFileInfo::exists("/var/lib/pacman/db.lck"))return false;
 static const QRegularExpression valid("^[a-zA-Z0-9@._+:-]+$"); for(const auto&p:packages)if(!valid.match(p).hasMatch())return false;
 QString action=op==Install?"install":op==Remove?"remove":op==UpdateSelected?"update-selected":"update-all";
 QStringList args={"--disable-internal-agent","/usr/lib/aero7-programs-center/aero7-programs-center-helper",action}; args+=packages;
 emit progress("Authenticating…",0,{}); m_process.start("pkexec",args); return true;
}

bool PackageTransaction::startFlatpak(Operation op,const QString &applicationId){
 if(m_process.state()!=QProcess::NotRunning||QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty())return false;
 static const QRegularExpression valid("^[A-Za-z0-9._-]+$");
 if(!valid.match(applicationId).hasMatch()||(op!=Install&&op!=Remove))return false;
 QStringList args;
 if(op==Install)args={QStringLiteral("install"),QStringLiteral("--user"),QStringLiteral("--noninteractive"),QStringLiteral("-y"),QStringLiteral("flathub"),applicationId};
 else args={QStringLiteral("uninstall"),QStringLiteral("--user"),QStringLiteral("--noninteractive"),QStringLiteral("-y"),applicationId};
 emit progress(QStringLiteral("Preparing Flatpak…"),0,{});
 if(op==Install){
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
