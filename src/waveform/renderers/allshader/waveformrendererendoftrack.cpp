#include "waveform/renderers/allshader/waveformrendererendoftrack.h"

#include <QDomNode>
#include <memory>

#include "control/controlproxy.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1000;
constexpr float positionArray[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
constexpr float verticalGradientArray[] = {1.f, 1.f, -1.f, -1.f};
constexpr float horizontalGradientArray[] = {-1.f, 1.f, -1.f, 1.f};

} // anonymous namespace

namespace allshader {

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget),
          m_pEndOfTrackControl(nullptr),
          m_pTimeRemainingControl(nullptr) {
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

void WaveformRendererEndOfTrack::initializeGL() {
    WaveformRenderer::initializeGL();
    m_shader.init();
}

void WaveformRendererEndOfTrack::fillWithGradient(QColor color) {
    const int colorLocation = m_shader.colorLocation();
    const int positionLocation = m_shader.positionLocation();
    const int gradientLocation = m_shader.gradientLocation();

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);
    m_shader.enableAttributeArray(gradientLocation);

    m_shader.setUniformValue(colorLocation, color);

    m_shader.setAttributeArray(
            positionLocation, GL_FLOAT, positionArray, 2);
    m_shader.setAttributeArray(gradientLocation,
            GL_FLOAT,
            m_waveformRenderer->getOrientation() == Qt::Vertical
                    ? verticalGradientArray
                    : horizontalGradientArray,
            1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(gradientLocation);
    m_shader.release();
}

void WaveformRendererEndOfTrack::paintGL() {
    if (!m_pEndOfTrackControl->toBool()) {
        return;
    }

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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fillWithGradient(color);
    }
}

} // namespace allshader
