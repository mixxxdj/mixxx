#include "waveform/renderers/waveformrendermark.h"

#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

class ImageGraphics : public WaveformMark::Graphics {
    QImage m_image;

  public:
    ImageGraphics(QImage&& image)
            : m_image{std::move(image)} {
    }

    const QImage& image() const {
        return m_image;
    }
};

WaveformRenderMark::WaveformRenderMark(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRenderMarkBase(waveformWidgetRenderer, true) {
}

void WaveformRenderMark::draw(QPainter* painter, QPaintEvent* /*event*/) {
    PainterScope PainterScope(painter);
    // Associates mark objects with their positions in the widget.
    QList<WaveformWidgetRenderer::WaveformMarkOnScreen> marksOnScreen;

    painter->setWorldMatrixEnabled(false);

    for (const auto& pMark : std::as_const(m_marks)) {
        const QImage& image = static_cast<ImageGraphics*>(pMark->m_pGraphics.get())->image();

        const double samplePosition = pMark->getSamplePosition();
        if (samplePosition != Cue::kNoPosition) {
            const double currentMarkPoint =
                    m_waveformRenderer->transformSamplePositionInRendererWorld(samplePosition);
            const double sampleEndPosition = pMark->getSampleEndPosition();
            if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
                const int markWidth = std::lround(image.width() /
                        m_waveformRenderer->getDevicePixelRatio());
                const int drawOffset = std::lround(currentMarkPoint + pMark->getOffset());

                bool visible = false;
                // Check if the current point needs to be displayed.
                if (drawOffset > -markWidth && drawOffset < m_waveformRenderer->getWidth()) {
                    painter->drawImage(drawOffset, 0, image);
                    visible = true;
                }

                // Check if the range needs to be displayed.
                if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
                    DEBUG_ASSERT(samplePosition < sampleEndPosition);
                    const double currentMarkEndPoint =
                            m_waveformRenderer->transformSamplePositionInRendererWorld(
                                    sampleEndPosition);
                    if (visible || currentMarkEndPoint > 0) {
                        QColor color = pMark->fillColor();
                        color.setAlphaF(0.4f);

                        QLinearGradient gradient(QPointF(0, 0),
                                QPointF(0, m_waveformRenderer->getHeight()));
                        gradient.setColorAt(0, color);
                        gradient.setColorAt(0.25, QColor(Qt::transparent));
                        gradient.setColorAt(0.75, QColor(Qt::transparent));
                        gradient.setColorAt(1, color);
                        painter->fillRect(
                                QRectF(QPointF(currentMarkPoint, 0),
                                        QPointF(currentMarkEndPoint,
                                                m_waveformRenderer
                                                        ->getHeight())),
                                QBrush(gradient));
                        visible = true;
                    }
                }

                if (visible) {
                    marksOnScreen.append(
                            WaveformWidgetRenderer::WaveformMarkOnScreen{
                                    pMark, drawOffset});
                }
            } else {
                const int markHeight = std::lroundf(image.height() /
                        m_waveformRenderer->getDevicePixelRatio());
                const int drawOffset =
                        std::lround(static_cast<float>(currentMarkPoint) +
                                pMark->getOffset());

                bool visible = false;
                // Check if the current point needs to be displayed.
                if (drawOffset > -markHeight && drawOffset < m_waveformRenderer->getHeight()) {
                    painter->drawImage(0, drawOffset, image);
                    visible = true;
                }

                // Check if the range needs to be displayed.
                if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
                    DEBUG_ASSERT(samplePosition < sampleEndPosition);
                    double currentMarkEndPoint =
                            m_waveformRenderer
                                    ->transformSamplePositionInRendererWorld(
                                            sampleEndPosition);
                    if (currentMarkEndPoint < m_waveformRenderer->getHeight()) {
                        QColor color = pMark->fillColor();
                        color.setAlphaF(0.4f);

                        QLinearGradient gradient(QPointF(0, 0),
                                QPointF(m_waveformRenderer->getWidth(), 0));
                        gradient.setColorAt(0, color);
                        gradient.setColorAt(0.25, QColor(Qt::transparent));
                        gradient.setColorAt(0.75, QColor(Qt::transparent));
                        gradient.setColorAt(1, color);
                        painter->fillRect(
                                QRectF(QPointF(0, currentMarkPoint),
                                        QPointF(m_waveformRenderer->getWidth(),
                                                currentMarkEndPoint)),
                                QBrush(gradient));
                        visible = true;
                    }
                }

                if (visible) {
                    marksOnScreen.append(
                            WaveformWidgetRenderer::WaveformMarkOnScreen{
                                    pMark, drawOffset});
                }
            }
        }
    }
    m_waveformRenderer->setMarkPositions(marksOnScreen);
}

void WaveformRenderMark::updateMarkImage(WaveformMarkPointer pMark) {
    if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
        pMark->m_pGraphics = std::make_unique<ImageGraphics>(
                pMark->generateImage(m_waveformRenderer->getDevicePixelRatio()));
    } else {
        pMark->m_pGraphics = std::make_unique<ImageGraphics>(
                pMark->generateImage(m_waveformRenderer->getDevicePixelRatio())
                        .transformed(QTransform().rotate(90)));
    }
}
void WaveformRenderMark::updateEndMarkImage(WaveformMarkPointer pMark) {
    if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
        pMark->m_pEndGraphics = std::make_unique<ImageGraphics>(
                pMark->generateEndImage(m_waveformRenderer->getDevicePixelRatio()));
    } else {
        pMark->m_pEndGraphics = std::make_unique<ImageGraphics>(
                pMark->generateEndImage(m_waveformRenderer->getDevicePixelRatio())
                        .transformed(QTransform().rotate(90)));
    }
}
