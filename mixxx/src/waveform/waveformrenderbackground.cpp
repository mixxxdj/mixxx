
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>
#include <QLine>
#include <QPixmap>

#include "waveformrenderbackground.h"
#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderBackground::WaveformRenderBackground(const char* group, WaveformRenderer *parent)
        : m_iWidth(0),
          m_iHeight(0),
          m_backgroundPixmap(),
          m_bRepaintBackground(true),
          bgColor(0,0,0) {
}

WaveformRenderBackground::~WaveformRenderBackground() {
    qDebug() << this << "~WaveformRenderBackground()";
}

void WaveformRenderBackground::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
    // Need to repaint the background if we've been resized.
    m_bRepaintBackground = true;
}

void WaveformRenderBackground::newTrack(TrackPointer pTrack) {
}

void WaveformRenderBackground::setup(QDomNode node) {
    bgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    bgColor = WSkinColor::getCorrectColor(bgColor);
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    m_bRepaintBackground = true;
}

void WaveformRenderBackground::draw(QPainter *pPainter, QPaintEvent *pEvent,
                                    QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(m_bRepaintBackground) {
        generatePixmap();
    }

    pPainter->fillRect(pEvent->rect(), bgColor);

    // Paint the background pixmap if it exists.
    if (!m_backgroundPixmap.isNull()) {
        pPainter->drawTiledPixmap(pEvent->rect(), m_backgroundPixmap, QPoint(0,0));
    }
}

void WaveformRenderBackground::generatePixmap() {
    if (m_backgroundPixmapPath != "") {
        m_backgroundPixmap = QPixmap(WWidget::getPath(m_backgroundPixmapPath));
    } else {
        m_backgroundPixmap = QPixmap();
    }
    m_bRepaintBackground = false;
}
