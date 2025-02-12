#pragma once

#include <qboxlayout.h>

#include "control/controlproxy.h"
#include "widget/wlabel.h"
#include "widget/wwidgetgroup.h"

class WKey : public WWidgetGroup {
    Q_OBJECT
  public:
    explicit WKey(const QString& group, QWidget* pParent = nullptr);

    void onConnectedControlChanged(double dParameter, double dValue) override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue);
    void keyNotationChanged(double dValue);
    void setCents();

  private:
    double m_dOldValue;
    bool m_displayCents;
    bool m_displayKey;
    ControlProxy m_keyNotation;
    ControlProxy m_engineKeyDistance;
    WLabel m_keyLabel;
};
