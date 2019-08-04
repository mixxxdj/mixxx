#ifndef WKEY_H
#define WKEY_H

#include <QLabel>

#include "widget/wlabel.h"
#include "control/controlproxy.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    explicit WKey(const char* group, QWidget* pParent=nullptr);

    void onConnectedControlChanged(double dParameter, double dValue) override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue);
    void preferencesUpdated(double dValue);
    void setCents();

  private:
    double m_dOldValue;
    bool m_displayCents;
    bool m_displayKey;
    ControlProxy m_preferencesUpdated;
    ControlProxy m_engineKeyDistance;
};

#endif /* WKEY_H */
