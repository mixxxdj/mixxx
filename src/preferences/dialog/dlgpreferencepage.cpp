#include "preferences/dialog/dlgpreferencepage.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLayout>
#include <QSlider>
#include <QSpinBox>

#include "moc_dlgpreferencepage.cpp"

DlgPreferencePage::DlgPreferencePage(QWidget* pParent)
        : QWidget(pParent) {
}

DlgPreferencePage::~DlgPreferencePage() {
}

QUrl DlgPreferencePage::helpUrl() const {
    return QUrl();
}

void DlgPreferencePage::setScrollSafeGuardForAllInputWidgets(QObject* pObj) {
    // Set the focus policy to for scrollable input widgets and connect them
    // to the custom event filter.
    // Note: finding all relevant widgets with pObj->findchildren<Type*>
    // is much faster than with
    // for (auto* ch : pObj->children()) { qobject_cast<Type*>(ch); }
    setScrollSafeGuardForChildrenOfType<QComboBox>(pObj);
    setScrollSafeGuardForChildrenOfType<QSpinBox>(pObj);
    setScrollSafeGuardForChildrenOfType<QDoubleSpinBox>(pObj);
    setScrollSafeGuardForChildrenOfType<QSlider>(pObj);
}

void DlgPreferencePage::setScrollSafeGuard(QWidget* pWidget) {
    pWidget->setFocusPolicy(Qt::StrongFocus);
    pWidget->installEventFilter(this);
}

template<typename T>
void DlgPreferencePage::setScrollSafeGuardForChildrenOfType(QObject* pObj) {
    QList<T*> children = pObj->findChildren<T*>();
    for (T* pChild : children) {
        setScrollSafeGuard(pChild);
    }
}

bool DlgPreferencePage::eventFilter(QObject* pObj, QEvent* pEvent) {
    if (pEvent->type() == QEvent::Wheel) {
        // Reject scrolling if widget is not focused.
        // Object to widget cast is needed to check the focus state.
        QComboBox* combo = qobject_cast<QComboBox*>(pObj);
        QSpinBox* spin = qobject_cast<QSpinBox*>(pObj);
        QDoubleSpinBox* spinDbl = qobject_cast<QDoubleSpinBox*>(pObj);
        QSlider* slider = qobject_cast<QSlider*>(pObj);
        if ((combo && !combo->hasFocus()) ||
                (spin && !spin->hasFocus()) ||
                (spinDbl && !spinDbl->hasFocus()) ||
                (slider && !slider->hasFocus())) {
            QApplication::sendEvent(layout()->parent(), pEvent);
            return true;
        }
    }
    return QObject::eventFilter(pObj, pEvent);
}
