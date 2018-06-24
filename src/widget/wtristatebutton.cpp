#include "wtristatebutton.h"
#include <QDebug>

WTriStateButton::WTriStateButton(QWidget* parent)
        : QToolButton(parent),
          m_state(State::Unactive) {
    setCheckable(true);
}

void WTriStateButton::setChecked(bool value) {
    //qDebug() << this << "setChecked" << value;
    // This omits the Hovered state
    State state = value ? State::Active : State::Unactive;
    setState(state);
}

bool WTriStateButton::isChecked() const {
    return m_state == State::Active;
}

void WTriStateButton::setState(State state) {
    if (state == m_state) return;
    m_state = state;
    updateButton();
}

WTriStateButton::State WTriStateButton::getState() const {
    return m_state;
}

void WTriStateButton::setHovered(bool value) {
    //qDebug() << this << "setHovered" << value;
    State state = value ? State::Hovered : State::Unactive;
    setState(state);
}

void WTriStateButton::setIcon(const QIcon& icon) {
    m_icon = icon;
    updateButton();
}

void WTriStateButton::updateButton() {
    QPixmap pix;
    switch (m_state) {
        case State::Unactive:
            pix = m_icon.pixmap(height(), QIcon::Normal, QIcon::Off);
            QToolButton::setChecked(false);
            break;
        case State::Active:
            pix = m_icon.pixmap(height(), QIcon::Normal, QIcon::On);
            QToolButton::setChecked(true);
            break;
        case State::Hovered:
            pix = m_icon.pixmap(height(), QIcon::Active, QIcon::Off);
            QToolButton::setChecked(false);
            break;
    }

    QToolButton::setIcon(QIcon(pix));
}
