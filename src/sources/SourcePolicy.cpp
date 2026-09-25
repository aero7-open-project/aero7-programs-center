#include "SourcePolicy.h"

namespace {
const QList<SourceDescriptor> &allSources()
{
    static const QList<SourceDescriptor> sources = {
        {SoftwareSource::Aero7, QStringLiteral("aero7"), QStringLiteral("Aero7"),
         QStringLiteral("Official Aero7 software, integration packages and updates."),
         QStringLiteral("Official Aero7 package"), QStringLiteral("success"), 600, true, true},
        {SoftwareSource::Arch, QStringLiteral("arch"), QStringLiteral("Arch Linux"),
         QStringLiteral("Official Arch Linux repositories."),
         QStringLiteral("Official Arch Linux package"), QStringLiteral("applications-system"), 500, true, true},
        {SoftwareSource::ApprovedNative, QStringLiteral("approved-native"),
         QStringLiteral("Aero7 approved"),
         QStringLiteral("A native third-party package reviewed for Aero7 compatibility."),
         QStringLiteral("Aero7-approved package"), QStringLiteral("information"), 400, false, false},
        {SoftwareSource::ChaoticAur, QStringLiteral("chaotic-aur"), QStringLiteral("Chaotic-AUR"),
         QStringLiteral("Precompiled community applications. Direct AUR access is not used."),
         QStringLiteral("Third-party software"), QStringLiteral("warning"), 300, false, false},
        {SoftwareSource::CachyOs, QStringLiteral("cachyos"), QStringLiteral("CachyOS Applications"),
         QStringLiteral("Only applications present in Aero7's maintained allowlist are shown."),
         QStringLiteral("Third-party software"), QStringLiteral("warning"), 200, false, false},
        {SoftwareSource::Flathub, QStringLiteral("flathub"), QStringLiteral("Flathub"),
         QStringLiteral("Flatpak applications from the external Flathub service."),
         QStringLiteral("Flatpak · Third-party software"), QStringLiteral("warning"), 100, false, false},
        {SoftwareSource::Local, QStringLiteral("local"), QStringLiteral("Installed locally"),
         QStringLiteral("Installed software that is not present in an enabled catalog."),
         QStringLiteral("Local package"), QStringLiteral("package"), 50, false, false},
        {SoftwareSource::Unknown, QStringLiteral("unknown"), QStringLiteral("Other source"),
         QStringLiteral("A configured source that is not maintained by Aero7 or Arch Linux."),
         QStringLiteral("Third-party software"), QStringLiteral("warning"), 25, false, false},
    };
    return sources;
}
}

SourceDescriptor SourcePolicy::descriptor(SoftwareSource source)
{
    for (const auto &candidate : allSources())
        if (candidate.source == source) return candidate;
    return allSources().constLast();
}

QList<SourceDescriptor> SourcePolicy::descriptors()
{
    return allSources();
}

SoftwareSource SourcePolicy::fromId(const QString &id)
{
    const QString normalized = id.trimmed().toLower();
    for (const auto &candidate : allSources())
        if (candidate.id == normalized) return candidate.source;
    return SoftwareSource::Unknown;
}

SoftwareSource SourcePolicy::fromRepository(const QString &repository)
{
    const QString name = repository.trimmed().toLower();
    if (name == QStringLiteral("aero7")) return SoftwareSource::Aero7;
    if (name == QStringLiteral("core") || name == QStringLiteral("extra")
        || name == QStringLiteral("multilib") || name == QStringLiteral("core-testing")
        || name == QStringLiteral("extra-testing") || name == QStringLiteral("multilib-testing")
        || name == QStringLiteral("gnome-unstable") || name == QStringLiteral("kde-unstable"))
        return SoftwareSource::Arch;
    if (name.startsWith(QStringLiteral("chaotic-aur"))) return SoftwareSource::ChaoticAur;
    if (name.startsWith(QStringLiteral("cachyos"))) return SoftwareSource::CachyOs;
    if (name == QStringLiteral("flathub")) return SoftwareSource::Flathub;
    if (name == QStringLiteral("local")) return SoftwareSource::Local;
    return SoftwareSource::Unknown;
}

bool SourcePolicy::isThirdParty(SoftwareSource source)
{
    return !descriptor(source).trusted
        && source != SoftwareSource::Local;
}

bool SourcePolicy::isNative(SoftwareSource source)
{
    return source != SoftwareSource::Flathub;
}

QString SourcePolicy::warning(SoftwareSource source)
{
    if (!isThirdParty(source)) return {};
    if (source == SoftwareSource::Flathub)
        return QStringLiteral("This application is provided through an external software source rather than the native Aero7 package system.");
    return QStringLiteral("This package is provided by a third-party software source and is not maintained by Aero7 or Arch Linux.");
}
