#include "ui/Aero7Icon.h"

#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QRect>

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

    QHash<int, QImage> sourceBaseline;
    QIcon::setThemeName(QStringLiteral("missing-aero7-test-theme"));
    for (const auto source : {SoftwareSource::Aero7, SoftwareSource::Arch,
                              SoftwareSource::ChaoticAur, SoftwareSource::CachyOs,
                              SoftwareSource::Flathub, SoftwareSource::Unknown}) {
        const QImage candidate = Aero7Icons::sourceIcon(source).pixmap(48, 48).toImage();
        if (candidate.isNull()) return 5;
        sourceBaseline.insert(static_cast<int>(source), candidate);
    }
    QIcon::setThemeName(QStringLiteral("breeze"));
    for (const auto source : {SoftwareSource::Aero7, SoftwareSource::Arch,
                              SoftwareSource::ChaoticAur, SoftwareSource::CachyOs,
                              SoftwareSource::Flathub, SoftwareSource::Unknown})
        if (Aero7Icons::sourceIcon(source).pixmap(48, 48).toImage()
            != sourceBaseline.value(static_cast<int>(source))) return 6;

    if (Aero7Icons::resource(QStringLiteral(":/missing/aero7-icon.svg"),
                             QStringLiteral("package")).isNull())
        return 2;

    // A named hicolor icon must occupy the taskbar slot, independent of the
    // desktop's theme and of which standard icon size the shell requests.
    for (const int size : {16, 22, 24, 32, 48, 64, 128, 256, 512}) {
        const QString path = QStringLiteral(AERO7_PROGRAMS_CENTER_SOURCE_DIR)
            + QStringLiteral("/resources/icons/%1x%1/aero7-programs-center.png").arg(size);
        const QImage icon(path);
        if (icon.isNull() || icon.width() != size || icon.height() != size)
            return 3;
        QRect content;
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                if (icon.pixelColor(x, y).alpha() != 0)
                    content |= QRect(x, y, 1, 1);
            }
        }
        if (content.width() * 100 < size * 90 || content.height() * 100 < size * 95)
            return 4;
    }
    return 0;
}
