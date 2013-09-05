#include "widget/wloopexporttext.h"

WLoopExportText::WLoopExportText(QWidget *pParent) : WNumber(pParent) { }

WLoopExportText::~WLoopExportText() { }

void WLoopExportText::setValue(double dValue) {
    m_pLabel->setText(QString(m_qsText).append("Sampler %1").arg(QString::number(dValue)));
}
