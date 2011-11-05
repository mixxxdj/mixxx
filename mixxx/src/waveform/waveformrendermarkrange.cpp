// waveformrendermarkrange.cpp
// Created 11/14/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveformrendermarkrange.h"

#include "waveformrenderer.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderMarkRange::WaveformRenderMarkRange(const char* pGroup,
                                                 WaveformRenderer *parent)
        : m_pGroup(pGroup),
          m_pParent(parent),
          m_pMarkStartPoint(NULL),
          m_pMarkEndPoint(NULL),
          m_pMarkEnabled(NULL),
          m_pTrackSamples(NULL),
          m_bMarkEnabled(true),
          m_iMarkStartPoint(-1),
          m_iMarkEndPoint(-1),
          m_iWidth(0),
          m_iHeight(0),
          m_dSamplesPerDownsample(-1),
          m_iNumSamples(0),
          m_iSampleRate(-1) {

    m_pTrackSamples = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(pGroup, "track_samples")));
    slotUpdateTrackSamples(m_pTrackSamples->get());
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateTrackSamples(double)));

    m_pTrackSampleRate = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(pGroup, "track_samplerate")));
    slotUpdateTrackSampleRate(m_pTrackSampleRate->get());
    connect(m_pTrackSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateTrackSampleRate(double)));
}

WaveformRenderMarkRange::~WaveformRenderMarkRange() {
    qDebug() << this << "~WaveformRenderMarkRange()";
    delete m_pTrackSamples;
    delete m_pMarkStartPoint;
    delete m_pMarkEndPoint;
    delete m_pMarkEnabled;
}

void WaveformRenderMarkRange::slotUpdateMarkStartPoint(double v) {
    //qDebug() << "WaveformRenderMarkRange :: MarkStartPoint = " << v;
    m_iMarkStartPoint = (int)v;
}

void WaveformRenderMarkRange::slotUpdateMarkEndPoint(double v) {
    //qDebug() << "WaveformRenderMarkRange :: MarkEndPoint = " << v;
    m_iMarkEndPoint = (int)v;
}

void WaveformRenderMarkRange::slotUpdateMarkEnabled(double v) {
    //qDebug() << "WaveformRenderMarkRange :: MarkEnabled = " << v;
    m_bMarkEnabled = !(v == 0.0f);
}

void WaveformRenderMarkRange::slotUpdateTrackSamples(double samples) {
    //qDebug() << "WaveformRenderMarkRange :: samples = " << int(samples);
    m_iNumSamples = (int)samples;
}

void WaveformRenderMarkRange::slotUpdateTrackSampleRate(double sampleRate) {
    //qDebug() << "WaveformRenderMarkRange :: sampleRate = " << int(sampleRate);

    // f = z * m * n
    double m = m_pParent->getSubpixelsPerPixel();
    double f = sampleRate;
    double z = m_pParent->getPixelsPerSecond();
    double n = f / (m*z);

    m_iSampleRate = static_cast<int>(sampleRate);
    m_dSamplesPerDownsample = n;
}


void WaveformRenderMarkRange::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderMarkRange::newTrack(TrackPointer pTrack) {
}

