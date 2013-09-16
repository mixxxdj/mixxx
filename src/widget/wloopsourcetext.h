#ifndef WLOOPSOURCETEXT_H
#define WLOOPSOURCETEXT_H

#include "widget/wlabel.h"

class WLoopSourceText : public WLabel {
    Q_OBJECT
  public:
    WLoopSourceText(QWidget *pParent=0);
    virtual ~WLoopSourceText();
    void setup(QDomNode node);
    QString getProperty();

  public slots:
    void slotUpdateLabel(QString text);

  private:
    QString m_property;
};
#endif
