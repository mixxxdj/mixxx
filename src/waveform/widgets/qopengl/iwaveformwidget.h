#pragma once

namespace qopengl {
class IWaveformWidget;
}

class qopengl::IWaveformWidget {
  public:
    virtual void renderGL() = 0;
};
