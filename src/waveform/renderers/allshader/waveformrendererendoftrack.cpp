#include "waveform/renderers/allshader/waveformrendererendoftrack.h"

#include <QDomNode>
#include <QVector4D>
#include <memory>

#include "control/controlproxy.h"
#include "moc_waveformrendererendoftrack.cpp"
#include "rendergraph/geometry.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
#include "util/colorcomponents.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1000;
constexpr int kDefaultRemainingSeccondTrigger = 30;

} // anonymous namespace

using namespace rendergraph;

namespace allshader {

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererAbstract(waveformWidget),
          m_pEndOfTrackControl(nullptr),
          m_pTimeRemainingControl(nullptr),
          m_remainingTimeTriggerSeconds(kDefaultRemainingSeccondTrigger) {
    initForRectangles<RGBAMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererEndOfTrack::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

bool WaveformRendererEndOfTrack::init() {
    m_timer.restart();

    if (m_waveformRenderer->getGroup().isEmpty()) {
        m_pEndOfTrackControl.reset();
        m_pTimeRemainingControl.reset();
        return true;
    }

    m_pEndOfTrackControl.reset(new ControlProxy(
            m_waveformRenderer->getGroup(), "end_of_track"));
    m_pTimeRemainingControl.reset(new ControlProxy(
            m_waveformRenderer->getGroup(), "time_remaining"));

    return true;
}

void WaveformRendererEndOfTrack::setup(const QDomNode& node, const SkinContext& skinContext) {
    m_color = QColor(200, 25, 20);
    const QString endOfTrackColorName =
            skinContext.selectString(node, QStringLiteral("EndOfTrackColor"));
    if (!endOfTrackColorName.isNull()) {
        m_color = QColor(endOfTrackColorName);
        m_color = WSkinColor::getCorrectColor(m_color);
    }
}

void WaveformRendererEndOfTrack::preprocess() {
    if (!preprocessInner()) {
        geometry().allocate(0);
        markDirtyGeometry();
    }
}

bool WaveformRendererEndOfTrack::preprocessInner() {
    if (!m_pEndOfTrackControl || !m_pEndOfTrackControl->toBool()) {
        return false;
    }

    const int elapsed = m_timer.elapsed().toIntegerMillis() % kBlinkingPeriodMillis;

    const double blinkIntensity =
            static_cast<double>(
                    2 * std::abs(elapsed - kBlinkingPeriodMillis / 2)) /
            kBlinkingPeriodMillis;

    const double remainingTime = m_pTimeRemainingControl->get();
    const double criticalIntensity = (m_remainingTimeTriggerSeconds - remainingTime) /
            m_remainingTimeTriggerSeconds;

    const double alpha = std::clamp(criticalIntensity * blinkIntensity, 0.0, 1.0);

    QSizeF size(m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
    float r, g, b, a;
    getRgbF(m_color, &r, &g, &b, &a);

    const float posx0 = 0.f;
    const float posx1 = static_cast<float>(size.width()) / 2.f;
    const float posx2 = static_cast<float>(size.width());
    const float posy1 = 0.f;
    const float posy2 = static_cast<float>(size.height());

    float minAlpha = 0.5f * static_cast<float>(alpha);
    float maxAlpha = 0.83f * static_cast<float>(alpha);

    geometry().allocate(6 * 2);
    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};
    vertexUpdater.addRectangleHGradient(
            {posx0, posy1}, {posx1, posy2}, {r, g, b, minAlpha}, {r, g, b, minAlpha});
    vertexUpdater.addRectangleHGradient(
            {posx1, posy1}, {posx2, posy2}, {r, g, b, minAlpha}, {r, g, b, maxAlpha});

    markDirtyGeometry();
    markDirtyMaterial();

    return true;
}

} // namespace allshader
