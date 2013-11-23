#ifndef WAVEFORMRENDERBACKGROUND_H
#define WAVEFORMRENDERBACKGROUND_H

#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "util.h"
#include "waveformrendererabstract.h"

class WaveformWidgetRenderer;

class WaveformRenderBackground : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBackground();

    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generateImage();

    QString m_backgroundPixmapPath;
    QColor m_backgroundColor;
    QImage m_backgroundImage;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};

#endif /* WAVEFORMRENDERBACKGROUND_H */
