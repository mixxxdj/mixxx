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
#include "track/phrases.h"
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
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    setUsePreprocess(true);

    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    m_showBarCounter = pWaveformWidgetFactory->getShowBarCounter();
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::showBarCounterChanged,
            this,
            &WaveformRenderBarCounter::setShowBarCounter);
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
    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats) {
        return false;
    }

    if (!m_color.alpha()) {
        return true;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0.0) {
        return false;
    }

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

    const auto firstMarker = trackBeats->cfirstmarker();
    const int downbeatOffset = trackBeats->downbeatOffset();

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
        auto playIt = trackBeats->iteratorFrom(playFramePos);
        if (playIt != trackBeats->cbegin()) {
            --playIt;
        }
        const int playBeatIndex = playIt - firstMarker;
        const int adjustedPlayIndex = playBeatIndex - downbeatOffset;
        if (m_beatsPerBar > 0) {
            currentBarNumber = (adjustedPlayIndex / m_beatsPerBar) + 1;
            currentBeatInBar = ((adjustedPlayIndex % m_beatsPerBar) + m_beatsPerBar) %
                            m_beatsPerBar +
                    1;
        }
    }

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        const int globalBeatIndex = it - firstMarker;
        const int adjustedIndex = globalBeatIndex - downbeatOffset;
        const int normalizedMod = m_beatsPerBar > 0
                ? ((adjustedIndex % m_beatsPerBar) + m_beatsPerBar) % m_beatsPerBar
                : 1;

        if (normalizedMod != 0) {
            continue;
        }

        const int barNumber = (adjustedIndex / m_beatsPerBar) + 1;

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

    // Beat counter at the playhead position (e.g. "12.1 Chorus" or "12.1 Drop in 4")
    if (m_showBarCounter && currentBarNumber > 0 && currentBeatInBar > 0) {
        const double playXPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        truePosSample, positionType);
        const float playX =
                static_cast<float>(qRound(playXPoint * devicePixelRatio) /
                        devicePixelRatio);

        QString counterText = QString::number(currentBarNumber) +
                QStringLiteral(".") + QString::number(currentBeatInBar);

        // Append phrase info if available
        mixxx::PhrasesPointer pPhrases = trackInfo->getPhrases();
        if (pPhrases && !pPhrases->isEmpty()) {
            const mixxx::Phrase* pCurrentPhrase =
                    pPhrases->findPhraseAtPosition(playFramePos);
            if (pCurrentPhrase) {
                counterText += QStringLiteral(" ") +
                        mixxx::Phrase::defaultLabel(pCurrentPhrase->type());

                // Calculate bars until next phrase boundary
                const auto& phrases = pPhrases->phrases();
                for (int i = 0; i < phrases.size(); ++i) {
                    if (&phrases[i] == pCurrentPhrase &&
                            i + 1 < phrases.size()) {
                        auto nextStart = phrases[i + 1].startPosition();
                        if (nextStart.isValid() && playFramePos < nextStart) {
                            int beatsUntil = trackBeats->numBeatsInRange(
                                    playFramePos, nextStart);
                            int barsUntil = beatsUntil / m_beatsPerBar;
                            if (barsUntil > 0 && barsUntil <= 16) {
                                counterText += QStringLiteral(" | ") +
                                        mixxx::Phrase::defaultLabel(
                                                phrases[i + 1].type()) +
                                        QStringLiteral(" in ") +
                                        QString::number(barsUntil);
                            }
                        }
                        break;
                    }
                }
            }
        }

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
