#include "waveform/renderers/allshader/waveformrendererendoftrack.h"

#include <QDomNode>
#include <memory>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/timer.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/allshader/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1000;
constexpr float positionArray[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
constexpr float verticalGradientArray[] = {1.f, 1.f, -1.f, -1.f};
constexpr float horizontalGradientArray[] = {-1.f, 1.f, -1.f, 1.f};

} // anonymous namespace

using namespace allshader;

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget),
          m_pEndOfTrackControl(nullptr),
          m_pTimeRemainingControl(nullptr) {
}

WaveformRendererEndOfTrack::~WaveformRendererEndOfTrack() {
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
        m_color.setNamedColor(endOfTrackColorName);
        m_color = WSkinColor::getCorrectColor(m_color);
    }
}

void WaveformRendererEndOfTrack::initializeGL() {
    m_shader.init();
}

void WaveformRendererEndOfTrack::fillWithGradient(QColor color) {
    const int colorLocation = m_shader.uniformLocation("color");
    const int positionLocation = m_shader.attributeLocation("position");
    const int gradientLocation = m_shader.attributeLocation("gradient");

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

void WaveformRendererEndOfTrack::renderGL() {
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
        color.setAlphaF(alpha);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fillWithGradient(color);
    }
}
