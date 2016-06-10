#include "wbuttonbar.h"

WButtonBar::WButtonBar(QWidget* parent)
        : WWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    setLayout(m_pLayout);
}

QAbstractButton* WButtonBar::addButton(const QIcon& icon, const QVariant& title) {
    QToolButton* button = new QToolButton(this);
    button->setIcon(icon);
    button->setText(title.toString());
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    
    m_pLayout->addWidget(button);
    return button;
}

