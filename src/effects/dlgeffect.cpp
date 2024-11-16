#include "effects/dlgeffect.h"

#include <QWidget>

#include "moc_dlgeffect.cpp"

DlgEffect::DlgEffect(QWidget* customUI) {
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
