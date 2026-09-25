#include "SourceBadge.h"

#include "Aero7Icon.h"

#include <QHBoxLayout>
#include <QLabel>

SourceBadge::SourceBadge(SoftwareSource source, QWidget *parent)
    : QFrame(parent), m_source(source)
{
    const auto info = SourcePolicy::descriptor(source);
    setObjectName(info.trusted ? QStringLiteral("trustedSourceBadge")
                               : QStringLiteral("thirdPartySourceBadge"));
    setToolTip(info.description);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 7, 2);
    layout->setSpacing(4);
    auto *icon = new QLabel;
    icon->setPixmap(Aero7Icons::icon(info.iconKey).pixmap(16, 16));
    icon->setFixedSize(16, 16);
    layout->addWidget(icon);
    auto *text = new QLabel(info.name);
    text->setObjectName(QStringLiteral("sourceBadgeText"));
    layout->addWidget(text);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}
