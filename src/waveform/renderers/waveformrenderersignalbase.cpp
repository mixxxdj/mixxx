#include "waveformrenderersignalbase.h"

#include "control/controlproxy.h"
#include "util/colorcomponents.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveformwidgetrenderer.h"

namespace {
const QString kEffectGroupFormat = QStringLiteral("[EqualizerRack1_%1_Effect1]");
} // namespace

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_pEQEnabled(nullptr),
          m_pLowFilterControlObject(nullptr),
          m_pMidFilterControlObject(nullptr),
          m_pHighFilterControlObject(nullptr),
          m_pLowKillControlObject(nullptr),
          m_pMidKillControlObject(nullptr),
          m_pHighKillControlObject(nullptr),
          m_alignment(Qt::AlignCenter),
          m_orientation(Qt::Horizontal),
          m_pColors(nullptr),
          m_axesColor_r(0),
          m_axesColor_g(0),
          m_axesColor_b(0),
          m_axesColor_a(0),
          m_signalColor_r(0),
          m_signalColor_g(0),
          m_signalColor_b(0),
          m_lowColor_r(0),
          m_lowColor_g(0),
          m_lowColor_b(0),
          m_midColor_r(0),
          m_midColor_g(0),
          m_midColor_b(0),
          m_highColor_r(0),
          m_highColor_g(0),
          m_highColor_b(0),
          m_rgbLowColor_r(0),
          m_rgbLowColor_g(0),
          m_rgbLowColor_b(0),
          m_rgbMidColor_r(0),
          m_rgbMidColor_g(0),
          m_rgbMidColor_b(0),
          m_rgbHighColor_r(0),
          m_rgbHighColor_g(0),
          m_rgbHighColor_b(0) {
}

WaveformRendererSignalBase::~WaveformRendererSignalBase() {
    deleteControls();
}

void WaveformRendererSignalBase::deleteControls() {
    if (m_pEQEnabled) {
        delete m_pEQEnabled;
    }
    if (m_pLowFilterControlObject) {
        delete m_pLowFilterControlObject;
    }
    if (m_pMidFilterControlObject) {
        delete m_pMidFilterControlObject;
    }
    if (m_pHighFilterControlObject) {
        delete m_pHighFilterControlObject;
    }
    if (m_pLowKillControlObject) {
        delete m_pLowKillControlObject;
    }
    if (m_pMidKillControlObject) {
        delete m_pMidKillControlObject;
    }
    if (m_pHighKillControlObject) {
        delete m_pHighKillControlObject;
    }
}

bool WaveformRendererSignalBase::init() {
    deleteControls();

    //create controls
    m_pEQEnabled = new ControlProxy(
            m_waveformRenderer->getGroup(), "filterWaveformEnable");
    const QString effectGroup = kEffectGroupFormat.arg(m_waveformRenderer->getGroup());
    m_pLowFilterControlObject = new ControlProxy(effectGroup, QStringLiteral("parameter1"));
    m_pMidFilterControlObject = new ControlProxy(effectGroup, QStringLiteral("parameter2"));
    m_pHighFilterControlObject = new ControlProxy(effectGroup, QStringLiteral("parameter3"));
    m_pLowKillControlObject = new ControlProxy(effectGroup, QStringLiteral("button_parameter1"));
    m_pMidKillControlObject = new ControlProxy(effectGroup, QStringLiteral("button_parameter2"));
    m_pHighKillControlObject = new ControlProxy(effectGroup, QStringLiteral("button_parameter3"));

    return onInit();
}

