#include "waveform/renderers/waveformrenderersignalbase.h"

#include <gtest/gtest.h>

#include "control/controlobject.h"
#include "test/mixxxtest.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

namespace {

const QString kGroup = QStringLiteral("[Channel1]");
const QString kEffectGroup = QStringLiteral("[EqualizerRack1_[Channel1]_Effect1]");

class TestSignalRenderer : public WaveformRendererSignalBase {
  public:
    using WaveformRendererSignalBase::getGains;
    using WaveformRendererSignalBase::WaveformRendererSignalBase;

    void onSetup(const QDomNode&) override {
    }
    void draw(QPainter*, QPaintEvent*) override {
    }
};

class WaveformRendererSignalBaseTest : public MixxxTest {
  protected:
    void expectGains(WaveformRendererSignalBase::Options options,
            float low,
            float mid,
            float high) {
        TestSignalRenderer renderer(&m_waveformWidgetRenderer, options);
        ASSERT_TRUE(renderer.init());
        renderer.setLowVisualGain(2.0);
        renderer.setMidVisualGain(3.0);
        renderer.setHighVisualGain(0.5);
        float gains[3] = {};
        renderer.getGains(nullptr, &gains[0], &gains[1], &gains[2]);
        EXPECT_FLOAT_EQ(low, gains[0]);
        EXPECT_FLOAT_EQ(mid, gains[1]);
        EXPECT_FLOAT_EQ(high, gains[2]);
    }

    ControlObject m_eqEnabled{ConfigKey(kGroup, QStringLiteral("filterWaveformEnable"))};
    ControlObject m_lowEq{ConfigKey(kEffectGroup, QStringLiteral("parameter1"))};
    ControlObject m_midEq{ConfigKey(kEffectGroup, QStringLiteral("parameter2"))};
    ControlObject m_highEq{ConfigKey(kEffectGroup, QStringLiteral("parameter3"))};
    ControlObject m_lowKill{ConfigKey(kEffectGroup, QStringLiteral("button_parameter1"))};
    ControlObject m_midKill{ConfigKey(kEffectGroup, QStringLiteral("button_parameter2"))};
    ControlObject m_highKill{ConfigKey(kEffectGroup, QStringLiteral("button_parameter3"))};
    WaveformWidgetRenderer m_waveformWidgetRenderer{kGroup};
};

TEST_F(WaveformRendererSignalBaseTest, IgnoreEqGain) {
    m_eqEnabled.set(1.0);
    m_lowEq.set(0.5);
    m_midEq.set(1.0);
    m_highEq.set(4.0);
    m_midKill.set(1.0);

    // EQ knobs, kill switches and visual gains
    expectGains(WaveformRendererSignalBase::Option::None, 1.0f, 0.0f, 2.0f);
    // Only the visual gains
    expectGains(WaveformRendererSignalBase::Option::IgnoreEqGain, 2.0f, 3.0f, 0.5f);

    // No per-band gain at all if EQs are disabled
    m_eqEnabled.set(0.0);
    expectGains(WaveformRendererSignalBase::Option::None, 1.0f, 1.0f, 1.0f);
    expectGains(WaveformRendererSignalBase::Option::IgnoreEqGain, 1.0f, 1.0f, 1.0f);
}

} // namespace
