#include "widget/wloopsourcetext.h"

WLoopSourceText::WLoopSourceText(QWidget* pParent)
        : WLabel(pParent) {

}

WLoopSourceText::~WLoopSourceText() {

}

void WLoopSourceText::slotSourceChanged() {
    updateLabel();
}

void WLoopSourceText::updateLabel() {
        m_pLabel->setText("");
}
