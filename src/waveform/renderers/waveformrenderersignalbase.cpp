#include "waveformrenderersignalbase.h"

#include "control/controlproxy.h"
#include "moc_waveformrenderersignalbase.cpp"
#include "util/colorcomponents.h"
#include "waveform/waveform.h"
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
          m_allChannelVisualGain(1),
          m_lowVisualGain(1),
          m_midVisualGain(1),
          m_highVisualGain(1),
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

    const auto* pColors = m_waveformRenderer->getWaveformSignalColors();

    getRgbF(pColors->getLowColor(), &m_lowColor_r, &m_lowColor_g, &m_lowColor_b);
    getRgbF(pColors->getMidColor(), &m_midColor_r, &m_midColor_g, &m_midColor_b);
    getRgbF(pColors->getHighColor(), &m_highColor_r, &m_highColor_g, &m_highColor_b);
    getRgbF(pColors->getRgbLowColor(), &m_rgbLowColor_r, &m_rgbLowColor_g, &m_rgbLowColor_b);
    getRgbF(pColors->getRgbMidColor(), &m_rgbMidColor_r, &m_rgbMidColor_g, &m_rgbMidColor_b);
    getRgbF(pColors->getRgbHighColor(), &m_rgbHighColor_r, &m_rgbHighColor_g, &m_rgbHighColor_b);
    getRgbF(pColors->getRgbLowFilteredColor(),
            &m_rgbLowFilteredColor_r,
            &m_rgbLowFilteredColor_g,
            &m_rgbLowFilteredColor_b);

    getRgbF(pColors->getRgbMidFilteredColor(),
            &m_rgbMidFilteredColor_r,
            &m_rgbMidFilteredColor_g,
            &m_rgbMidFilteredColor_b);

    getRgbF(pColors->getRgbHighFilteredColor(),
            &m_rgbHighFilteredColor_r,
            &m_rgbHighFilteredColor_g,
            &m_rgbHighFilteredColor_b);

    getRgbF(pColors->getAxesColor(),
            &m_axesColor_r,
            &m_axesColor_g,
            &m_axesColor_b,
            &m_axesColor_a);
    getRgbF(pColors->getSignalColor(), &m_signalColor_r, &m_signalColor_g, &m_signalColor_b);
    onSetup(node);

    auto* pWaveformFactory = WaveformWidgetFactory::instance();
    connect(pWaveformFactory,
            &WaveformWidgetFactory::visualGainChanged,
            this,
            [this](CSAMPLE_GAIN allChannelGain,
                    CSAMPLE_GAIN lowGain,
                    CSAMPLE_GAIN midGain,
                    CSAMPLE_GAIN highGain) {
                setAllChannelVisualGain(allChannelGain);
                setLowVisualGain(lowGain);
                setMidVisualGain(midGain);
                setHighVisualGain(highGain);
            });

    setAllChannelVisualGain(pWaveformFactory->getVisualGain(FilterIndex::AllChannel));
    setLowVisualGain(pWaveformFactory->getVisualGain(FilterIndex::Low));
    setMidVisualGain(pWaveformFactory->getVisualGain(FilterIndex::Mid));
    setHighVisualGain(pWaveformFactory->getVisualGain(FilterIndex::High));
}

void WaveformRendererSignalBase::getGains(float* pAllGain,
        bool applyCompensation,
        float* pLowGain,
        float* pMidGain,
        float* pHighGain) {
    if (pAllGain) {
        *pAllGain = static_cast<CSAMPLE_GAIN>(
                            m_waveformRenderer->getGain(applyCompensation)) *
                m_allChannelVisualGain;
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

            lowGain *= m_lowVisualGain;
            midGain *= m_midVisualGain;
            highGain *= m_highVisualGain;

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
