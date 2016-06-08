#include "wbuttonbar.h"

WButtonBar::WButtonBar(QWidget* parent)
        : WWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    setLayout(m_pLayout);

    m_pButtonGroup = new QButtonGroup(this);
}

void WButtonBar::slotButtonClicked(int id) {
    emit(buttonClicked(m_data[id]));
}

QAbstractButton* WButtonBar::addButton(const QIcon& icon, const QVariant& title) {
    
    QPushButton* button = new QPushButton(icon, title.toString(), this);
    
    m_pLayout->addWidget(button);
    return button;
}

