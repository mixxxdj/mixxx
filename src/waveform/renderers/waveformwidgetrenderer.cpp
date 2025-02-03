#include "waveform/renderers/waveformwidgetrenderer.h"

#include <QPainter>
#include <QPainterPath>

#include "control/controlproxy.h"
#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/visualplayposition.h"
#include "waveform/waveform.h"

const double WaveformWidgetRenderer::s_waveformMinZoom = 1.0;
const double WaveformWidgetRenderer::s_waveformMaxZoom = 10.0;
const double WaveformWidgetRenderer::s_waveformDefaultZoom = 3.0;
const double WaveformWidgetRenderer::s_defaultPlayMarkerPosition = 0.5;

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
} // namespace

WaveformWidgetRenderer::WaveformWidgetRenderer(const QString& group)
        : m_group(group),
#ifdef __STEM__
          m_selectedStems(mixxx::StemChannelSelection()),
#endif
          m_orientation(Qt::Horizontal),
          m_dimBrightThreshold(kDefaultDimBrightThreshold),
          m_height(-1),
          m_width(-1),
          m_devicePixelRatio(1.0f),

          m_trackPixelCount(0.0),

          m_zoomFactor(1.0),
          m_visualSamplePerPixel(1.0),
          m_audioSamplePerPixel(1.0),
          m_alphaBeatGrid(90),
          // Really create some to manage those;
          m_visualPlayPosition(nullptr),
          m_totalVSamples(0),
          m_gain(1.0),
          m_trackSamples(0.0),
          m_scaleFactor(1.0),
          m_playMarkerPosition(s_defaultPlayMarkerPosition),
          m_pContext(nullptr),
          m_passthroughEnabled(false) {
    //qDebug() << "WaveformWidgetRenderer";
    for (int type = ::WaveformRendererAbstract::Play;
            type <= ::WaveformRendererAbstract::Slip;
            type++) {
        m_firstDisplayedPosition[type] = 0.0;
        m_lastDisplayedPosition[type] = 0.0;
        m_posVSample[type] = 0.0;
        m_pos[type] = -1.0; // disable renderers
        m_truePosSample[type] = -1.0;
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_timer = new QTime();
    currentFrame = 0;
    m_lastFrameTime = 0;
    for (int i = 0; i < 100; ++i) {
        m_lastFramesTime[i] = 0;
    }
    m_lastSystemFrameTime = 0;
    for (int i = 0; i < 100; ++i) {
        m_lastSystemFramesTime[i] = 0;
    }
#endif
}

WaveformWidgetRenderer::~WaveformWidgetRenderer() {
    //qDebug() << "~WaveformWidgetRenderer";

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        delete m_rendererStack[i];
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    delete m_timer;
#endif
}

bool WaveformWidgetRenderer::init() {
    //qDebug() << "WaveformWidgetRenderer::init, m_group=" << m_group;

    m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRateRatioCO = std::make_unique<ControlProxy>(
            m_group, QStringLiteral("rate_ratio"));
    m_pGainControlObject = std::make_unique<ControlProxy>(
            m_group, QStringLiteral("total_gain"));
    m_pTrackSamplesControlObject = std::make_unique<ControlProxy>(
            m_group, QStringLiteral("track_samples"));

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        if (!m_rendererStack[i]->init()) {
            return false;
        }
    }
    return true;
}

