#pragma once

#include <QObject>
#include <QColor>
#include <QDomNode>
#include <QPainter>
#include <QPaintEvent>

#include <vector>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ConfigKey;
class ControlObject;

class WaveformRenderMarkRange : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderMarkRange() override = default;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    void generateImages();

    std::vector<WaveformMarkRange> m_markRanges;
};
