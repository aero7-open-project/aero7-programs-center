#include "SourceCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <algorithm>

namespace {
const QRegularExpression &validIdentifier()
{
    static const QRegularExpression expression(QStringLiteral("^[A-Za-z0-9@._+:-]+$"));
    return expression;
}

bool betterVariant(const Package &left, const Package &right)
{
    if (left.sourcePriority != right.sourcePriority)
        return left.sourcePriority > right.sourcePriority;
    if (left.provider != right.provider)
        return left.provider == PackageProvider::Pacman;
    return left.name < right.name;
}
}

QString SourceCatalog::key(SoftwareSource source, const QString &packageId)
{
    return SourcePolicy::descriptor(source).id + QLatin1Char(':') + packageId.toLower();
}

bool SourceCatalog::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(QStringLiteral(":/catalog/sources.json"));
        if (!file.open(QIODevice::ReadOnly)) {
            m_error = QStringLiteral("The Aero7 source catalog is unavailable.");
            return false;
        }
    }
    return loadBytes(file.readAll(), &m_error);
}

bool SourceCatalog::loadBytes(const QByteArray &data, QString *error)
{
    m_rules.clear();
    m_cachyAllowlist.clear();
    m_error.clear();
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        m_error = QStringLiteral("The Aero7 source catalog is invalid JSON.");
        if (error) *error = m_error;
        return false;
    }
    const auto root = document.object();
    if (root.value(QStringLiteral("schema")).toInt() != 1) {
        m_error = QStringLiteral("The Aero7 source catalog uses an unsupported schema.");
        if (error) *error = m_error;
        return false;
    }

    for (const auto value : root.value(QStringLiteral("approved_cachyos_packages")).toArray()) {
        const QString packageId = value.toString();
        if (!validIdentifier().match(packageId).hasMatch()) {
            m_error = QStringLiteral("The CachyOS application allowlist contains an invalid package name.");
            if (error) *error = m_error;
            return false;
        }
        m_cachyAllowlist.insert(packageId.toLower());
    }

    for (const auto value : root.value(QStringLiteral("applications")).toArray()) {
        const auto application = value.toObject();
        const QString product = application.value(QStringLiteral("id")).toString();
        if (!validIdentifier().match(product).hasMatch()) {
            m_error = QStringLiteral("The source catalog contains an invalid product identifier.");
            if (error) *error = m_error;
            return false;
        }
        for (const auto variantValue : application.value(QStringLiteral("variants")).toArray()) {
            const auto variant = variantValue.toObject();
            const QString sourceId = variant.value(QStringLiteral("source")).toString();
            const QString packageId = variant.value(QStringLiteral("package")).toString();
            const auto source = SourcePolicy::fromId(sourceId);
            if (source == SoftwareSource::Unknown
                || !validIdentifier().match(packageId).hasMatch()) {
                m_error = QStringLiteral("The source catalog contains an invalid source variant.");
                if (error) *error = m_error;
                return false;
            }
            if (source == SoftwareSource::CachyOs
                && !m_cachyAllowlist.contains(packageId.toLower())) {
                m_error = QStringLiteral("A CachyOS variant is missing from the Aero7 allowlist.");
                if (error) *error = m_error;
                return false;
            }
            const int requestedPriority = variant.value(QStringLiteral("priority")).toInt();
            const int defaultPriority = SourcePolicy::descriptor(source).priority;
            m_rules.insert(key(source, packageId), {
                product,
                requestedPriority > 0 ? requestedPriority : defaultPriority,
                variant.value(QStringLiteral("approved_system_replacement")).toBool(false),
            });
        }
    }
    if (error) error->clear();
    return true;
}

QString SourceCatalog::productId(SoftwareSource source, const QString &packageId,
                                 const QString &fallback) const
{
    const auto found = m_rules.constFind(key(source, packageId));
    if (found != m_rules.cend()) return found->productId;
    return fallback.isEmpty() ? packageId : fallback;
}

int SourceCatalog::priority(SoftwareSource source, const QString &packageId) const
{
    const auto found = m_rules.constFind(key(source, packageId));
    return found == m_rules.cend() ? SourcePolicy::descriptor(source).priority : found->priority;
}

bool SourceCatalog::cachyPackageAllowed(const QString &packageId) const
{
    return m_cachyAllowlist.contains(packageId.toLower());
}

bool SourceCatalog::systemReplacementApproved(SoftwareSource source,
                                              const QString &packageId) const
{
    const auto found = m_rules.constFind(key(source, packageId));
    return found != m_rules.cend() && found->approvedSystemReplacement;
}

QList<Package> SourceCatalog::groupProducts(const QList<Package> &packages) const
{
    QHash<QString, QList<Package>> groups;
    for (auto package : packages) {
        const QString fallback = package.productId.isEmpty() ? package.name : package.productId;
        package.productId = productId(package.source, package.name, fallback);
        package.sourcePriority = priority(package.source, package.name);
        package.approvedSystemReplacement = systemReplacementApproved(package.source, package.name);
        if (package.approvedSystemReplacement) package.blockedReason.clear();
        if (package.source == SoftwareSource::CachyOs && !cachyPackageAllowed(package.name))
            continue;
        groups[package.productId.toLower()].append(package);
    }

    QList<Package> products;
    products.reserve(groups.size());
    for (auto group : groups) {
        std::sort(group.begin(), group.end(), betterVariant);
        int selected = 0;
        for (int index = 0; index < group.size(); ++index) {
            if (group.at(index).state == PackageState::Installed
                || group.at(index).state == PackageState::UpdateAvailable) {
                selected = index;
                break;
            }
        }
        Package product = group.at(selected);
        product.variants.clear();
        for (const auto &variant : group) product.variants.append(packageVariant(variant));
        products.append(product);
    }
    std::sort(products.begin(), products.end(), [](const Package &left, const Package &right) {
        return left.displayName.localeAwareCompare(right.displayName) < 0;
    });
    return products;
}
