#ifndef WAVEFORMRENDERMARKRANGE_H
#define WAVEFORMRENDERMARKRANGE_H

#include <QObject>
#include <QColor>
#include <QDomNode>
#include <QPainter>
#include <QPaintEvent>

#include <vector>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ConfigKey;
class ControlObject;

class WaveformRenderMarkRange : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderMarkRange();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generateImages();

    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};

#endif
