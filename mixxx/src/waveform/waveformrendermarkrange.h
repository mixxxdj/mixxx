// waveformrendermarkrange.h
// Created 11/14/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WAVEFORMRENDERMARKRANGE_H
#define WAVEFORMRENDERMARKRANGE_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QPixmap>

#include "configobject.h"
#include "waveform/renderobject.h"

#include "waveformrendererabstract.h"

class QDomNode;
class QPainter;
class QPaintEvent;
class ConfigKey;
class ControlObjectThreadMain;
class WaveformRenderer;

class MarkRange
{
public:
    MarkRange();
    bool isValid() const { return m_markStartPoint && m_markEndPoint;}
    bool isActive() const;

private:
    void generatePixmap( int weidth, int height);

private:
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
    WaveformRenderMarkRange( WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

private:
    void setupMarkRange( const QDomNode& node, MarkRange& markRange);
    void generatePixmaps();

private:
    QVector<MarkRange> markRanges_;
};

#endif
