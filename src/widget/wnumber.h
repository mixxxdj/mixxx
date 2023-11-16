#pragma once

#include "widget/wlabel.h"

class WNumber : public WLabel  {
    Q_OBJECT
  public:
    explicit WNumber(QWidget* pParent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

    void onConnectedControlChanged(double dParameter, double dValue) override;

  public slots:
    virtual void setValue(double dValue);

  protected:
    // Number of digits to round to.
    int m_iNoDigits;
};
