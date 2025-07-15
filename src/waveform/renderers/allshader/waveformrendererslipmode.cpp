#include "waveform/renderers/allshader/waveformrendererslipmode.h"

#include <QDomNode>
#include <QVector4D>
#include <memory>

#include "control/controlproxy.h"
#include "rendergraph/geometry.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
#include "util/colorcomponents.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

namespace {

constexpr int kBlinkingPeriodMillis = 1600;

// Used as default outline color in case no value is provided in the theme
constexpr QColor kDefaultColor = QColor(224, 224, 224);

} // anonymous namespace

using namespace rendergraph;

namespace allshader {

WaveformRendererSlipMode::WaveformRendererSlipMode(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererAbstract(waveformWidget),
          m_slipBorderTopOutlineSize(10.f),
          m_slipBorderBottomOutlineSize(10.f) {
    initForRectangles<RGBAMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererSlipMode::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

bool WaveformRendererSlipMode::init() {
    m_timer.restart();

    if (m_waveformRenderer->getGroup().isEmpty()) {
        m_pSlipModeControl.reset();
        return true;
    }

    m_pSlipModeControl.reset(new ControlProxy(
            m_waveformRenderer->getGroup(), QStringLiteral("slip_enabled")));

    return true;
}

void WaveformRendererSlipMode::setup(const QDomNode& node, const SkinContext& skinContext) {
    const QString slipModeOutlineColorName =
            skinContext.selectString(node, QStringLiteral("SlipBorderOutlineColor"));
    ;
    if (!slipModeOutlineColorName.isNull()) {
        m_color = WSkinColor::getCorrectColor(QColor(slipModeOutlineColorName));
    } else {
        m_color = kDefaultColor;
    }
    const float slipBorderTopOutlineSize = skinContext.selectFloat(
            node, QStringLiteral("SlipBorderTopOutlineSize"), m_slipBorderTopOutlineSize);
    if (slipBorderTopOutlineSize >= 0) {
        m_slipBorderTopOutlineSize = slipBorderTopOutlineSize;
    }
    const float slipBorderBottomOutlineSize = skinContext.selectFloat(
            node, QStringLiteral("SlipBorderBottomOutlineSize"), m_slipBorderBottomOutlineSize);
    if (slipBorderBottomOutlineSize >= 0) {
        m_slipBorderBottomOutlineSize = slipBorderBottomOutlineSize;
    }
}

void WaveformRendererSlipMode::preprocess() {
    if (!preprocessInner()) {
        geometry().allocate(0);
        markDirtyGeometry();
    }
}

bool WaveformRendererSlipMode::preprocessInner() {
    if (!m_pSlipModeControl || !m_pSlipModeControl->toBool() ||
            !m_waveformRenderer->isSlipActive()) {
        return false;
    }

    const int elapsed = m_timer.elapsed().toIntegerMillis() % kBlinkingPeriodMillis;

    const float blinkIntensity =
            static_cast<float>(
                    2 * std::abs(elapsed - kBlinkingPeriodMillis / 2)) /
            kBlinkingPeriodMillis;

    const float alpha = std::clamp(0.25f + 0.5f * blinkIntensity, 0.0f, 1.0f);

    const float posx1 = 0.f;
    const float posx2 = m_waveformRenderer->getLength();
    const float posy1 = 0.f;
    const float posy2 = m_waveformRenderer->getBreadth() / 2.f;

    const float sideBorderOutlineSide = m_slipBorderTopOutlineSize;

    const QVector4D bgColor{0.f, 0.f, 0.f, 1.f};
    const QVector4D borderColor{m_color.redF(), m_color.greenF(), m_color.blueF(), alpha};
    const QVector4D borderColor0{m_color.redF(), m_color.greenF(), m_color.blueF(), 0.f};

    const int numVerticesPerLine = 6;            // 2 triangles
    geometry().allocate(numVerticesPerLine * 5); // border on 4 sides + bgColor filler in center

    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};

    vertexUpdater.addRectangle(
            {posx1, posy1},
            {posx2, posy2},
            bgColor);

    vertexUpdater.addRectangle(
            {posx1, posy1}, {posx2, posy1 + m_slipBorderTopOutlineSize}, borderColor);
    vertexUpdater.addRectangle({posx1, posy1 + m_slipBorderTopOutlineSize},
            {posx1 + sideBorderOutlineSide,
                    posy2},
            borderColor);
    vertexUpdater.addRectangle({posx2 - sideBorderOutlineSide, posy1 + m_slipBorderTopOutlineSize},
            {posx2, posy2},
            borderColor);
    vertexUpdater.addRectangleVGradient({posx1, posy2},
            {posx2, posy2 + m_slipBorderBottomOutlineSize},
            borderColor,
            borderColor0);
    markDirtyGeometry();
    markDirtyMaterial();

    return true;
}

} // namespace allshader