void WaveformWidgetRenderer::onPreRender(VSyncThread* vsyncThread) {
    if (m_passthroughEnabled) {
        // disables renderers in draw()
        for (int type = ::WaveformRendererAbstract::Play;
                type <= ::WaveformRendererAbstract::Slip;
                type++) {
            m_pos[type] = -1.0;
            m_truePosSample[type] = -1.0;
        }
        return;
    }

    // For a valid track to render we need
    m_trackSamples = m_pTrackSamplesControlObject->get();
    if (m_trackSamples <= 0) {
        return;
    }

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    double rateRatio = m_pRateRatioCO->get();

    m_gain = m_pGainControlObject->get();

    // Compute visual sample to pixel ratio
    // Allow waveform to spread one visual sample across a hundred pixels
    // NOTE: The hundred pixel limit is totally arbitrary. Theoretically,
    // there should be no limit to how far the waveforms can be zoomed in.
    double visualSamplePerPixel = m_zoomFactor * rateRatio / m_scaleFactor;
    m_visualSamplePerPixel = math_max(0.01, visualSamplePerPixel);

    TrackPointer pTrack = m_pTrack;
    if (pTrack) {
        ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            m_audioSamplePerPixel = m_visualSamplePerPixel * pWaveform->getAudioVisualRatio();
        }
    }

    double truePos[2]{0};
    m_visualPlayPosition->getPlaySlipAtNextVSync(vsyncThread,
            truePos + ::WaveformRendererAbstract::Play,
            truePos + ::WaveformRendererAbstract::Slip);
    // truePlayPos = -1 happens, when a new track is in buffer but m_visualPlayPosition was not updated

    if (m_audioSamplePerPixel > 0) {
        // Track length in pixels.
        m_trackPixelCount = m_trackSamples / 2.0 / m_audioSamplePerPixel;
        double leftOffset = m_playMarkerPosition;
        double rightOffset = 1.0 - m_playMarkerPosition;

        double displayedLengthLeft = (static_cast<double>(getLength()) / m_trackPixelCount) * leftOffset;
        double displayedLengthRight = (static_cast<double>(getLength()) / m_trackPixelCount) * rightOffset;

        //qDebug() << "WaveformWidgetRenderer::onPreRender" <<
        //        "m_playMarkerPosition=" << m_playMarkerPosition <<
        //        "leftOffset=" << leftOffset <<
        //        "rightOffset=" << rightOffset <<
        //        "displayedLengthLeft=" << displayedLengthLeft <<
        //        "displayedLengthRight=" << displayedLengthRight;

        m_totalVSamples = static_cast<int>(m_trackPixelCount * m_visualSamplePerPixel);
        for (int type = ::WaveformRendererAbstract::Play;
                type <= ::WaveformRendererAbstract::Slip;
                type++) {
            // Avoid pixel jitter in play position by rounding to the nearest track
            // pixel.
            m_pos[type] = round(truePos[type] * m_trackPixelCount) / m_trackPixelCount;
            m_posVSample[type] = static_cast<int>(m_pos[type] * m_totalVSamples);
            m_truePosSample[type] = truePos[type] * static_cast<double>(m_trackSamples);
            m_firstDisplayedPosition[type] = m_pos[type] - displayedLengthLeft;
            m_lastDisplayedPosition[type] = m_pos[type] + displayedLengthRight;
        }

    } else {
        for (int type = ::WaveformRendererAbstract::Play;
                type <= ::WaveformRendererAbstract::Slip;
                type++) {
            m_pos[type] = -1.0; // disable renderers
            m_truePosSample[type] = -1.0;
        }
    }

    // qDebug() << "WaveformWidgetRenderer::onPreRender" <<
    //         "m_group" << m_group <<
    //         "m_trackSamples" << m_trackSamples <<
    //         "m_playPos" << m_playPos <<
    //         "rateRatio" << rateRatio <<
    //         "m_gain" << m_gain;
}

