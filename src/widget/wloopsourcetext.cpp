#include "widget/wloopsourcetext.h"

WLoopSourceText::WLoopSourceText(QWidget *pParent)
        : WLabel(pParent),
        m_property("") { }

WLoopSourceText::~WLoopSourceText() { }

void WLoopSourceText::setup(QDomNode node) {
    WLabel::setup(node);

    m_property = selectNodeQString(node, "Property");
}

QString WLoopSourceText::getProperty() {
    return m_property;
}

void WLoopSourceText::slotUpdateLabel(QString text) {
    m_pLabel->setText(text);
}
