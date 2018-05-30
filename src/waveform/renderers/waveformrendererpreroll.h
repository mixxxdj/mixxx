#ifndef WAVEFORMRENDERPREROLL_H
#define WAVEFORMRENDERPREROLL_H

#include <QColor>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRendererPreroll : public WaveformRendererAbstract {
  public:
    explicit WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererPreroll();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_color;
    
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};

#endif /* WAVEFORMRENDERPREROLL_H */
