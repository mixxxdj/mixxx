#include "waveform/renderers/allshader/waveformrendererendoftrack.h"

#include <QDomNode>
#include <QVector4D>
#include <memory>

#include "control/controlproxy.h"
#include "rendergraph/geometry.h"
#include "rendergraph/material/endoftrackmaterial.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1000;

} // anonymous namespace

using namespace rendergraph;

namespace allshader {

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererAbstract(waveformWidget),
          m_pEndOfTrackControl(nullptr),
          m_pTimeRemainingControl(nullptr) {
    setGeometry(std::make_unique<Geometry>(EndOfTrackMaterial::attributes(), 4));
    setMaterial(std::make_unique<EndOfTrackMaterial>());
    setUsePreprocess(true);

    geometry().setAttributeValues(0, positionArray, 4);
    geometry().setAttributeValues(1, horizontalGradientArray, 4);
    material().setUniform(0, QVector4D{0.f, 0.f, 0.f, 0.f});
}

void WaveformRendererEndOfTrack::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

bool WaveformRendererEndOfTrack::init() {
    m_timer.restart();

    m_pEndOfTrackControl.reset(new ControlProxy(
            m_waveformRenderer->getGroup(), "end_of_track"));
    m_pTimeRemainingControl.reset(new ControlProxy(
            m_waveformRenderer->getGroup(), "time_remaining"));

    return true;
}

void WaveformRendererEndOfTrack::setup(const QDomNode& node, const SkinContext& context) {
    m_color = QColor(200, 25, 20);
    const QString endOfTrackColorName = context.selectString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_color = QColor(endOfTrackColorName);
        m_color = WSkinColor::getCorrectColor(m_color);
    }
}

void WaveformRendererEndOfTrack::preprocess() {
    const int elapsed = m_timer.elapsed().toIntegerMillis() % kBlinkingPeriodMillis;

    const double blinkIntensity = (double)(2 * abs(elapsed - kBlinkingPeriodMillis / 2)) /
            kBlinkingPeriodMillis;

    const double remainingTime = m_pTimeRemainingControl->get();
    const double remainingTimeTriggerSeconds =
            WaveformWidgetFactory::instance()->getEndOfTrackWarningTime();
    const double criticalIntensity = (remainingTimeTriggerSeconds - remainingTime) /
            remainingTimeTriggerSeconds;

    const double alpha = criticalIntensity * blinkIntensity;

    if (alpha != 0.0) {
        QColor color = m_color;
        color.setAlphaF(static_cast<float>(alpha));

        material().setUniform(0, color);
    }
}

bool WaveformRendererEndOfTrack::isSubtreeBlocked() const {
    return !m_pEndOfTrackControl->toBool();
}

} // namespace allshader
