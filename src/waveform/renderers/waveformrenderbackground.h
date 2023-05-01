#pragma once

#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "skin/legacy/skincontext.h"
#include "util/class.h"
#include "waveformrendererabstract.h"

class WaveformWidgetRenderer;

class WaveformRenderBackground : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBackground();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  protected:
    bool drawImage(QPainter* painter);

    QColor m_backgroundColor;

  private:
    void generateImage();

    QString m_backgroundPixmapPath;
    QImage m_backgroundImage;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};
