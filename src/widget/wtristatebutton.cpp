#include "wtristatebutton.h"


WTriStateButton::WTriStateButton(QWidget* parent)
        : QToolButton(parent),
          m_state(State::Unactive) {
    setCheckable(true);
}

void WTriStateButton::setChecked(bool value) {
    m_state = value ? State::Active : State::Unactive;
    updateButton();
}

bool WTriStateButton::isChecked() const {
    return m_state == State::Active;
}

void WTriStateButton::setState(State state) {
    m_state = state;
    updateButton();
}

WTriStateButton::State WTriStateButton::getState() const {
    return m_state;
}

void WTriStateButton::setHovered(bool value) {
    m_state = value ? State::Hovered : State::Unactive;
    updateButton();
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
            break;
        case State::Active:
            pix = m_icon.pixmap(height(), QIcon::Normal, QIcon::On);
            break;
        case State::Hovered:
            pix = m_icon.pixmap(height(), QIcon::Active, QIcon::Off);
            break;
    }
    QToolButton::setIcon(QIcon(pix));
}
