#ifndef WKEY_H
#define WKEY_H

#include <QLabel>

#include "widget/wlabel.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    WKey(QWidget* pParent=NULL);
    virtual ~WKey();

  private slots:
    void setValue(double dValue);
};

#endif /* WKEY_H */
