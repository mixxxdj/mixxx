#include "widget/wlooptext.h"

WLoopText::WLoopText(QWidget *pParent)
        : WLabel(pParent),
        m_property("") { }

WLoopText::~WLoopText() { }

void WLoopText::setup(QDomNode node) {
    WLabel::setup(node);

    m_property = selectNodeQString(node, "Property");
}

QString WLoopText::getProperty() {
    return m_property;
}

void WLoopText::slotUpdateLabel(QString text) {
    m_pLabel->setText(text);
}
