#include "ProgramCard.h"
#include "PackageIcon.h"
#include "SourceBadge.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

ProgramCard::ProgramCard(const Package&p,bool officialConfigured,QWidget*parent):QFrame(parent),m_productId(p.productId.isEmpty()?p.name:p.productId){
 setObjectName("programCard");setCursor(Qt::PointingHandCursor);setFrameShape(QFrame::StyledPanel);setMinimumSize(250,136);
 setAccessibleName(p.displayName);setToolTip(QStringLiteral("Open details for %1").arg(p.displayName));
 auto*h=new QHBoxLayout(this);auto*icon=new QLabel;icon->setPixmap(packageArtwork(p).pixmap(48,48));icon->setFixedSize(52,52);h->addWidget(icon,0,Qt::AlignTop);
 auto*v=new QVBoxLayout;auto*name=new QLabel(p.displayName);name->setObjectName("cardName");name->setWordWrap(true);v->addWidget(name);auto*desc=new QLabel(p.summary);desc->setObjectName("cardSummary");desc->setWordWrap(true);v->addWidget(desc);v->addStretch();
 if(SourcePolicy::isThirdParty(p.source)){auto*warning=new QLabel("Third-party software");warning->setObjectName("cardWarning");v->addWidget(warning);}
 auto*row=new QHBoxLayout;row->addWidget(new SourceBadge(p.source,p.repository,officialConfigured));row->addStretch();auto*b=new QPushButton(p.state==PackageState::Installed?"Installed":p.blockedReason.isEmpty()?"Install":"Unavailable");b->setEnabled(p.state!=PackageState::Installed&&p.blockedReason.isEmpty());row->addWidget(b);v->addLayout(row);h->addLayout(v,1);
 connect(b,&QPushButton::clicked,this,[this,p]{emit actionRequested(p.name);});
}

void ProgramCard::mouseReleaseEvent(QMouseEvent *event)
{
 if(event->button()==Qt::LeftButton)emit detailsRequested(m_productId);
 QFrame::mouseReleaseEvent(event);
}
