#include "waveform/renderers/allshader/waveformrenderbarcounter.h"

#include <QDomNode>
#include <QFont>
#include <QImage>
#include <QPainter>

#include "moc_waveformrenderbarcounter.cpp"
#include "rendergraph/context.h"
#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/texturematerial.h"
#include "rendergraph/texture.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderBarCounter::WaveformRenderBarCounter(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_introStartPosCO(m_waveformRenderer->getGroup(),
                  QStringLiteral("intro_start_position")) {
    setUsePreprocess(true);

    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    m_showBarCounter = pWaveformWidgetFactory->getShowBarCounter();
    m_beatsPerBar = pWaveformWidgetFactory->getBeatsPerBar();
    m_downbeatsEnabled = pWaveformWidgetFactory->getDownbeatsEnabled();
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::showBarCounterChanged,
            this,
            &WaveformRenderBarCounter::setShowBarCounter);
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::beatsPerBarChanged,
            this,
            &WaveformRenderBarCounter::setBeatsPerBar);
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::downbeatsEnabledChanged,
            this,
            &WaveformRenderBarCounter::setDownbeatsEnabled);
}

void WaveformRenderBarCounter::onSetTrack() {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::beatsUpdated,
                this,
                &WaveformRenderBarCounter::slotBeatsUpdated);
        disconnect(m_pLoadedTrack.get(),
                &Track::cuesUpdated,
                this,
                &WaveformRenderBarCounter::slotCuesUpdated);
    }

    m_pLoadedTrack = m_waveformRenderer->getTrackInfo();
    m_pTrackBeats.reset();
    m_anchorBeatIndex = 0;

    if (m_pLoadedTrack) {
        m_pTrackBeats = m_pLoadedTrack->getBeats();
        connect(m_pLoadedTrack.get(),
                &Track::beatsUpdated,
                this,
                &WaveformRenderBarCounter::slotBeatsUpdated);
        connect(m_pLoadedTrack.get(),
                &Track::cuesUpdated,
                this,
                &WaveformRenderBarCounter::slotCuesUpdated);
        updateDownbeatAnchor();
    }
}

void WaveformRenderBarCounter::slotBeatsUpdated() {
    if (m_pLoadedTrack) {
        m_pTrackBeats = m_pLoadedTrack->getBeats();
    }
    updateDownbeatAnchor();
}

void WaveformRenderBarCounter::slotCuesUpdated() {
    updateDownbeatAnchor();
}

void WaveformRenderBarCounter::updateDownbeatAnchor() {
    m_anchorBeatIndex = 0;
    if (!m_pTrackBeats) {
        return;
    }
    const double introStartSample = m_introStartPosCO.get();
    if (introStartSample <= 0.0) {
        return;
    }
    const auto introPos = mixxx::audio::FramePos::fromEngineSamplePos(introStartSample);
    if (!introPos.isValid()) {
        return;
    }
    const auto closestBeat = m_pTrackBeats->findClosestBeat(introPos);
    if (!closestBeat.isValid()) {
        return;
    }
    const auto anchorIt = m_pTrackBeats->iteratorFrom(closestBeat);
    if (anchorIt != m_pTrackBeats->cend()) {
        m_anchorBeatIndex = anchorIt - m_pTrackBeats->cfirstmarker();
    }
}

void WaveformRenderBarCounter::setup(
        const QDomNode& node, const SkinContext& skinContext) {
    const QString colorStr = skinContext.selectString(
            node, QStringLiteral("BarCounterColor"));
    if (colorStr.isEmpty()) {
        m_color = QColor(255, 255, 255, 180);
    } else {
        m_color = WSkinColor::getCorrectColor(QColor(colorStr)).toRgb();
    }
}

void WaveformRenderBarCounter::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderBarCounter::removeAllChildNodes() {
    while (firstChild()) {
        detachChildNode(firstChild());
    }
}

void WaveformRenderBarCounter::preprocess() {
    if (!preprocessInner()) {
        removeAllChildNodes();
    }
}

