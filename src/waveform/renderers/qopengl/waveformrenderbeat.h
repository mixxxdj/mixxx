#pragma once

#include <QColor>

#include "util/class.h"
#include "waveform/renderers/qopengl/waveformshaderrenderer.h"

class QDomNode;
class SkinContext;

namespace qopengl {
class WaveformRenderBeat;
}

class qopengl::WaveformRenderBeat : public qopengl::WaveformShaderRenderer {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRenderBeat() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void renderGL() override;
    void initializeGL() override;

  private:
    QColor m_beatColor;
    QVector<QVector2D> m_vertices;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
