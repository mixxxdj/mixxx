
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

WaveformRenderBackground::WaveformRenderBackground(const char* group, WaveformRenderer *parent) :
    m_iWidth(0),
    m_iHeight(0),
    m_backgroundPixmap(),
    m_bRepaintBackground(true),
    bgColor(0,0,0)
{
}

void WaveformRenderBackground::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
    // Need to repaint the background if we've been resized.
    m_backgroundPixmap = QPixmap(w,h);
    m_bRepaintBackground = true;
}

void WaveformRenderBackground::newTrack(TrackPointer pTrack) {
}

void WaveformRenderBackground::setup(QDomNode node) {
    bgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    bgColor = WSkinColor::getCorrectColor(bgColor);
}

void WaveformRenderBackground::draw(QPainter *pPainter, QPaintEvent *pEvent, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(m_bRepaintBackground) {
        generatePixmap();
    }

    // Paint the background
    pPainter->drawPixmap(m_backgroundPixmap.rect(), m_backgroundPixmap, pEvent->rect());
}

void WaveformRenderBackground::generatePixmap() {
    QLinearGradient linearGrad(QPointF(0,0), QPointF(0,m_iHeight));
    linearGrad.setColorAt(0.0, bgColor);
    linearGrad.setColorAt(0.5, bgColor.light(180));
    linearGrad.setColorAt(1.0, bgColor);

    // linearGrad.setColorAt(0.0, Qt::black);
    // linearGrad.setColorAt(0.3, bgColor);
    // linearGrad.setColorAt(0.7, bgColor);
    // linearGrad.setColorAt(1.0, Qt::black);
    QBrush brush(linearGrad);

    QPainter newPainter;
    newPainter.begin(&m_backgroundPixmap);
    newPainter.fillRect(m_backgroundPixmap.rect(), brush);
    newPainter.end();

    m_bRepaintBackground = false;
}
