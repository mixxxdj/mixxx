#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveform/renderers/waveformrenderbeat.h"

#include "control/controlobject.h"
#include "track/beats.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_beats.resize(128);

    // TODO(ntmusic): Read from settings
    m_measureSize = 4;
    m_phraseSize = 16;
    m_phraseColor.setRgb(255,0,0);
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    if (m_beatColor.alphaF() > 0.99)
        m_beatColor.setAlphaF(0.9);
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo)
        return;

    BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats)
        return;

    if (!m_waveformRenderer->isBeatGridEnabled())
        return;

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    //qDebug() << "trackSamples" << trackSamples
    //         << "firstDisplayedPosition" << firstDisplayedPosition
    //         << "lastDisplayedPosition" << lastDisplayedPosition;

    std::unique_ptr<BeatIterator> it(trackBeats->findBeats(
            firstDisplayedPosition * trackSamples,
            lastDisplayedPosition * trackSamples));

    // if no beat do not waste time saving/restoring painter
    if (!it || !it->hasNext()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(std::max(0.5, scaleFactor()));

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    int beatCount = 0;

    while (it->hasNext()) {
        double beatPosition = it->next();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = qRound(xBeatPoint);

        int beatIndex = it->beatIndex();

        // If we don't have enough space, double the size.
        if (beatCount >= m_beats.size()) {
            m_beats.resize(m_beats.size() * 2);
        }

        if(beatIndex >= 0) {

            if (orientation == Qt::Horizontal) {
                QRect rect(xBeatPoint + 2, rendererHeight - 4, 4, 4);

                if((beatIndex % m_phraseSize) == 0) {
                    painter->fillRect(rect, m_phraseColor);
                }
                else if((beatIndex % m_measureSize) == 0) {
                    painter->fillRect(rect, m_beatColor);
                }
            } else {
                // TODO(ntmusic): Handle Qt::Vertical, test w/ vertical waveform skin
            }
        }

        if (orientation == Qt::Horizontal) {
            m_beats[beatCount++].setLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        } else {
            m_beats[beatCount++].setLine(0.0f, xBeatPoint, rendererWidth, xBeatPoint);
        }
    }

    // Make sure to use constData to prevent detaches!
    painter->setPen(beatPen);
    painter->drawLines(m_beats.constData(), beatCount);

    painter->restore();
}
