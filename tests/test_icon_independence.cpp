#include "ui/Aero7Icon.h"

#include <QApplication>
#include <QIcon>
#include <QImage>

namespace {

QImage imageFor(const QString &name)
{
    return Aero7Icons::icon(name).pixmap(32, 32).toImage();
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QStringList names = {
        QStringLiteral("home"), QStringLiteral("search"),
        QStringLiteral("install"), QStringLiteral("uninstall"),
        QStringLiteral("update"), QStringLiteral("warning"),
        QStringLiteral("error"), QStringLiteral("success"),
        QStringLiteral("internet"), QStringLiteral("multimedia"),
        QStringLiteral("graphics and photos"), QStringLiteral("office")
    };

    QHash<QString, QImage> baseline;
    QIcon::setThemeName(QStringLiteral("Aero7-test-theme"));
    for (const QString &name : names)
        baseline.insert(name, imageFor(name));

    for (const QString &theme : {QStringLiteral("breeze"),
                                 QStringLiteral("breeze-dark"),
                                 QStringLiteral("missing-aero7-test-theme")}) {
        QIcon::setThemeName(theme);
        for (const QString &name : names) {
            const QImage candidate = imageFor(name);
            if (candidate.isNull() || candidate != baseline.value(name))
                return 1;
        }
    }

    if (Aero7Icons::resource(QStringLiteral(":/missing/aero7-icon.svg"),
                             QStringLiteral("package")).isNull())
        return 2;
    return 0;
}
