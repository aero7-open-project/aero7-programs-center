#pragma once
#include "models/Package.h"
#include <QFrame>
class QLabel; class QPushButton;
class ProgramCard final:public QFrame{
 Q_OBJECT
public: explicit ProgramCard(const Package&,QWidget *parent=nullptr);
signals:void detailsRequested(const QString&);void actionRequested(const QString&);
};
