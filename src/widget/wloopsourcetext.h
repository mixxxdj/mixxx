#ifndef WLOOPSOURCETEXT_H
#define WLOOPSOURCETEXT_H

#include "widget/wlabel.h"

class WLoopSourceText : public WLabel {
    Q_OBJECT
  public:
    WLoopSourceText(QWidget *parent);
    virtual ~WLoopSourceText();

  public slots:
    void slotSourceChanged()

  private slots:
    void updateLabel();
};

#endif
