#pragma once

#include "control/controlproxy.h"

class FaderStartControl {
  public:
    explicit FaderStartControl(const QString& group);
    void process();

  private:
    ControlProxy m_play;
    ControlProxy m_volume;
};
