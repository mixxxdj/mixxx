#include "wbuttonbar.h"

WButtonBar::WButtonBar(QWidget* parent)
        : QWidget(parent) {

    m_pLayout = new QVBoxLayout(this);
    this->setLayout(m_pLayout);

    m_pButtonGroup = new QButtonGroup(this);
    connect(m_pButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotButtonClicked(int)));
}

void WButtonBar::slotButtonClicked(int id) {
    emit(buttonClicked(m_data[id]));
}

void WButtonBar::addItem(const QIcon& icon, const QVariant& title, const QVariant& data) {
    QPushButton* button = new QPushButton(icon, title.toString(), this);

    m_pButtonGroup->addButton(button, m_data.size());
    m_pLayout->addWidget(button);
    m_data.append(data);
}

