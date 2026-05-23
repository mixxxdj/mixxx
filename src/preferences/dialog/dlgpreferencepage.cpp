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
    // This ensures that scrollable input widgets react on wheel events only if
    // they have focus. This avoid unintended value changes when scrolling the
    // preferences pages.
    // This works by setting the focus policy to Qt::StrongFocus and installing
    // our custom event filter on them.
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
    // Note: finding all relevant widgets with pObj->findchildren<Type*>
    // is much faster than with
    // for (auto* ch : pObj->children()) { qobject_cast<Type*>(ch); }
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QApplication::sendEvent(layout()->parent(), pEvent);
#else
            QApplication::sendEvent(qobject_cast<QObject*>(layout()), pEvent);
#endif
            return true;
        }
    }
    return QObject::eventFilter(pObj, pEvent);
}
