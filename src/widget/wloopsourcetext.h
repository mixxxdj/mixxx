#ifndef WLOOPSOURCETEXT_H
#define WLOOPSOURCETEXT_H

#include "widget/wnumber.h"

class WLoopSourceText : public WNumber {
    Q_OBJECT
public:
    WLoopSourceText(QWidget *pParent=0);
    virtual ~WLoopSourceText();
    void setValue(double dValue);
};
#endif
