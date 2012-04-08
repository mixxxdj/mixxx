#ifndef WAVEFORMRENDERMARKRANGE_H
#define WAVEFORMRENDERMARKRANGE_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QPixmap>
#include <QDomNode>
#include <QPainter>
#include <QPaintEvent>

#include "configobject.h"
#include "util.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ConfigKey;
class ControlObjectThreadMain;
class ControlObject;

class MarkRange {
  public:
    MarkRange();
    virtual ~MarkRange();

    bool isValid() const { return m_markStartPoint && m_markEndPoint;}
    bool isActive() const;

  private:
    void generatePixmap(int weidth, int height);

    ControlObject* m_markStartPoint;
    ControlObject* m_markEndPoint;
    ControlObject* m_markEnabled;

    QColor m_activeColor;
    QColor m_disabledColor;

    QPixmap m_activePixmap;
    QPixmap m_disabledPixmap;

    friend class WaveformRenderMarkRange;
};

class WaveformRenderMarkRange : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderMarkRange();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void setupMarkRange(const QDomNode& node, MarkRange& markRange);
    void generatePixmaps();

    QVector<MarkRange> markRanges_;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};

#endif
