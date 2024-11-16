#include "effects/dlgeffect.h"

#include "moc_dlgeffect.cpp"

DlgEffect::DlgEffect(QWidget* customUI) {
    setCustomUI(customUI);
}

void DlgEffect::setCustomUI(QWidget* customUI) {
    if (customUI != nullptr) {
        customUI->setParent(this);
    }
    m_customUI = customUI;
}
