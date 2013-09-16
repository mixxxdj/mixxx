#ifndef WLOOPTEXT_H
#define WLOOPTEXT_H

#include "widget/wlabel.h"

class WLoopText : public WLabel {
    Q_OBJECT
  public:
    WLoopText(QWidget *pParent=0);
    virtual ~WLoopText();
    void setup(QDomNode node);
    QString getProperty();

  public slots:
    void slotUpdateLabel(QString text);

  private:
    QString m_property;
};
#endif