void WaveformRendererSignalBase::setup(const QDomNode& node,
                                       const SkinContext& context) {
    QString orientationString = context.selectString(node, "Orientation").toLower();
    if (orientationString == "vertical") {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }

    QString alignString = context.selectString(node, "Align").toLower();
    if (m_orientation == Qt::Horizontal) {
        if (alignString == "bottom") {
            m_alignment = Qt::AlignBottom;
        } else if (alignString == "top") {
            m_alignment = Qt::AlignTop;
        } else {
            m_alignment = Qt::AlignCenter;
        }
    } else {
        if (alignString == "left") {
            m_alignment = Qt::AlignLeft;
        } else if (alignString == "right") {
            m_alignment = Qt::AlignRight;
        } else {
            m_alignment = Qt::AlignCenter;
        }
    }

    m_pColors = m_waveformRenderer->getWaveformSignalColors();

    const QColor& l = m_pColors->getLowColor();
    getRgbF(l, &m_lowColor_r, &m_lowColor_g, &m_lowColor_b);

    const QColor& m = m_pColors->getMidColor();
    getRgbF(m, &m_midColor_r, &m_midColor_g, &m_midColor_b);

    const QColor& h = m_pColors->getHighColor();
    getRgbF(h, &m_highColor_r, &m_highColor_g, &m_highColor_b);

    const QColor& rgbLow = m_pColors->getRgbLowColor();
    getRgbF(rgbLow, &m_rgbLowColor_r, &m_rgbLowColor_g, &m_rgbLowColor_b);

    const QColor& rgbMid = m_pColors->getRgbMidColor();
    getRgbF(rgbMid, &m_rgbMidColor_r, &m_rgbMidColor_g, &m_rgbMidColor_b);

    const QColor& rgbHigh = m_pColors->getRgbHighColor();
    getRgbF(rgbHigh, &m_rgbHighColor_r, &m_rgbHighColor_g, &m_rgbHighColor_b);

    const QColor& rgbFilteredLow = m_pColors->getRgbLowFilteredColor();
    getRgbF(rgbFilteredLow,
            &m_rgbLowFilteredColor_r,
            &m_rgbLowFilteredColor_g,
            &m_rgbLowFilteredColor_b);

    const QColor& rgbFilteredMid = m_pColors->getRgbMidFilteredColor();
    getRgbF(rgbFilteredMid,
            &m_rgbMidFilteredColor_r,
            &m_rgbMidFilteredColor_g,
            &m_rgbMidFilteredColor_b);

    const QColor& rgbFilteredHigh = m_pColors->getRgbHighFilteredColor();
    getRgbF(rgbFilteredHigh,
            &m_rgbHighFilteredColor_r,
            &m_rgbHighFilteredColor_g,
            &m_rgbHighFilteredColor_b);

    const QColor& axes = m_pColors->getAxesColor();
    getRgbF(axes, &m_axesColor_r, &m_axesColor_g, &m_axesColor_b, &m_axesColor_a);

    const QColor& signal = m_pColors->getSignalColor();
    getRgbF(signal, &m_signalColor_r, &m_signalColor_g, &m_signalColor_b);

    onSetup(node);
}

void WaveformRendererSignalBase::getGains(float* pAllGain,
        bool applyCompensation,
        float* pLowGain,
        float* pMidGain,
        float* pHighGain) {
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    if (pAllGain) {
        *pAllGain = static_cast<CSAMPLE_GAIN>(m_waveformRenderer->getGain(applyCompensation)) *
                static_cast<CSAMPLE_GAIN>(factory->getVisualGain(WaveformWidgetFactory::All));
        ;
    }

    if (pLowGain || pMidGain || pHighGain) {
        // Per-band gain from the EQ knobs.
        CSAMPLE_GAIN lowGain = 1.0, midGain = 1.0, highGain = 1.0;

        // Only adjust low/mid/high gains if EQs are enabled.
        if (m_pEQEnabled->get() > 0.0) {
            if (m_pLowFilterControlObject &&
                m_pMidFilterControlObject &&
                m_pHighFilterControlObject) {
                lowGain = static_cast<CSAMPLE_GAIN>(m_pLowFilterControlObject->get());
                midGain = static_cast<CSAMPLE_GAIN>(m_pMidFilterControlObject->get());
                highGain = static_cast<CSAMPLE_GAIN>(m_pHighFilterControlObject->get());
            }

            lowGain *= static_cast<CSAMPLE_GAIN>(
                    factory->getVisualGain(WaveformWidgetFactory::Low));
            midGain *= static_cast<CSAMPLE_GAIN>(
                    factory->getVisualGain(WaveformWidgetFactory::Mid));
            highGain *= static_cast<CSAMPLE_GAIN>(
                    factory->getVisualGain(WaveformWidgetFactory::High));

            if (m_pLowKillControlObject && m_pLowKillControlObject->get() > 0.0) {
                lowGain = 0;
            }

            if (m_pMidKillControlObject && m_pMidKillControlObject->get() > 0.0) {
                midGain = 0;
            }

            if (m_pHighKillControlObject && m_pHighKillControlObject->get() > 0.0) {
                highGain = 0;
            }
        }

        if (pLowGain != nullptr) {
            *pLowGain = lowGain;
        }
        if (pMidGain != nullptr) {
            *pMidGain = midGain;
        }
        if (pHighGain != nullptr) {
            *pHighGain = highGain;
        }
    }
}

std::span<float, 256> WaveformRendererSignalBase::unscaleTable() {
    // Table to undo the scaling std::pow(invalue, 2.0f * 0.316f);
    // done in scaleSignal in analyzerwaveform.h
    static std::array<float, 256> result;
    for (int i = 0; i < 256; i++) {
        result[i] = 255.f * std::pow(static_cast<float>(i) / 255.f, 1.f / 0.632f);
    }
    return result;
}
