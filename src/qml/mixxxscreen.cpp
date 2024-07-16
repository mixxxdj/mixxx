#include "mixxxscreen.h"

namespace mixxx {
namespace qml {

int MixxxScreen::width() {
    return m_size.width();
}

void MixxxScreen::setWidth(int value) {
    m_size = QSize(value, m_size.height());
}

int MixxxScreen::height() {
    return m_size.width();
}

void MixxxScreen::setHeight(int value) {
    m_size = QSize(m_size.width(), value);
}

uint MixxxScreen::splashOff() {
    return m_splashOff.count();
}

void MixxxScreen::setSplashOff(uint value) {
    m_splashOff = std::chrono::milliseconds(value);
}

} // namespace qml
} // namespace mixxx
