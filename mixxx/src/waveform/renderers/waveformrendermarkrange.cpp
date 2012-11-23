#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveform/renderers/waveformrendermarkrange.h"

#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderMarkRange::WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRenderMarkRange::~WaveformRenderMarkRange() {
}

void WaveformRenderMarkRange::setup(const QDomNode &node) {
    m_markRanges.clear();
    m_markRanges.reserve(1);

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "MarkRange") {
            m_markRanges.push_back(WaveformMarkRange());
            m_markRanges.back().setup( m_waveformRenderer->getGroup(), child);
        }
        child = child.nextSibling();
    }
}

void WaveformRenderMarkRange::draw(QPainter *painter, QPaintEvent * /*event*/) {
    painter->save();

    painter->setWorldMatrixEnabled(false);

    if (isDirty())
        generatePixmaps();

    for (unsigned int i = 0; i < m_markRanges.size(); i++) {
        WaveformMarkRange& markRange = m_markRanges[i];

        // If the mark range is not active we should not draw it.
        if (!markRange.active()) {
            continue;
        }

        // Active mark ranges by definition have starts/ends that are not
        // disabled so no need to check.
        int startSample = markRange.start();
        int endSample = markRange.end();

        m_waveformRenderer->regulateVisualSample(startSample);
        double startPosition = m_waveformRenderer->transformSampleIndexInRendererWorld(startSample);

        m_waveformRenderer->regulateVisualSample(endSample);
        double endPosition = m_waveformRenderer->transformSampleIndexInRendererWorld(endSample);

        //range not in the current display
        if (startPosition > m_waveformRenderer->getWidth() || endPosition < 0)
            continue;

        QPixmap* selectedPixmap = NULL;

        selectedPixmap = markRange.enabled() ? &markRange.m_activePixmap : &markRange.m_disabledPixmap;

        //draw the correcponding portion of the selected pixmap
        //this shouldn't involve *any* scaling it should be fast even in software mode
        QRect rect(startPosition,0,endPosition-startPosition,m_waveformRenderer->getHeight());
        painter->drawPixmap(rect, *selectedPixmap, rect);
    }

    painter->restore();
}

void WaveformRenderMarkRange::generatePixmaps() {
    for (unsigned int i = 0; i < m_markRanges.size(); i++) {
        m_markRanges[i].generatePixmap(m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
    }
    setDirty(false);
}
