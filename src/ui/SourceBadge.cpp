#include "SourceBadge.h"

#include "Aero7Icon.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>

SourceBadge::SourceBadge(SoftwareSource source, const QString &repository, QWidget *parent)
    : QFrame(parent), m_source(source), m_repository(repository)
{
    const auto info = SourcePolicy::descriptor(source);
    setObjectName(info.trusted ? QStringLiteral("trustedSourceBadge")
                               : QStringLiteral("thirdPartySourceBadge"));
    setToolTip(info.description);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 7, 2);
    layout->setSpacing(4);
    auto *icon = new QLabel;
    icon->setPixmap(Aero7Icons::sourceIcon(source).pixmap(16, 16));
    icon->setFixedSize(16, 16);
    layout->addWidget(icon);
    auto *text = new QLabel(source == SoftwareSource::Unknown && !repository.isEmpty()
        ? QStringLiteral("External: %1").arg(repository) : info.name);
    text->setObjectName(QStringLiteral("sourceBadgeText"));
    layout->addWidget(text);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

void SourceBadge::setSource(SoftwareSource source, const QString &repository)
{
    if (m_source == source && m_repository == repository) return;
    m_source = source;
    m_repository = repository;
    const auto info = SourcePolicy::descriptor(source);
    setObjectName(info.trusted ? QStringLiteral("trustedSourceBadge")
                               : QStringLiteral("thirdPartySourceBadge"));
    setToolTip(info.description);
    auto *icon = qobject_cast<QLabel *>(layout()->itemAt(0)->widget());
    auto *text = qobject_cast<QLabel *>(layout()->itemAt(1)->widget());
    icon->setPixmap(Aero7Icons::sourceIcon(source).pixmap(16, 16));
    text->setText(source == SoftwareSource::Unknown && !repository.isEmpty()
        ? QStringLiteral("External: %1").arg(repository) : info.name);
    style()->unpolish(this);
    style()->polish(this);
    update();
}