void WaveformWidgetRenderer::draw(QPainter* painter, QPaintEvent* event) {
#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_lastSystemFrameTime = m_timer->restart().toIntegerNanos();
#endif

    //PerformanceTimer timer;
    //timer.start();

    // not ready to display need to wait until track initialization is done
    // draw only first in stack (background)
    int stackSize = m_rendererStack.size();
    if (shouldOnlyDrawBackground()) {
        if (stackSize) {
            m_rendererStack.at(0)->draw(painter, event);
        }
        if (m_passthroughEnabled) {
            drawPassthroughLabel(painter);
        }
        return;
    } else {
        for (int i = 0; i < stackSize; i++) {
            //qDebug() << i << " a  " << timer.restart().formatNanosWithUnit();
            m_rendererStack.at(i)->draw(painter, event);
            //qDebug() << i << " e " << timer.restart().formatNanosWithUnit();
        }

        drawPlayPosmarker(painter);
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    int systemMax = -1;
    int frameMax = -1;
    for (int i = 0; i < 100; ++i) {
        frameMax = math_max(frameMax, m_lastFramesTime[i]);
        systemMax = math_max(systemMax, m_lastSystemFramesTime[i]);
    }

    // hud debug display
    painter->drawText(1,
            12,
            QString::number(m_lastFrameTime).rightJustified(2, '0') + "(" +
                    QString::number(frameMax).rightJustified(2, '0') + ")" +
                    QString::number(m_lastSystemFrameTime) + "(" +
                    QString::number(systemMax) + ")" +
                    QString::number(realtimeError));

    painter->drawText(1,
            m_height - 1,
            QString::number(m_playPos) + " [" +
                    QString::number(m_firstDisplayedPosition) + "-" +
                    QString::number(m_lastDisplayedPosition) + "]" +
                    QString::number(rateRatio) + " | " + QString::number(m_gain) +
                    " | " + QString::number(m_rateDir) + " | " +
                    QString::number(m_zoomFactor));

    m_lastFrameTime = m_timer->restart().toIntegerNanos();

    ++currentFrame;
    currentFrame = currentFrame % 100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
#endif

    //qDebug() << "draw() end" << timer.restart().formatNanosWithUnit();
}

void WaveformWidgetRenderer::drawPlayPosmarker(QPainter* painter) {
    const int lineX = std::lround(m_width * m_playMarkerPosition);
    const int lineY = std::lround(m_height * m_playMarkerPosition);

    // draw dim outlines to increase playpos/waveform contrast
    painter->setOpacity(0.5);
    painter->setPen(m_colors.getBgColor());
    QBrush bgFill = m_colors.getBgColor();
    if (m_orientation == Qt::Horizontal) {
        // lines next to playpos
        // Note: don't draw lines where they would overlap the triangles,
        // otherwise both translucent strokes add up to a darker tone.
        painter->drawLine(lineX + 1, 4, lineX + 1, m_height);
        painter->drawLine(lineX - 1, 4, lineX - 1, m_height);

        // triangle at top edge
        // Increase line/waveform contrast
        painter->setOpacity(0.8);
        QPointF t0 = QPointF(lineX - 5, 0);
        QPointF t1 = QPointF(lineX + 5, 0);
        QPointF t2 = QPointF(lineX, 6);
        drawTriangle(painter, bgFill, t0, t1, t2);
    } else { // vertical waveforms
        painter->drawLine(4, lineY + 1, m_width, lineY + 1);
        painter->drawLine(4, lineY - 1, m_width, lineY - 1);
        // triangle at left edge
        painter->setOpacity(0.8);
        QPointF l0 = QPointF(0, lineY - 5.01);
        QPointF l1 = QPointF(0, lineY + 4.99);
        QPointF l2 = QPointF(6, lineY);
        drawTriangle(painter, bgFill, l0, l1, l2);
    }

    // draw colored play position indicators
    painter->setOpacity(1.0);
    painter->setPen(m_colors.getPlayPosColor());
    QBrush fgFill = m_colors.getPlayPosColor();
    if (m_orientation == Qt::Horizontal) {
        // play position line
        painter->drawLine(lineX, 0, lineX, m_height);
        // triangle at top edge
        QPointF t0 = QPointF(lineX - 4, 0);
        QPointF t1 = QPointF(lineX + 4, 0);
        QPointF t2 = QPointF(lineX, 5);
        drawTriangle(painter, fgFill, t0, t1, t2);
    } else {
        // vertical waveforms
        painter->drawLine(0, lineY, m_width, lineY);
        // triangle at left edge
        QPointF l0 = QPointF(0, lineY - 4.01);
        QPointF l1 = QPointF(0, lineY + 4);
        QPointF l2 = QPointF(5, lineY);
        drawTriangle(painter, fgFill, l0, l1, l2);
    }
}

void WaveformWidgetRenderer::drawTriangle(QPainter* painter,
        const QBrush& fillColor,
        QPointF p0,
        QPointF p1,
        QPointF p2) {
    QPainterPath triangle;
    painter->setPen(Qt::NoPen);
    triangle.moveTo(p0); // ° base 1
    triangle.lineTo(p1); // > base 2
    triangle.lineTo(p2); // > peak
    triangle.lineTo(p0); // > base 1
    painter->fillPath(triangle, fillColor);
}

void WaveformWidgetRenderer::drawPassthroughLabel(QPainter* painter) {
    QFont font;
    font.setFamily("Open Sans"); // default label font
    // Make the label always fit
    font.setPixelSize(math_min(25, int(m_height * 0.8)));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    font.setWeight(75); // bold
#else
    font.setWeight(QFont::Bold); // bold
#endif
    font.setItalic(false);

    QString label = QObject::tr("Passthrough");
    QFontMetrics metrics(font);
    QRect labelRect = metrics.boundingRect(label);
    // Center label
    labelRect.moveTo(
            int(m_width / 2 - labelRect.width() / 2),
            int(m_height / 2 - labelRect.height() / 2));

    // Draw text
    painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
    painter->setFont(font);
    painter->setPen(m_passthroughLabelColor);
    painter->drawText(labelRect, Qt::AlignCenter, label);
}

void WaveformWidgetRenderer::setPassThroughEnabled(bool enabled) {
    m_passthroughEnabled = enabled;
    // Nothing to do if passthrough is disabled
    if (!enabled) {
        return;
    }
    // If passthrough is activated while no track has been loaded previously mark
    // the renderer state dirty in order trigger the render process. This is only
    // required for the background renderer since that's the only one that'll
    // be processed if passtrhough is active.
    if (!m_rendererStack.isEmpty()) {
        m_rendererStack[0]->setDirty(true);
    }
}

void WaveformWidgetRenderer::resizeRenderer(int width, int height, float devicePixelRatio) {
    m_width = width;
    m_height = height;
    m_devicePixelRatio = devicePixelRatio;
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->onResize();
    }
}

