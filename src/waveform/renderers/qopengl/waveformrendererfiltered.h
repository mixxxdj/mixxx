#pragma once

#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererFiltered;
}

class qopengl::WaveformRendererFiltered : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererFiltered(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererFiltered() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    QVector<float> m_lineValuesForGroup[4];

    void addRectangleToGroup(float x1, float y1, float x2, float y2, int group);

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererFiltered);
};
