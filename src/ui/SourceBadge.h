#pragma once

#include "sources/SourcePolicy.h"

#include <QFrame>

class SourceBadge final : public QFrame {
    Q_OBJECT
public:
    explicit SourceBadge(SoftwareSource source, const QString &repository = {},
                         bool officialConfigured = false, QWidget *parent = nullptr);
    SoftwareSource source() const { return m_source; }
    void setSource(SoftwareSource source, const QString &repository = {}, bool officialConfigured = false);

private:
    SoftwareSource m_source;
    QString m_repository;
    bool m_officialConfigured = false;
};
