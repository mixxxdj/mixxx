#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"

#include <QBrush>

#include <vector>

class ControlObject;

class GLWaveformRendererFilteredSignal : public WaveformRendererAbstract {
  public:
    explicit GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererFilteredSignal();

    virtual void init();
    virtual void setup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  protected:
    virtual void onResize();
    int buildPolygon();

  protected:
    ControlObject* m_lowFilterControlObject;
    ControlObject* m_midFilterControlObject;
    ControlObject* m_highFilterControlObject;

    ControlObject* m_lowKillControlObject;
    ControlObject* m_midKillControlObject;
    ControlObject* m_highKillControlObject;

    WaveformSignalColors m_colors;
    Qt::Alignment m_alignment;

    QBrush m_lowBrush;
    QBrush m_midBrush;
    QBrush m_highBrush;
    QBrush m_lowKilledBrush;
    QBrush m_midKilledBrush;
    QBrush m_highKilledBrush;

    std::vector<QPointF> m_polygon[3];
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
