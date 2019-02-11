#ifndef WAVEFORMRENDERBEAT_H
#define WAVEFORMRENDERBEAT_H

#include <QColor>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "preferences/usersettings.h"
#include "control/controlproxy.h"

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBeat();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_beatColor;
    QColor m_barColor;
    QColor m_phraseColor;
    QVector<QLineF> m_beats;
    ControlProxy* m_showBarAndPhrase;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};

#endif //WAVEFORMRENDERBEAT_H
