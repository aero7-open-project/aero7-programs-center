#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

// User-added source names cannot borrow any built-in source identity.
// Flatpak remotes use a key-bearing .flatpakrepo description in the user's installation.
class SourceRegistration {
public:
    static bool validName(const QString &name);
    static bool validFlatpakRepo(const QString &name, const QString &url,
                                 const QByteArray &description, QString *error = nullptr);
};