void WaveformRenderMarkRange::setup(QDomNode node) {

    if (m_pMarkStartPoint) {
        // Disconnect the old control
        disconnect(m_pMarkStartPoint, 0, this, 0);
        delete m_pMarkStartPoint;
        m_pMarkStartPoint = NULL;
    }

    if (m_pMarkEndPoint) {
        // Disconnect the old control
        disconnect(m_pMarkEndPoint, 0, this, 0);
        delete m_pMarkEndPoint;
        m_pMarkEndPoint = NULL;
    }

    if (m_pMarkEnabled) {
        // Disconnect the old control
        disconnect(m_pMarkEnabled, 0, this, 0);
        delete m_pMarkEnabled;
        m_pMarkEnabled = NULL;
    }

    ConfigKey configKey;
    configKey.group = m_pGroup;

    configKey.item = WWidget::selectNodeQString(node, "StartControl");
    m_pMarkStartPoint = new ControlObjectThreadMain(
        ControlObject::getControl(configKey));
    slotUpdateMarkStartPoint(m_pMarkStartPoint->get());
    connect(m_pMarkStartPoint, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateMarkStartPoint(double)));

    configKey.item = WWidget::selectNodeQString(node, "EndControl");
    m_pMarkEndPoint = new ControlObjectThreadMain(
        ControlObject::getControl(configKey));
    slotUpdateMarkEndPoint(m_pMarkEndPoint->get());
    connect(m_pMarkEndPoint, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateMarkEndPoint(double)));

    // Leave m_pMarkEnabled NULL if it is not specified
    if (!WWidget::selectNode(node, "EnabledControl").isNull()) {
        configKey.item = WWidget::selectNodeQString(node, "EnabledControl");
        m_pMarkEnabled = new ControlObjectThreadMain(
            ControlObject::getControl(configKey));
        slotUpdateMarkEnabled(m_pMarkEnabled->get());
        connect(m_pMarkEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateMarkEnabled(double)));
    }

    // Read the mark color, otherwise get MarkerColor of the Visual element
    QString markColor = WWidget::selectNodeQString(node, "Color");
    if (markColor == "") {
        // As a fallback, grab the mark color from the parent's MarkerColor
        markColor = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:"
                 << markColor;
        m_markColor.setNamedColor(markColor);
        // m_markColor = QColor(255 - m_markColor.red(),
        //                      255 - m_markColor.green(),
        //                      255 - m_markColor.blue());
    } else {
        m_markColor.setNamedColor(markColor);
    }
    m_markColor = WSkinColor::getCorrectColor(m_markColor);

    QString markDisabledColor = WWidget::selectNodeQString(node, "DisabledColor");
    if (markDisabledColor == "") {
        // As a fallback, grab the mark color from the parent's MarkerColor
        markDisabledColor = WWidget::selectNodeQString(
            node.parentNode(), "SignalColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:"
                 << markDisabledColor;
        m_markDisabledColor.setNamedColor(markDisabledColor);
        // m_markDisabledColor = QColor(255 - m_markDisabledColor.red(),
        //                      255 - m_markDisabledColor.green(),
        //                      255 - m_markDisabledColor.blue());
    } else {
        m_markDisabledColor.setNamedColor(markDisabledColor);
    }
    m_markDisabledColor = WSkinColor::getCorrectColor(m_markDisabledColor);
}


void WaveformRenderMarkRange::draw(QPainter *pPainter, QPaintEvent *event,
                              QVector<float> *buffer, double dPlayPos,
                              double rateAdjust) {
    if (m_iSampleRate == -1 || m_iSampleRate == 0 || m_iNumSamples == 0)
        return;

    // necessary?
    if (buffer == NULL)
        return;

    // The range is not active, do nothing.
    if (m_iMarkStartPoint == -1 || m_iMarkEndPoint == -1)
        return;

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel()*(1.0+rateAdjust);

    pPainter->save();
    pPainter->scale(1.0/subpixelsPerPixel,1.0);
    QPen oldPen = pPainter->pen();
    QBrush oldBrush = pPainter->brush();

    double subpixelWidth = m_iWidth * subpixelsPerPixel;
    double subpixelHalfWidth = subpixelWidth / 2.0;
    double halfh = m_iHeight/2;

    double curPos = dPlayPos * (m_iNumSamples/2);

    double markStartPointMono = m_iMarkStartPoint >> 1;
    double markEndPointMono = m_iMarkEndPoint >> 1;

    double iStart = (markStartPointMono - curPos)/m_dSamplesPerDownsample;
    double xStart = iStart + subpixelHalfWidth;
    double iEnd = (markEndPointMono - curPos)/m_dSamplesPerDownsample;
    double xEnd = iEnd + subpixelHalfWidth;

    QRectF markRect(QPointF(xStart, halfh), QPointF(xEnd, -halfh));

    QColor color = m_bMarkEnabled ? m_markColor : m_markDisabledColor;
    color.setAlphaF(0.3);
    QPen newPen(color);
    pPainter->setPen(newPen);
    pPainter->setBrush(QBrush(color));
    pPainter->drawRect(markRect);

    pPainter->setPen(oldPen);
    pPainter->setBrush(oldBrush);
    pPainter->restore();
}