void WaveformWidgetRenderer::setup(
        const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();
    QString orientationString = context.selectString(node, "Orientation").toLower();
    if (orientationString == "vertical") {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }

    bool okay;
    m_dimBrightThreshold = context.selectInt(node, QStringLiteral("DimBrightThreshold"), &okay);
    if (!okay) {
        m_dimBrightThreshold = kDefaultDimBrightThreshold;
    }

    m_colors.setup(node, context);
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setScaleFactor(m_scaleFactor);
        m_rendererStack[i]->setup(node, context);
    }
    m_passthroughLabelColor = m_colors.getPassthroughLabelColor();
}

void WaveformWidgetRenderer::setZoom(double zoom) {
    //qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    m_zoomFactor = math_clamp<double>(zoom, s_waveformMinZoom, s_waveformMaxZoom);
}

void WaveformWidgetRenderer::setDisplayBeatGridAlpha(int alpha) {
    m_alphaBeatGrid = alpha;
}

#ifdef __STEM__
void WaveformWidgetRenderer::selectStem(mixxx::StemChannelSelection stemMask) {
    m_selectedStems = stemMask;
}
#endif

void WaveformWidgetRenderer::setTrack(TrackPointer track) {
    m_pTrack = track;
    //used to postpone first display until track sample is actually available
    m_trackSamples = -1.0;

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->onSetTrack();
    }
}

ConstWaveformPointer WaveformWidgetRenderer::getWaveform() const {
    if (m_pTrack) {
        return m_pTrack->getWaveform();
    }
    return {};
}

WaveformMarkPointer WaveformWidgetRenderer::getCueMarkAtPoint(QPoint point) const {
    // The m_markPositions list follows the order of drawing, so we search the
    // list in reverse order to find the hovered mark.
    //
    // TODO It would be preferable to use WaveformMarkSet::findHoveredMark here,
    // as done by WOverview, but that requires a) making WaveformMarkSet m_marks
    // a member of this class and b) decoupling the calculation of the
    // drawoffset from the drawing and c) storing it in WaveformMark.

    for (auto it = m_markPositions.crbegin(); it != m_markPositions.crend(); ++it) {
        const WaveformMarkPointer& pMark = it->m_pMark;
        VERIFY_OR_DEBUG_ASSERT(pMark) {
            continue;
        }

        int markImagePositionInWidgetSpace = it->m_offsetOnScreen;
        QPoint pointInImageSpace;
        if (getOrientation() == Qt::Horizontal) {
            pointInImageSpace = QPoint(point.x() - markImagePositionInWidgetSpace, point.y());
        } else {
            DEBUG_ASSERT(getOrientation() == Qt::Vertical);
            pointInImageSpace = QPoint(point.x(), point.y() - markImagePositionInWidgetSpace);
        }
        if (pMark->contains(pointInImageSpace, getOrientation())) {
            return pMark;
        }
    }
    for (auto it = m_markPositions.crbegin(); it != m_markPositions.crend(); ++it) {
        const WaveformMarkPointer& pMark = it->m_pMark;
        VERIFY_OR_DEBUG_ASSERT(pMark) {
            continue;
        }

        int markImagePositionInWidgetSpace = it->m_offsetOnScreen;
        QPoint pointInImageSpace;
        if (getOrientation() == Qt::Horizontal) {
            pointInImageSpace = QPoint(point.x() - markImagePositionInWidgetSpace, point.y());
        } else {
            DEBUG_ASSERT(getOrientation() == Qt::Vertical);
            pointInImageSpace = QPoint(point.x(), point.y() - markImagePositionInWidgetSpace);
        }
        if (pMark->lineHovered(pointInImageSpace, getOrientation())) {
            return pMark;
        }
    }
    return nullptr;
}

CuePointer WaveformWidgetRenderer::getCuePointerFromIndex(int cueIndex) const {
    if (cueIndex != Cue::kNoHotCue && m_pTrack) {
        const QList<CuePointer> cueList = m_pTrack->getCuePoints();
        for (const auto& pCue : cueList) {
            if (pCue->getHotCue() == cueIndex) {
                return pCue;
            }
        }
    }
    return {};
}
