#pragma once

#include "widget/wvumeterbase.h"

class WVuMeter : public WVuMeterBase {
  public:
    explicit WVuMeter(QWidget* parent = nullptr);

    void draw() override;
};
