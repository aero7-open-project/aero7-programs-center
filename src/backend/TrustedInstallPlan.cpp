#include "TrustedInstallPlan.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QHash>
#include <QFileInfo>

namespace {
const QRegularExpression packageName(QStringLiteral("\\A[A-Za-z0-9@._+-]+\\z"));
const QRegularExpression expressionToken(QStringLiteral("\\A[A-Za-z0-9@._+:/<>=-]+\\z"));

struct CommandResult {
    QString output;
    QString error;
};

CommandResult runPacman(const QString &executable, const QStringList &arguments)
{
    QProcess process;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process.setProcessEnvironment(environment);
    process.start(executable, arguments);
    if (!process.waitForStarted(3000)) return {{}, QStringLiteral("Could not start the package preflight.")};
    if (!process.waitForFinished(15000)) {
        process.kill();
        process.waitForFinished(1000);
        return {{}, QStringLiteral("The package preflight timed out.")};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {{}, QStringLiteral("The package preflight failed (exit %1).")
            .arg(process.exitCode())};
    const QByteArray output = process.readAllStandardOutput();
    if (output.size() > 1024 * 1024) return {{}, QStringLiteral("The package preflight returned too much data.")};
    return {QString::fromUtf8(output), {}};
}

QStringList tokens(const QString &value)
{
    return value.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}
}

TrustedInstallPlan TrustedInstallPlan::inspect(const QString &target, const QString &pacmanExecutable)
{
    TrustedInstallPlan result;
    result.target = target;
    if (QFileInfo::exists(QStringLiteral("/var/lib/pacman/db.lck"))) {
        result.error = QStringLiteral("Another package transaction is in progress.");
        return result;
    }
    const QStringList targetParts = target.split(QLatin1Char('/'));
    if (targetParts.size() != 2 || !packageName.match(targetParts.at(0)).hasMatch()
        || !packageName.match(targetParts.at(1)).hasMatch()) {
        result.error = QStringLiteral("The package target is invalid.");
        return result;
    }
    const auto plan = runPacman(pacmanExecutable,
        {QStringLiteral("--sync"), QStringLiteral("--needed"), QStringLiteral("--print"),
         QStringLiteral("--print-format"), QStringLiteral("%r/%n"), QStringLiteral("--"), target});
    if (!plan.error.isEmpty()) { result.error = plan.error; return result; }
    QStringList planned;
    QSet<QString> expected;
    for (const QString &line : plan.output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList parts = line.split(QLatin1Char('/'));
        if (parts.size() != 2 || !packageName.match(parts.at(0)).hasMatch()
            || !packageName.match(parts.at(1)).hasMatch() || expected.contains(line)) {
            result.error = QStringLiteral("The package preflight returned an invalid package list.");
            return result;
        }
        planned.append(line);
        expected.insert(line);
    }
    if (planned.isEmpty() || planned.size() > 256) {
        result.error = QStringLiteral("The package preflight returned no usable package plan.");
        return result;
    }

    QStringList metadataArguments = {QStringLiteral("--sync"), QStringLiteral("--info"), QStringLiteral("--")};
    metadataArguments.append(planned);
    const auto metadata = runPacman(pacmanExecutable, metadataArguments);
    if (!metadata.error.isEmpty()) { result.error = metadata.error; return result; }
    QSet<QString> seen;
    const QRegularExpression fieldLine(QStringLiteral("^([^:]+):[ \\t]*(.*)$"));
    for (const QString &block : metadata.output.split(QRegularExpression(QStringLiteral("\\n[ \\t]*\\n")), Qt::SkipEmptyParts)) {
        QString repository, name;
        QHash<QString, QString> fields;
        QString currentField;
        for (const QString &line : block.split(QLatin1Char('\n'))) {
            const auto match = fieldLine.match(line);
            if (match.hasMatch()) {
                currentField = match.captured(1).trimmed();
                fields.insert(currentField, match.captured(2).trimmed());
            } else if (!currentField.isEmpty() && !line.trimmed().isEmpty()) {
                fields[currentField] += QLatin1Char(' ') + line.trimmed();
            }
        }
        repository = fields.value(QStringLiteral("Repository"));
        name = fields.value(QStringLiteral("Name"));
        const QString package = repository + QLatin1Char('/') + name;
        if (!expected.contains(package) || seen.contains(package)
            || !fields.contains(QStringLiteral("Conflicts With"))
            || !fields.contains(QStringLiteral("Replaces"))) {
            result.error = QStringLiteral("The package preflight returned incomplete metadata.");
            return result;
        }
        seen.insert(package);
        for (const QString &field : {QStringLiteral("Conflicts With"), QStringLiteral("Replaces")}) {
            const QString value = fields.value(field);
            if (value.isEmpty()) {
                result.error = QStringLiteral("The package preflight returned incomplete metadata.");
                return result;
            }
            if (value == QStringLiteral("None")) continue;
            for (const QString &expression : tokens(value)) {
                if (!expressionToken.match(expression).hasMatch()) {
                    result.error = QStringLiteral("The package preflight returned invalid conflict metadata.");
                    return result;
                }
                const QString namedPackage = expression.section(QRegularExpression(QStringLiteral("[<>=]")), 0, 0);
                if (!packageName.match(namedPackage).hasMatch()) {
                    result.error = QStringLiteral("The package preflight returned invalid conflict metadata.");
                    return result;
                }
                result.declarations.append({package, field, expression, namedPackage, false});
            }
        }
    }
    if (seen != expected) {
        result.error = QStringLiteral("The package preflight omitted package metadata.");
        return result;
    }

    const auto installed = runPacman(pacmanExecutable, {QStringLiteral("--query"), QStringLiteral("--quiet")});
    if (!installed.error.isEmpty()) { result.error = installed.error; return result; }
    QSet<QString> installedNames;
    for (const QString &line : installed.output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        if (!packageName.match(line).hasMatch()) {
            result.error = QStringLiteral("The installed-package snapshot was invalid.");
            return result;
        }
        installedNames.insert(line);
    }
    for (const QString &package : planned) {
        if (installedNames.contains(package.section(QLatin1Char('/'), 1)))
            result.installedPackagesInPlan.append(package);
        else result.newPackages.append(package);
    }
    for (auto &declaration : result.declarations)
        declaration.namedPackageInstalled = installedNames.contains(declaration.namedPackage);
    if (QFileInfo::exists(QStringLiteral("/var/lib/pacman/db.lck")))
        result.error = QStringLiteral("The package database changed during preflight.");
    return result;
}

QString TrustedInstallPlan::detailsText() const
{
    if (!complete()) return QStringLiteral("Package details could not be verified: %1").arg(error);
    QStringList lines = {
        QStringLiteral("Read-only package plan for %1").arg(target),
        QString(),
        QStringLiteral("New packages in this dry-run plan (%1):").arg(newPackages.size())
    };
    for (const auto &package : newPackages) lines.append(QStringLiteral("  %1").arg(package));
    if (!installedPackagesInPlan.isEmpty()) {
        lines.append(QString());
        lines.append(QStringLiteral("Already-installed package names in the plan (possible updates):"));
        for (const auto &package : installedPackagesInPlan) lines.append(QStringLiteral("  %1").arg(package));
    }
    lines.append(QString());
    lines.append(QStringLiteral("Declared replacement or conflict metadata:"));
    for (const auto &declaration : declarations) {
        lines.append(QStringLiteral("  %1 — %2: %3").arg(declaration.package, declaration.field, declaration.expression));
        lines.append(declaration.namedPackageInstalled
            ? QStringLiteral("    %1 is installed by name; a matching version or actual removal is not confirmed.").arg(declaration.namedPackage)
            : QStringLiteral("    %1 is not installed by name; no direct replacement of it is shown.").arg(declaration.namedPackage));
    }
    lines.append(QString());
    lines.append(QStringLiteral("Actual removals/replacements: not determined by this dry-run output. It lists packages to install or update, but does not report removals or resolve all provider and version matches. The normal installer resolves the transaction again and does not offer another package-by-package confirmation. Continue only if you accept this limit."));
    return lines.join(QLatin1Char('\n'));
}
