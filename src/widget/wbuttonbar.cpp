#include "wbuttonbar.h"

WButtonBar::WButtonBar(QWidget* parent)
        : WWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    setLayout(m_pLayout);
}

WRightClickButton *WButtonBar::addButton(const QIcon& icon, const QVariant& title) {
    WRightClickButton* button = new WRightClickButton(this);
    button->setIcon(icon);
    button->setText(title.toString());
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    
    m_pLayout->addWidget(button);
    return button;
}

