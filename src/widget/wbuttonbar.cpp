#include <QKeyEvent>

#include "wbuttonbar.h"
#include "library/libraryfeature.h"

WButtonBar::WButtonBar(QWidget* parent)
        : WWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    setLayout(m_pLayout);
    setFocusPolicy(Qt::NoFocus);
}

WFeatureClickButton* WButtonBar::addButton(LibraryFeature* pFeature) {
    WFeatureClickButton* button = new WFeatureClickButton(pFeature, this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    //button->setFocusPolicy(Qt::NoFocus);
    
    m_pLayout->addWidget(button);
    return button;
}

