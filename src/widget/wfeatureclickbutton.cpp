#include "wfeatureclickbutton.h"

WFeatureClickButton::WFeatureClickButton(QWidget* parent)
        : QToolButton(parent) {
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

void WFeatureClickButton::setData(const QString& data) {
    m_data = data;
}



void WFeatureClickButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit(rightClicked(event->globalPos()));
    }
    QToolButton::mousePressEvent(event);
}

void WFeatureClickButton::dropEvent(QDropEvent* event) {
    
}

void WFeatureClickButton::slotClicked() {
    emit(clicked(m_data));
}

