#include "waveform/renderers/allshader/waveformrendererslipmode.h"

#include <QDomNode>
#include <memory>

#include "control/controlproxy.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1600;
// Position matrix passed to OpenGL when drawing the shader
constexpr float positionArray[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};

// Used as default outline color in case no value is provided in the theme

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
constexpr
#else
const
#endif
        QColor kDefaultColor = QColor(224, 224, 224);

} // anonymous namespace

namespace allshader {

WaveformRendererSlipMode::WaveformRendererSlipMode(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget),
          m_slipBorderTopOutlineSize(10.f),
          m_slipBorderBottomOutlineSize(10.f) {
}

bool WaveformRendererSlipMode::init() {
    m_timer.restart();

    m_pSlipMode = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "slip_enabled");

    return true;
}

void WaveformRendererSlipMode::setup(const QDomNode& node, const SkinContext& context) {
    const QString slipModeOutlineColorName = context.selectString(node, "SlipBorderOutlineColor");
    if (!slipModeOutlineColorName.isNull()) {
        m_color = WSkinColor::getCorrectColor(QColor(slipModeOutlineColorName));
    } else {
        m_color = kDefaultColor;
    }
    const float slipBorderTopOutlineSize = context.selectFloat(
            node, "SlipBorderTopOutlineSize", m_slipBorderTopOutlineSize);
    if (slipBorderTopOutlineSize >= 0) {
        m_slipBorderTopOutlineSize = slipBorderTopOutlineSize;
    }
    const float slipBorderBottomOutlineSize = context.selectFloat(
            node, "SlipBorderBottomOutlineSize", m_slipBorderBottomOutlineSize);
    if (slipBorderBottomOutlineSize >= 0) {
        m_slipBorderBottomOutlineSize = slipBorderBottomOutlineSize;
    }
}

void WaveformRendererSlipMode::initializeGL() {
    WaveformRenderer::initializeGL();
    m_shader.init();
}

void WaveformRendererSlipMode::paintGL() {
    if (!m_pSlipMode->toBool() || !m_waveformRenderer->isSlipActive()) {
        return;
    }

    const int elapsed = m_timer.elapsed().toIntegerMillis() % kBlinkingPeriodMillis;

    const double blinkIntensity =
            static_cast<double>(2 * abs(elapsed - kBlinkingPeriodMillis / 2)) /
            kBlinkingPeriodMillis;

    const double alpha = 0.25 + 0.5 * blinkIntensity;

    if (alpha != 0.0) {
        QColor color = m_color;
        color.setAlphaF(static_cast<float>(alpha));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const int colorLocation = m_shader.colorLocation();
        const int borderLocation = m_shader.boarderLocation();
        const int positionLocation = m_shader.positionLocation();
        const int gradientLocation = m_shader.dimensionLocation();

        m_shader.bind();
        m_shader.enableAttributeArray(positionLocation);

        m_shader.setUniformValue(colorLocation, color);
        m_shader.setUniformValue(borderLocation,
                m_slipBorderTopOutlineSize,
                m_slipBorderBottomOutlineSize);

        m_shader.setAttributeArray(
                positionLocation, GL_FLOAT, positionArray, 2);

        m_shader.setUniformValue(gradientLocation,
                static_cast<float>(m_waveformRenderer->getLength()) / 2,
                static_cast<float>(m_waveformRenderer->getBreadth()) / 2);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_shader.disableAttributeArray(positionLocation);
        m_shader.release();
    }
}

} // namespace allshader
