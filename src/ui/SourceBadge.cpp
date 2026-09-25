#include "SourceBadge.h"

#include "Aero7Icon.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>

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

void SourceBadge::setSource(SoftwareSource source)
{
    if (m_source == source) return;
    m_source = source;
    const auto info = SourcePolicy::descriptor(source);
    setObjectName(info.trusted ? QStringLiteral("trustedSourceBadge")
                               : QStringLiteral("thirdPartySourceBadge"));
    setToolTip(info.description);
    auto *icon = qobject_cast<QLabel *>(layout()->itemAt(0)->widget());
    auto *text = qobject_cast<QLabel *>(layout()->itemAt(1)->widget());
    icon->setPixmap(Aero7Icons::icon(info.iconKey).pixmap(16, 16));
    text->setText(info.name);
    style()->unpolish(this);
    style()->polish(this);
    update();
}
