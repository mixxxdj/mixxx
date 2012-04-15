#include <QDomNode>
#include <QLineF>
#include <QLinearGradient>
#include <QMutexLocker>

#include "controlobject.h"
#include "defs.h"
#include "glwaveformrendererfilteredsignal.h"
#include "trackinfoobject.h"
#include "waveform/waveform.h"
#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "waveform/waveformwidgetfactory.h"

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_lowFilterControlObject = NULL;
    m_midFilterControlObject = NULL;
    m_highFilterControlObject = NULL;
    m_lowKillControlObject = NULL;
    m_midKillControlObject = NULL;
    m_highKillControlObject = NULL;
    m_alignment = Qt::AlignCenter;
}

GLWaveformRendererFilteredSignal::~GLWaveformRendererFilteredSignal() {
}

void GLWaveformRendererFilteredSignal::init() {
    //create controls
    m_lowFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLow"));
    m_midFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMid"));
    m_highFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHigh"));
    m_lowKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLowKill"));
    m_midKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMidKill"));
    m_highKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHighKill"));
}

void GLWaveformRendererFilteredSignal::setup(const QDomNode& node) {
    QString alignString = WWidget::selectNodeQString(node, "Align");
    if (alignString == "bottom") {
        m_alignment = Qt::AlignBottom;
    } else if (alignString == "top") {
        m_alignment = Qt::AlignTop;
    } else {
        m_alignment = Qt::AlignCenter;
    }

    m_colors.setup(node);

    QColor low = m_colors.getLowColor();
    QColor mid = m_colors.getMidColor();
    QColor high = m_colors.getHighColor();

    QColor lowCenter = low;
    QColor midCenter = mid;
    QColor highCenter = high;

    low.setAlphaF(0.9);
    mid.setAlphaF(0.9);
    high.setAlphaF(0.9);

    lowCenter.setAlphaF(0.5);
    midCenter.setAlphaF(0.5);
    highCenter.setAlphaF(0.5);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0, low);
    gradientLow.setColorAt(0.25,low.lighter(85));
    gradientLow.setColorAt(0.5, lowCenter.darker(115));
    gradientLow.setColorAt(0.75,low.lighter(85));
    gradientLow.setColorAt(1.0, low);
    m_lowBrush = QBrush(gradientLow);

    QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0, mid);
    gradientMid.setColorAt(0.35,mid.lighter(85));
    gradientMid.setColorAt(0.5, midCenter.darker(115));
    gradientMid.setColorAt(0.65,mid.lighter(85));
    gradientMid.setColorAt(1.0, mid);
    m_midBrush = QBrush(gradientMid);

    QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0, high);
    gradientHigh.setColorAt(0.45,high.lighter(85));
    gradientHigh.setColorAt(0.5, highCenter.darker(115));
    gradientHigh.setColorAt(0.55,high.lighter(85));
    gradientHigh.setColorAt(1.0, high);
    m_highBrush = QBrush(gradientHigh);

    low.setAlphaF(0.3);
    mid.setAlphaF(0.3);
    high.setAlphaF(0.3);

    QLinearGradient gradientKilledLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledLow.setColorAt(0.0,low.darker(80));
    gradientKilledLow.setColorAt(0.5,lowCenter.darker(150));
    gradientKilledLow.setColorAt(1.0,low.darker(80));
    m_lowKilledBrush = QBrush(gradientKilledLow);

    QLinearGradient gradientKilledMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledMid.setColorAt(0.0,mid.darker(80));
    gradientKilledMid.setColorAt(0.5,midCenter.darker(150));
    gradientKilledMid.setColorAt(1.0,mid.darker(80));
    m_midKilledBrush = QBrush(gradientKilledMid);

    QLinearGradient gradientKilledHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledHigh.setColorAt(0.0,high.darker(80));
    gradientKilledHigh.setColorAt(0.5,highCenter.darker(150));
    gradientKilledHigh.setColorAt(1.0,high.darker(80));
    m_highKilledBrush = QBrush(gradientKilledHigh);
}

