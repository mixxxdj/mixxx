#pragma once

#include "widget/wvumeterbase.h"

class WVuMeter : public WVuMeterBase {
    Q_OBJECT
  public:
    explicit WVuMeter(QWidget* parent = nullptr);

    void draw() override;
};