bool WaveformRenderBarCounter::preprocessInner() {
    if (!m_downbeatsEnabled || !m_pTrackBeats ||
            (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0.0) {
        return false;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    if (!m_color.alpha()) {
        return true;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition(positionType);
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition(positionType);

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);

    if (!startPosition.isValid() || !endPosition.isValid()) {
        return false;
    }

    const auto firstMarker = m_pTrackBeats->cfirstmarker();

    // Remove previous frame's nodes
    removeAllChildNodes();

    rendergraph::Context* pContext = m_waveformRenderer->getContext();

    // Font setup for bar number labels
    const int fontSize = static_cast<int>(10.0f * devicePixelRatio);
    QFont font;
    font.setPixelSize(fontSize);
    font.setBold(true);

    const QFontMetrics metrics(font);
    const float topPadding = 2.0f;

    QColor textColor = m_color;
    if (textColor.lightnessF() < 0.65f) {
        textColor = QColor(255, 255, 255);
    }
    textColor.setAlpha(245);

    // Find the current playback position for the beat counter
    const double truePosSample = m_waveformRenderer->getTruePosSample(positionType);
    const auto playFramePos = mixxx::audio::FramePos::fromEngineSamplePos(truePosSample);
    int currentBarNumber = -1;
    int currentBeatInBar = -1;

    // Determine bar/beat at the playhead
    if (playFramePos.isValid()) {
        auto playIt = m_pTrackBeats->iteratorFrom(playFramePos);
        if (playIt != m_pTrackBeats->cbegin()) {
            --playIt;
        }
        const int playBeatIndex = (playIt - firstMarker) - m_anchorBeatIndex;
        if (m_beatsPerBar > 0) {
            currentBarNumber = (playBeatIndex / m_beatsPerBar) + 1;
            currentBeatInBar = ((playBeatIndex % m_beatsPerBar) + m_beatsPerBar) %
                            m_beatsPerBar +
                    1;
        }
    }

    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        const int globalBeatIndex = (it - firstMarker) - m_anchorBeatIndex;
        const int normalizedMod = m_beatsPerBar > 0
                ? ((globalBeatIndex % m_beatsPerBar) + m_beatsPerBar) % m_beatsPerBar
                : 1;

        if (normalizedMod != 0) {
            continue;
        }

        const int barNumber = (globalBeatIndex / m_beatsPerBar) + 1;

        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);
        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const QString text = QString::number(barNumber);
        const int textWidth = metrics.horizontalAdvance(text) + 8;
        const int textHeight = metrics.height() + 4;

        QImage image(textWidth, textHeight, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        image.setDevicePixelRatio(devicePixelRatio);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);

        const float bgPadX = 2.0f * devicePixelRatio;
        const float bgPadY = 1.0f * devicePixelRatio;
        QRectF bgRect(bgPadX, bgPadY, textWidth - 2.0f * bgPadX, textHeight - 2.0f * bgPadY);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 190));
        painter.drawRoundedRect(bgRect, 2.0, 2.0);

        painter.setFont(font);
        painter.setPen(textColor);
        painter.drawText(image.rect(), Qt::AlignCenter, text);
        painter.end();

        auto pNode = std::make_unique<GeometryNode>();
        pNode->initForRectangles<TextureMaterial>(1);

        dynamic_cast<TextureMaterial&>(pNode->material())
                .setTexture(std::make_unique<Texture>(pContext, image));
        pNode->markDirtyMaterial();

        const float x1 = static_cast<float>(xBeatPoint) + 3.0f;
        const float labelWidth = static_cast<float>(textWidth) / devicePixelRatio;
        const float labelHeight = static_cast<float>(textHeight) / devicePixelRatio;
        const float y1 = topPadding;
        const float x2 = x1 + labelWidth;
        const float y2 = y1 + labelHeight;

        TexturedVertexUpdater updater{
                pNode->geometry().vertexDataAs<Geometry::TexturedPoint2D>()};
        updater.addRectangle({x1, y1}, {x2, y2}, {0.f, 0.f}, {1.f, 1.f});
        pNode->markDirtyGeometry();

        appendChildNode(std::move(pNode));
    }

    // Beat counter at the playhead position (e.g. "12.1")
    if (m_showBarCounter && currentBarNumber > 0 && currentBeatInBar > 0) {
        const double playXPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        truePosSample, positionType);
        const float playX =
                static_cast<float>(qRound(playXPoint * devicePixelRatio) /
                        devicePixelRatio);

        const QString counterText = QString::number(currentBarNumber) +
                QStringLiteral(".") + QString::number(currentBeatInBar);

        QFont counterFont;
        const int counterFontSize = static_cast<int>(11.0f * devicePixelRatio);
        counterFont.setPixelSize(counterFontSize);
        counterFont.setBold(true);

        const QFontMetrics counterMetrics(counterFont);
        const int cTextWidth = counterMetrics.horizontalAdvance(counterText) + 8;
        const int cTextHeight = counterMetrics.height() + 4;

        QImage counterImage(
                cTextWidth, cTextHeight, QImage::Format_ARGB32_Premultiplied);
        counterImage.fill(Qt::transparent);
        counterImage.setDevicePixelRatio(devicePixelRatio);

        QPainter counterPainter(&counterImage);
        counterPainter.setRenderHint(QPainter::Antialiasing);

        const float bgPadX = 2.0f * devicePixelRatio;
        const float bgPadY = 1.0f * devicePixelRatio;
        QRectF bgRect(bgPadX, bgPadY, cTextWidth - 2.0f * bgPadX, cTextHeight - 2.0f * bgPadY);
        counterPainter.setPen(Qt::NoPen);
        counterPainter.setBrush(QColor(0, 0, 0, 200));
        counterPainter.drawRoundedRect(bgRect, 3.0, 3.0);

        counterPainter.setFont(counterFont);
        counterPainter.setPen(textColor);
        counterPainter.drawText(counterImage.rect(), Qt::AlignCenter, counterText);
        counterPainter.end();

        auto pCounterNode = std::make_unique<GeometryNode>();
        pCounterNode->initForRectangles<TextureMaterial>(1);

        dynamic_cast<TextureMaterial&>(pCounterNode->material())
                .setTexture(std::make_unique<Texture>(pContext, counterImage));
        pCounterNode->markDirtyMaterial();

        const float cLabelWidth =
                static_cast<float>(cTextWidth) / devicePixelRatio;
        const float cLabelHeight =
                static_cast<float>(cTextHeight) / devicePixelRatio;
        const float cx1 = playX + 4.0f;
        const float cy1 = topPadding;
        const float cx2 = cx1 + cLabelWidth;
        const float cy2 = cy1 + cLabelHeight;

        TexturedVertexUpdater counterUpdater{
                pCounterNode->geometry()
                        .vertexDataAs<Geometry::TexturedPoint2D>()};
        counterUpdater.addRectangle(
                {cx1, cy1}, {cx2, cy2}, {0.f, 0.f}, {1.f, 1.f});
        pCounterNode->markDirtyGeometry();

        appendChildNode(std::move(pCounterNode));
    }

    return true;
}

} // namespace allshader
