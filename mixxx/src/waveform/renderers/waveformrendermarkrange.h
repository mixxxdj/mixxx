#ifndef WAVEFORMRENDERMARKRANGE_H
#define WAVEFORMRENDERMARKRANGE_H

#include <QObject>
#include <QColor>
#include <QPixmap>
#include <QDomNode>
#include <QPainter>
#include <QPaintEvent>

#include <vector>

#include "configobject.h"
#include "util.h"

#include "waveformrendererabstract.h"
#include "waveformmarkrange.h"

class ConfigKey;
class ControlObjectThreadMain;
class ControlObject;

class WaveformRenderMarkRange : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderMarkRange();

    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generateImages();

    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};

#endif
