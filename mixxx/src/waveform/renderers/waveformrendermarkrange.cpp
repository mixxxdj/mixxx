#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveform/renderers/waveformrendermarkrange.h"

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderMarkRange::WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer)
{
}

WaveformRenderMarkRange::~WaveformRenderMarkRange() {
}

void WaveformRenderMarkRange::init() {
}

void WaveformRenderMarkRange::setup(const QDomNode &node) {
    markRanges_.clear();
    markRanges_.reserve(32);

    QDomNode child = node.firstChild();
    while (!child.isNull())
    {
        if (child.nodeName() == "MarkRange")
        {
            markRanges_.push_back(WaveformMarkRange());
            setupMarkRange(child,markRanges_.back());
        }
        child = child.nextSibling();
    }
}

void WaveformRenderMarkRange::draw(QPainter *painter, QPaintEvent * /*event*/) {
    painter->save();

    painter->setWorldMatrixEnabled(false);

    if (isDirty())
        generatePixmaps();

    for (int i = 0; i < markRanges_.size(); i++) {
        WaveformMarkRange& markRange = markRanges_[i];

        if (!markRange.isValid())
            continue;

        int startSample = markRange.m_markStartPoint->get();
        int endSample = markRange.m_markEndPoint->get();
        if (startSample < 0 || endSample < 0)
            continue;

        m_waveformRenderer->regulateVisualSample(startSample);
        double startPosition = m_waveformRenderer->transformSampleIndexInRendererWorld(startSample);

        m_waveformRenderer->regulateVisualSample(endSample);
        double endPosition = m_waveformRenderer->transformSampleIndexInRendererWorld(endSample);

        //range not in the current display
        if (startPosition > m_waveformRenderer->getWidth() ||
                endPosition < 0)
            continue;

        QPixmap* selectedPixmap = 0;

        if (markRange.m_markEnabled && markRange.m_markEnabled->get() < 0.5)
            selectedPixmap = &markRange.m_disabledPixmap;
        else
            selectedPixmap = &markRange.m_activePixmap;

        //draw the correcponding portion of the selected pixmap
        //this shouldn't involve *any* scaling it should be fast even in software mode
        QRect rect(startPosition,0,endPosition-startPosition,m_waveformRenderer->getHeight());
        painter->drawPixmap(rect, *selectedPixmap, rect);
    }

    painter->restore();
}

void WaveformRenderMarkRange::setupMarkRange(const QDomNode &node, WaveformMarkRange &markRange) {
    markRange.m_activeColor = WWidget::selectNodeQString(node, "Color");
    if (markRange.m_activeColor == "") {
        //vRince kind of legacy fallback ...
        // As a fallback, grab the mark color from the parent's MarkerColor
        markRange.m_activeColor = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:" << markRange.m_activeColor;
    }

    markRange.m_disabledColor = WWidget::selectNodeQString(node, "DisabledColor");
    if (markRange.m_disabledColor == "") {
        //vRince kind of legacy fallback ...
        // Read the text color, otherwise use the parent's SignalColor.
        markRange.m_disabledColor = WWidget::selectNodeQString(node.parentNode(), "SignalColor");
        qDebug() << "Didn't get mark TextColor, using parent's SignalColor:" << markRange.m_disabledColor;
    }

    markRange.m_markStartPoint = ControlObject::getControl(
                ConfigKey(m_waveformRenderer->getGroup(),
                          WWidget::selectNodeQString(node, "StartControl")));
    markRange.m_markEndPoint = ControlObject::getControl(
                ConfigKey(m_waveformRenderer->getGroup(),
                          WWidget::selectNodeQString(node, "EndControl")));
    markRange.m_markEnabled = ControlObject::getControl(
                ConfigKey(m_waveformRenderer->getGroup(),
                          WWidget::selectNodeQString(node, "EnabledControl")));
}

void WaveformRenderMarkRange::generatePixmaps() {
    for (int i = 0; i < markRanges_.size(); i++)
        markRanges_[i].generatePixmap(m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
    setDirty(false);
}
