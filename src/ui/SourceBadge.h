#pragma once

#include "sources/SourcePolicy.h"

#include <QFrame>

class SourceBadge final : public QFrame {
    Q_OBJECT
public:
    explicit SourceBadge(SoftwareSource source, QWidget *parent = nullptr);
    SoftwareSource source() const { return m_source; }

private:
    SoftwareSource m_source;
};
