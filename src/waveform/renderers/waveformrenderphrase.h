#pragma once

#include "skin/legacy/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/phrasecolors.h"
#include "waveform/renderers/waveformrendererabstract.h"

/// Legacy (QPainter) renderer that paints the track's phrases as coloured
/// semi-transparent background segments behind the waveform. The allshader
/// equivalent lives in allshader/waveformrenderphrase.{h,cpp}.
class WaveformRenderPhrase : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderPhrase(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderPhrase() override = default;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    mixxx::PhraseColors m_colors{mixxx::defaultPhraseColors()};

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderPhrase);
};
