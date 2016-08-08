#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

#include "wbuttonbar.h"
#include "library/libraryfeature.h"

WButtonBar::WButtonBar(QWidget* parent)
        : QFrame(parent) {
    
    QHBoxLayout* pHb = new QHBoxLayout(this);
    pHb->setContentsMargins(0,0,0,0);
    
    QWidget* w1 = new QWidget(this);
    QWidget* w2 = new QWidget(this);
    
    // QSizePolicy::Maximum -> treat the size hint as maximum. This protects us 
    // from growing to the scroll area size which includes the Scroll bar. 
    w1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    w2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    pHb->addWidget(w1);
    pHb->addWidget(w2);
    
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    m_pLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    w1->setLayout(m_pLayout);
    setLayout(pHb);
    
    setFocusPolicy(Qt::NoFocus);
    setAutoFillBackground(true);
}

WFeatureClickButton* WButtonBar::addButton(LibraryFeature* pFeature) {
    WFeatureClickButton* button = new WFeatureClickButton(pFeature, this);    
    m_pLayout->addWidget(button);
    updateGeometry();
    return button;
}
