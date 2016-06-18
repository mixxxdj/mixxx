#include "wbuttonbar.h"

WButtonBar::WButtonBar(QWidget* parent)
        : WWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    setLayout(m_pLayout);
}

WFeatureClickButton* WButtonBar::addButton(const QIcon& icon, 
                                         const QVariant& title,
                                         const QString& data) {
    WFeatureClickButton* button = new WFeatureClickButton(this);
    button->setIcon(icon);
    button->setText(title.toString());
    button->setData(data);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    
    m_pLayout->addWidget(button);
    return button;
}

