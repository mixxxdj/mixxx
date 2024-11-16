#include "effects/dlgeffect.h"

#include <QWidget>

#include "moc_dlgeffect.cpp"

DlgEffect::DlgEffect(QWidget* customUI)
        : m_closesWithoutSignal(false) {
    setWindowTitle("Effect");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setCustomUI(customUI);
}

DlgEffect::~DlgEffect() {
}

void DlgEffect::setCustomUI(QWidget* customUI) {
    if (customUI != nullptr) {
        customUI->setParent(this);
    }
    m_customUI = customUI;
}

void DlgEffect::setClosesWithoutSignal(bool closesWithoutSignal) {
    m_closesWithoutSignal = closesWithoutSignal;
}

void DlgEffect::closeEvent(QCloseEvent* e) {
    Q_UNUSED(e);
    if (!m_closesWithoutSignal) {
        emit closed();
    }
}
