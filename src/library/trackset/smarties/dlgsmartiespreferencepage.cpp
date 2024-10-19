#include "DlgSmartiesPreferencePage.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLayout>
#include <QSlider>
#include <QSpinBox>

#include "moc_DlgSmartiesPreferencePage.cpp"

DlgSmartiesPreferencePage::DlgSmartiesPreferencePage(QWidget* pParent)
        : QWidget(pParent) {
}

DlgSmartiesPreferencePage::~DlgSmartiesPreferencePage() {
}

QUrl DlgSmartiesPreferencePage::helpUrl() const {
    return QUrl();
}

void DlgSmartiesPreferencePage::setScrollSafeGuardForAllInputWidgets(QObject* obj) {
    // Set the focus policy to for scrollable input widgets and connect them
    // to the custom event filter
    for (auto* ch : obj->children()) {
        // children() does not descend into QGroupBox,
        // so we need to do it manually
        QGroupBox* gBox = qobject_cast<QGroupBox*>(ch);
        if (gBox) {
            setScrollSafeGuardForAllInputWidgets(gBox);
            continue;
        }

        QComboBox* combo = qobject_cast<QComboBox*>(ch);
        if (combo) {
            setScrollSafeGuard(combo);
            continue;
        }
        QSpinBox* spin = qobject_cast<QSpinBox*>(ch);
        if (spin) {
            setScrollSafeGuard(spin);
            continue;
        }
        QDoubleSpinBox* spinDouble = qobject_cast<QDoubleSpinBox*>(ch);
        if (spinDouble) {
            setScrollSafeGuard(spinDouble);
            continue;
        }
        QSlider* slider = qobject_cast<QSlider*>(ch);
        if (slider) {
            setScrollSafeGuard(slider);
            continue;
        }
    }
}

void DlgSmartiesPreferencePage::setScrollSafeGuard(QWidget* pWidget) {
    pWidget->setFocusPolicy(Qt::StrongFocus);
    pWidget->installEventFilter(this);
}

bool DlgSmartiesPreferencePage::eventFilter(QObject* obj, QEvent* e) {
    if (e->type() == QEvent::Wheel) {
        // Reject scrolling only if widget is unfocused.
        // Object to widget cast is needed to check the focus state.
        QComboBox* combo = qobject_cast<QComboBox*>(obj);
        QSpinBox* spin = qobject_cast<QSpinBox*>(obj);
        QDoubleSpinBox* spinDbl = qobject_cast<QDoubleSpinBox*>(obj);
        QSlider* slider = qobject_cast<QSlider*>(obj);
        if ((combo && !combo->hasFocus()) ||
                (spin && !spin->hasFocus()) ||
                (spinDbl && !spinDbl->hasFocus()) ||
                (slider && !slider->hasFocus())) {
            QApplication::sendEvent(qobject_cast<QObject*>(layout()), e);
            // QApplication::sendEvent(layout()->parent(), e); ??
            return true;
        }
    }
    return QObject::eventFilter(obj, e);
}
