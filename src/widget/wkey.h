#ifndef WKEY_H
#define WKEY_H

#include <QLabel>

#include "widget/wlabel.h"
#include "controlobjectthread.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    WKey(QWidget* pParent=NULL);
    virtual ~WKey();

    virtual void onConnectedControlChanged(double dParameter, double dValue);

  private slots:
    void setValue(double dValue);
    void preferencesUpdated(double dValue);

  private:
    double m_dOldValue;
    ControlObjectThread m_preferencesUpdated;
};

#endif /* WKEY_H */
