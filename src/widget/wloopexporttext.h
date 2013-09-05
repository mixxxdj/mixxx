#ifndef WLOOPEXPORTTEXT_H
#define WLOOPEXPORTTEXT_H

#include "widget/wnumber.h"

class WLoopExportText : public WNumber {
    Q_OBJECT
public:
    WLoopExportText(QWidget *pParent=0);
    virtual ~WLoopExportText();
    void setValue(double dValue);
};
#endif
