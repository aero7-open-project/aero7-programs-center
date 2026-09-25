#pragma once

#include <QList>
#include <QString>

enum class SoftwareSource {
    Aero7,
    Arch,
    ApprovedNative,
    ChaoticAur,
    CachyOs,
    Flathub,
    Local,
    Unknown,
};

enum class PackageProvider {
    Pacman,
    Flatpak,
};

struct SourceDescriptor {
    SoftwareSource source = SoftwareSource::Unknown;
    QString id;
    QString name;
    QString description;
    QString badgeText;
    QString iconKey;
    int priority = 0;
    bool trusted = false;
    bool alwaysEnabled = false;
};

class SourcePolicy {
public:
    static SourceDescriptor descriptor(SoftwareSource source);
    static QList<SourceDescriptor> descriptors();
    static SoftwareSource fromId(const QString &id);
    static SoftwareSource fromRepository(const QString &repository);
    static bool isThirdParty(SoftwareSource source);
    static bool isNative(SoftwareSource source);
    static QString warning(SoftwareSource source);
};
