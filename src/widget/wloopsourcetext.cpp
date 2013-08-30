#include "widget/wloopsourcetext.h"
#include "looprecording/defs_looprecording.h"

WLoopSourceText::WLoopSourceText(QWidget *pParent) : WNumber(pParent) {

}

WLoopSourceText::~WLoopSourceText() {

}


void WLoopSourceText::setValue(double dValue) {
    QString text = "";
    if (dValue == INPUT_NONE) {
        text = "Off";
    } else {
        text = "On";
    }
    m_pLabel->setText(QString(m_qsText).append(text));
}
