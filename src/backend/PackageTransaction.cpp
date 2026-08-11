#include "PackageTransaction.h"
#include <QFileInfo>
#include <QRegularExpression>

PackageTransaction::PackageTransaction(QObject *p):QObject(p){
 connect(&m_process,&QProcess::readyReadStandardOutput,this,[this]{emit progress("Working…",-1,QString::fromUtf8(m_process.readAllStandardOutput()).trimmed());});
 connect(&m_process,&QProcess::readyReadStandardError,this,[this]{emit progress("Working…",-1,QString::fromUtf8(m_process.readAllStandardError()).trimmed());});
 connect(&m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus s){emit finished(s==QProcess::NormalExit&&code==0,QString::fromUtf8(m_process.readAll()));});
}
bool PackageTransaction::start(Operation op,const QStringList &packages){
 if(m_process.state()!=QProcess::NotRunning||QFileInfo::exists("/var/lib/pacman/db.lck"))return false;
 static const QRegularExpression valid("^[a-zA-Z0-9@._+:-]+$"); for(const auto&p:packages)if(!valid.match(p).hasMatch())return false;
 QString action=op==Install?"install":op==Remove?"remove":op==UpdateSelected?"update-selected":"update-all";
 QStringList args={"--disable-internal-agent","/usr/lib/aero7-programs-center/aero7-programs-center-helper",action}; args+=packages;
 emit progress("Authenticating…",0,{}); m_process.start("pkexec",args); return true;
}
