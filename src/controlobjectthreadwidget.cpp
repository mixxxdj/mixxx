#include <QtDebug>
#include <QApplication>

#include "controlobjectthreadwidget.h"
#include "controlevent.h"
#include "control/control.h"

ControlObjectThreadWidget::ControlObjectThreadWidget(const ConfigKey& key, QObject* pParent)
        : ControlObjectThreadMain(key, pParent) {
}

ControlObjectThreadWidget::ControlObjectThreadWidget(const char* g, const char* i, QObject* pParent)
        : ControlObjectThreadMain(g, i, pParent) {
}

ControlObjectThreadWidget::ControlObjectThreadWidget(const QString& g, const QString& i, QObject* pParent)
        : ControlObjectThreadMain(g, i, pParent) {
}

ControlObjectThreadWidget::~ControlObjectThreadWidget() {
}

double ControlObjectThreadWidget::get() {
    return m_pControl ? m_pControl->getParameter() : 0.0;
}

void ControlObjectThreadWidget::set(double v) {
    if (m_pControl) {
        m_pControl->setParameter(v, this);
    }
}
