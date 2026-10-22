#pragma once
#include "models/Package.h"
#include <QFrame>
class QMouseEvent;
class QLabel; class QPushButton;
class ProgramCard final:public QFrame{
 Q_OBJECT
public: explicit ProgramCard(const Package&, bool officialConfigured = false, QWidget *parent=nullptr);
signals:void detailsRequested(const QString&);void actionRequested(const QString&);
protected:void mouseReleaseEvent(QMouseEvent *event) override;
private:QString m_productId;
};
