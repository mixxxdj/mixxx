#ifndef WKEY_H
#define WKEY_H

#include <QLabel>

#include "widget/wlabel.h"
#include "controlobjectslave.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    WKey(const char* group, QWidget* pParent=NULL);
    virtual ~WKey();

    virtual void onConnectedControlChanged(double dParameter, double dValue);
    void setup(QDomNode node, const SkinContext& context);

  private slots:
    void setValue(double dValue);
    void preferencesUpdated(double dValue);
    void setCents();

  private:
    double m_dOldValue;
    bool m_displayCents;
    ControlObjectSlave m_preferencesUpdated;
    ControlObjectSlave m_engineKeyDistance;
};

#endif /* WKEY_H */
