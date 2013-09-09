#include "widget/wloopsourcetext.h"
//#include "looprecording/defs_looprecording.h"

WLoopSourceText::WLoopSourceText(QWidget *pParent) : WNumber(pParent) { }

WLoopSourceText::~WLoopSourceText() { }

void WLoopSourceText::setValue(double dValue) {
    QString text = "";
//
//    // TODO: switch statement here?
//    if (dValue == INPUT_NONE) {
//        text = "off";
//    } else if (dValue == INPUT_MASTER) {
//        text = "master";
//    } else if (dValue == INPUT_HEAD) {
//        text = "headphones";
//    } else if (dValue == INPUT_MICROPHONE) {
//        text = "mic";
//    } else if (dValue == INPUT_PT1) {
//        text = "pt1";
//    } else if (dValue == INPUT_PT2) {
//        text = "pt2";
//    } else if (dValue > INPUT_DECK_BASE && dValue < INPUT_SAMPLER_BASE) {
//        text = QString("deck  %1").arg(QString::number(dValue-INPUT_DECK_BASE));
//    } else if (dValue > INPUT_SAMPLER_BASE) {
//        text = QString("sampler %1").arg(QString::number(dValue-INPUT_SAMPLER_BASE));
//    } else {
//        text = "invalid";
//    }
    m_pLabel->setText(QString(m_qsText).append(text));
}
