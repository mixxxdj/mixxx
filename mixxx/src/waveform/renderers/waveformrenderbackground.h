#ifndef WAVEFORMRENDERBACKGROUND_H
#define WAVEFORMRENDERBACKGROUND_H

#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>

#include "util.h"
#include "waveformrendererabstract.h"

class WaveformWidgetRenderer;

class WaveformRenderBackground : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBackground();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generatePixmap();

    QString m_backgroundPixmapPath;
    QColor m_backgroundColor;
    QPixmap m_backgroundPixmap;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};

#endif /* WAVEFORMRENDERBACKGROUND_H */