void GLWaveformRendererFilteredSignal::onResize() {
    m_polygon[0].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[1].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[2].resize(2*m_waveformRenderer->getWidth()+2);
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

int GLWaveformRendererFilteredSignal::buildPolygon() {
    // We have to check the track is present because it might have been unloaded
    // between the call to draw and the call to buildPolygon
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return 0;
    }

    const Waveform* waveform = pTrack->getWaveform();
    if (waveform == NULL) {
        return 0;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return 0;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return 0;
    }

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;
    int pointIndex = 0;
    setPoint(m_polygon[0][pointIndex], 0.0, 0.0);
    setPoint(m_polygon[1][pointIndex], 0.0, 0.0);
    setPoint(m_polygon[2][pointIndex], 0.0, 0.0);
    pointIndex++;

    const double offset = firstVisualIndex;


    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) /
            (double)m_waveformRenderer->getWidth();

    // The number of visual samples that should be represented within one pixel
    // given our current zoom level and track speed.
    const double visualSamplePerPixel = m_waveformRenderer->getVisualSamplePerPixel();

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0);
    if (m_lowFilterControlObject &&
            m_midFilterControlObject &&
            m_highFilterControlObject) {
        lowGain = m_lowFilterControlObject->get();
        midGain = m_midFilterControlObject->get();
        highGain = m_highFilterControlObject->get();
    }

    //apply separate visual gain
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    lowGain *= factory->getVisualGain(WaveformWidgetFactory::Low);
    midGain *= factory->getVisualGain(WaveformWidgetFactory::Mid);
    highGain *= factory->getVisualGain(WaveformWidgetFactory::High);

    //NOTE(vrince) Please help me find a better name for "channelSeparation"
    //this variable stand for merged channel ... 1 = merged & 2 = separated
    int channelSeparation = 2;
    if (m_alignment != Qt::AlignCenter)
        channelSeparation = 1;

    for (int channel = 0; channel < channelSeparation; ++channel) {
        int startPixel = 0;
        int endPixel = m_waveformRenderer->getWidth() - 1;
        int delta = 1;
        double direction = 1.0;

        //Reverse display for merged bottom channel
        if (m_alignment == Qt::AlignBottom)
            direction = -1.0;

        if (channel == 1) {
            startPixel = m_waveformRenderer->getWidth() - 1;
            endPixel = 0;
            delta = -1;
            direction = -1.0;

            // After preparing the first channel, insert the pivot point.
            setPoint(m_polygon[0][pointIndex], m_waveformRenderer->getWidth(), 0.0);
            setPoint(m_polygon[1][pointIndex], m_waveformRenderer->getWidth(), 0.0);
            setPoint(m_polygon[2][pointIndex], m_waveformRenderer->getWidth(), 0.0);
            pointIndex++;
        }

        for (int x = startPixel;
             (startPixel < endPixel) ? (x <= endPixel) : (x >= endPixel);
             x += delta) {

            // TODO(rryan) remove before 1.11 release. I'm seeing crashes
            // sometimes where the pointIndex is very very large. It hasn't come
            // back since adding locking, but I'm leaving this so that we can
            // get some info about it before crashing. (The crash usually
            // corrupts a lot of the stack).
            if (pointIndex > 2*m_waveformRenderer->getWidth()+2) {
                qDebug() << "OUT OF CONTROL"
                         << 2*m_waveformRenderer->getWidth()+2
                         << dataSize
                         << channel << pointIndex << x;
            }

            // Width of the x position in visual indices.
            const double xSampleWidth = gain * x;

            // Effective visual index of x
            const double xVisualSampleIndex = xSampleWidth + offset;

            // Our current pixel (x) corresponds to a number of visual samples
            // (visualSamplerPerPixel) in our waveform object. We take the max of
            // all the data points on either side of xVisualSampleIndex within a
            // window of 'maxSamplingRange' visual samples to measure the maximum
            // data point contained by this pixel.
            double maxSamplingRange = gain / 2.0;

            // Since xVisualSampleIndex is in visual-samples (e.g. R,L,R,L) we want
            // to check +/- maxSamplingRange frames, not samples. To do this, divide
            // xVisualSampleIndex by 2. Since frames indices are integers, we round
            // to the nearest integer by adding 0.5 before casting to int.
            int visualFrameStart = int(xVisualSampleIndex / 2.0 - maxSamplingRange + 0.5);
            int visualFrameStop = int(xVisualSampleIndex / 2.0 + maxSamplingRange + 0.5);

            // If the entire sample range is off the screen then don't calculate a
            // point for this pixel.
            const int lastVisualFrame = dataSize / 2 - 1;
            if (visualFrameStop < 0 || visualFrameStart > lastVisualFrame) {
                setPoint(m_polygon[0][pointIndex], x, 0.0);
                setPoint(m_polygon[1][pointIndex], x, 0.0);
                setPoint(m_polygon[2][pointIndex], x, 0.0);
                pointIndex++;
                continue;
            }

            // We now know that some subset of [visualFrameStart,
            // visualFrameStop] lies within the valid range of visual
            // frames. Clamp visualFrameStart/Stop to within [0,
            // lastVisualFrame].
            visualFrameStart = math_max(math_min(lastVisualFrame, visualFrameStart), 0);
            visualFrameStop = math_max(math_min(lastVisualFrame, visualFrameStop), 0);

            int visualIndexStart = visualFrameStart * 2 + channel;
            int visualIndexStop = visualFrameStop * 2 + channel;

            // if (x == m_waveformRenderer->getWidth() / 2) {
            //     qDebug() << "audioVisualRatio" << waveform->getAudioVisualRatio();
            //     qDebug() << "visualSampleRate" << waveform->getVisualSampleRate();
            //     qDebug() << "audioSamplesPerVisualPixel" << waveform->getAudioSamplesPerVisualSample();
            //     qDebug() << "visualSamplePerPixel" << visualSamplePerPixel;
            //     qDebug() << "xSampleWidth" << xSampleWidth;
            //     qDebug() << "xVisualSampleIndex" << xVisualSampleIndex;
            //     qDebug() << "maxSamplingRange" << maxSamplingRange;;
            //     qDebug() << "Sampling pixel " << x << "over [" << visualIndexStart << visualIndexStop << "]";
            // }

            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for (int i = visualIndexStart; i >= 0 && i < dataSize && i <= visualIndexStop;
                 i += channelSeparation) {
                const WaveformData& waveformData = *(data + i);
                unsigned char low = waveformData.filtered.low;
                unsigned char mid = waveformData.filtered.mid;
                unsigned char high = waveformData.filtered.high;
                maxLow = math_max(maxLow, low);
                maxBand = math_max(maxBand, mid);
                maxHigh = math_max(maxHigh, high);
            }

            setPoint(m_polygon[0][pointIndex], x, (float)maxLow*lowGain*direction);
            setPoint(m_polygon[1][pointIndex], x, (float)maxBand*midGain*direction);
            setPoint(m_polygon[2][pointIndex], x, (float)maxHigh*highGain*direction);
            pointIndex++;
        }
    }

    //If channel are not displyed separatly we nne to close the loop properly
    if (channelSeparation == 1) {
        setPoint(m_polygon[0][pointIndex], m_waveformRenderer->getWidth(), 0.0);
        setPoint(m_polygon[1][pointIndex], m_waveformRenderer->getWidth(), 0.0);
        setPoint(m_polygon[2][pointIndex], m_waveformRenderer->getWidth(), 0.0);
        pointIndex++;
    }

    return pointIndex;
}

void GLWaveformRendererFilteredSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    const TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack)
        return;

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->resetTransform();

    //visual gain
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    double visualGain = factory->getVisualGain(::WaveformWidgetFactory::All);

    if (m_alignment == Qt::AlignTop) {
        painter->translate(0.0,0.0);
        painter->scale(1.0,2.0*visualGain*m_waveformRenderer->getGain()*(double)m_waveformRenderer->getHeight()/255.0);
    } else if (m_alignment == Qt::AlignBottom) {
        painter->translate(0.0,m_waveformRenderer->getHeight());
        painter->scale(1.0,2.0*visualGain*m_waveformRenderer->getGain()*(double)m_waveformRenderer->getHeight()/255.0);
    } else {
        painter->translate(0.0,m_waveformRenderer->getHeight()/2.0);
        painter->scale(1.0,1.0*visualGain*m_waveformRenderer->getGain()*(double)m_waveformRenderer->getHeight()/255.0);
    }

    int numberOfPoints = buildPolygon();

    if (m_lowKillControlObject && m_lowKillControlObject->get() > 0.1) {
        painter->setPen(QPen(m_lowKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    } else {
        painter->setPen(QPen(m_lowBrush, 0.0));
        painter->setBrush(m_lowBrush);
    }
    painter->drawPolygon(&m_polygon[0][0],numberOfPoints);

    if (m_midKillControlObject && m_midKillControlObject->get() > 0.1) {
        painter->setPen(QPen(m_midKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    } else {
        painter->setPen(QPen(m_midBrush, 0.0));
        painter->setBrush(m_midBrush);
    }
    painter->drawPolygon(&m_polygon[1][0],numberOfPoints);

    if (m_highKillControlObject && m_highKillControlObject->get() > 0.1) {
        painter->setPen(QPen(m_highKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    } else {
        painter->setPen(QPen(m_highBrush, 0.0));
        painter->setBrush(m_highBrush);
    }
    painter->drawPolygon(&m_polygon[2][0],numberOfPoints);

    painter->restore();
}
