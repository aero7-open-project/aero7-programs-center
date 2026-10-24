#pragma once
#include <QObject>
#include <QProcess>

class PackageTransaction final : public QObject {
    Q_OBJECT
public:
    enum Operation{Install,Remove,UpdateSelected,UpdateAll};
    explicit PackageTransaction(QObject *parent=nullptr);
    static bool isValidNativeTarget(Operation op, const QString &target);
    static bool canRetryWithNormalAuthentication(int exitCode, QProcess::ExitStatus status,
                                                 const QString &details);
    bool start(Operation op,const QStringList &validatedPackages={});
    bool startTrustedInstall(const QString &repository, const QString &package);
    bool startFlatpak(Operation op,const QString &applicationId,const QString &remote=QStringLiteral("flathub"));
    QString lastStartError() const { return m_startError; }
signals:
    void progress(const QString &phase,int percent,const QString &detail);
    void normalAuthenticationRequired(const QString &reason);
    void finished(bool ok,const QString &details);
private:
    QProcess m_process;
    QString m_startError;
    QString m_output;
    QString m_followUpProgram;
    QStringList m_followUpArguments;
    bool m_trustedInstall = false;
};
