#include "SourceRegistration.h"
#include "SourcePolicy.h"

#include <QRegularExpression>
#include <QUrl>

bool SourceRegistration::validName(const QString &name)
{
    static const QRegularExpression pattern(QStringLiteral("^[a-z0-9][a-z0-9._-]{0,63}$"));
    return pattern.match(name).hasMatch()
        && SourcePolicy::fromId(name) == SoftwareSource::Unknown
        && SourcePolicy::fromRepository(name) == SoftwareSource::Unknown;
}

bool SourceRegistration::validFlatpakRepo(const QString &name, const QString &url,
                                          const QByteArray &description, QString *error)
{
    auto reject = [error](const QString &reason) { if (error) *error = reason; return false; };
    const QUrl address(url);
    if (!validName(name)) return reject(QStringLiteral("Choose a unique external source name using lowercase letters, digits, dots, dashes or underscores."));
    if (!address.isValid() || address.scheme() != QStringLiteral("https")
        || address.host().isEmpty() || !address.userInfo().isEmpty()
        || !address.path().endsWith(QStringLiteral(".flatpakrepo")))
        return reject(QStringLiteral("Use a direct HTTPS .flatpakrepo URL without embedded credentials."));
    if (description.size() > 256 * 1024 || description.isEmpty())
        return reject(QStringLiteral("The Flatpak repository description is empty or too large."));
    const QString text = QString::fromUtf8(description);
    if (!text.contains(QRegularExpression(QStringLiteral("(?m)^\\[Flatpak Repo\\]\\s*$"))))
        return reject(QStringLiteral("The file is not a Flatpak repository description."));
    if (text.count(QRegularExpression(QStringLiteral("(?m)^Url="))) != 1
        || text.count(QRegularExpression(QStringLiteral("(?m)^GPGKey="))) != 1)
        return reject(QStringLiteral("The repository description must contain one URL and one signing key."));
    const auto repositoryUrl = QRegularExpression(QStringLiteral("(?m)^Url=(https://[^\\s]+)\\s*$")).match(text);
    if (!repositoryUrl.hasMatch() || QUrl(repositoryUrl.captured(1)).host().isEmpty())
        return reject(QStringLiteral("The repository must use HTTPS."));
    if (!text.contains(QRegularExpression(QStringLiteral("(?m)^GPGKey=[A-Za-z0-9+/=]{40,}\\s*$"))))
        return reject(QStringLiteral("The repository description must include a GPG signing key."));
    if (text.contains(QRegularExpression(QStringLiteral("(?mi)^GPGVerify\\s*=\\s*(false|no|0)\\s*$"))))
        return reject(QStringLiteral("The repository description disables GPG verification."));
    if (error) error->clear();
    return true;
}
