#pragma once

#include "sources/SourcePolicy.h"

#include <QFrame>

class SourceBadge final : public QFrame {
    Q_OBJECT
public:
    explicit SourceBadge(SoftwareSource source, const QString &repository = {}, QWidget *parent = nullptr);
    SoftwareSource source() const { return m_source; }
    void setSource(SoftwareSource source, const QString &repository = {});

private:
    SoftwareSource m_source;
    QString m_repository;
};
