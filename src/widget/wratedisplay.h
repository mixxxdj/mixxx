#pragma once

#include "widget/wnumber.h"

class ControlProxy;

enum class VerticalPosition {
    Top,
    Bottom
};

class WRateDisplay : public WNumber {
    Q_OBJECT
  public:
    explicit WRateDisplay(const QString& group, QWidget* parent = nullptr);
    
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue();

  private:
    ControlProxy* m_pRateRangeControl;
    ControlProxy* m_pRateDirControl;
    VerticalPosition m_nodePosition;
};
