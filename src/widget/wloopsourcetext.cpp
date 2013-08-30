#include "widget/wloopsourcetext.h"

WLoopSourceText::WLoopSourceText : WNumber(pParent) {
    m_pRateControl = new ControlObjectThreadMain(group, "loop_source");
    connect(m_pRateControl, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)));
    // Initialize the widget.
    setValue(0);
}

WLoopSourceText::~WLoopSourceText() {

}

void WLoopSourceText::slotSourceChanged() {
    updateLabel();
}

void WLoopSourceText::updateLabel() {
        m_pLabel->setText("");
}
