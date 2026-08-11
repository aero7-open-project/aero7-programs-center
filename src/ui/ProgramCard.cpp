#include "ProgramCard.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

ProgramCard::ProgramCard(const Package&p,QWidget*parent):QFrame(parent){
 setObjectName("programCard");setCursor(Qt::PointingHandCursor);setFrameShape(QFrame::StyledPanel);setMinimumSize(230,116);
 auto*h=new QHBoxLayout(this);auto*icon=new QLabel;icon->setPixmap(QIcon::fromTheme(p.iconName.isEmpty()?"package-x-generic":p.iconName).pixmap(48));icon->setFixedSize(52,52);h->addWidget(icon,0,Qt::AlignTop);
 auto*v=new QVBoxLayout;auto*name=new QLabel(p.displayName);name->setObjectName("cardName");name->setWordWrap(true);v->addWidget(name);auto*desc=new QLabel(p.summary);desc->setObjectName("cardSummary");desc->setWordWrap(true);v->addWidget(desc);v->addStretch();
 auto*row=new QHBoxLayout;auto*source=new QLabel(friendlyRepository(p.repository));source->setObjectName("cardSource");row->addWidget(source);row->addStretch();auto*b=new QPushButton(p.state==PackageState::Installed?"Installed":"Install");b->setEnabled(p.state!=PackageState::Installed);row->addWidget(b);v->addLayout(row);h->addLayout(v,1);
 connect(b,&QPushButton::clicked,this,[this,p]{emit actionRequested(p.name);});
}
