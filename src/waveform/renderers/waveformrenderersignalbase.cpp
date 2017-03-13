#include "waveformrenderersignalbase.h"

#include <QDomNode>

#include "waveform/waveformwidgetfactory.h"
#include "waveformwidgetrenderer.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer),
      m_pEQEnabled(NULL),
      m_pLowFilterControlObject(NULL),
      m_pMidFilterControlObject(NULL),
      m_pHighFilterControlObject(NULL),
      m_pLowKillControlObject(NULL),
      m_pMidKillControlObject(NULL),
      m_pHighKillControlObject(NULL),
      m_alignment(Qt::AlignCenter),
      m_pColors(0),
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
      m_highColor_b(0) {
}

WaveformRendererSignalBase::~WaveformRendererSignalBase() {
    deleteControls();
}

void WaveformRendererSignalBase::deleteControls() {
    if (m_pEQEnabled)
        delete m_pEQEnabled;
    if (m_pLowFilterControlObject)
        delete m_pLowFilterControlObject;
    if (m_pMidFilterControlObject)
        delete m_pMidFilterControlObject;
    if (m_pHighFilterControlObject)
        delete m_pHighFilterControlObject;
    if (m_pLowKillControlObject)
        delete m_pLowKillControlObject;
    if (m_pMidKillControlObject)
        delete m_pMidKillControlObject;
    if (m_pHighKillControlObject)
        delete m_pHighKillControlObject;
}

bool WaveformRendererSignalBase::init() {
    deleteControls();

    //create controls
    m_pEQEnabled = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterWaveformEnable");
    m_pLowFilterControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterLow");
    m_pLowFilterControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterLow");
    m_pMidFilterControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterMid");
    m_pHighFilterControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterHigh");
    m_pLowKillControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterLowKill");
    m_pMidKillControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterMidKill");
    m_pHighKillControlObject = new ControlObjectSlave(
            m_waveformRenderer->getGroup(), "filterHighKill");

    return onInit();
}

void WaveformRendererSignalBase::setup(const QDomNode& node,
                                       const SkinContext& context) {
    QString alignString = context.selectString(node, "Align").toLower();
    if (alignString == "bottom") {
        m_alignment = Qt::AlignBottom;
    } else if (alignString == "top") {
        m_alignment = Qt::AlignTop;
    } else {
        m_alignment = Qt::AlignCenter;
    }

    m_pColors = m_waveformRenderer->getWaveformSignalColors();

    const QColor& l = m_pColors->getLowColor();
    l.getRgbF(&m_lowColor_r, &m_lowColor_g, &m_lowColor_b);

    const QColor& m = m_pColors->getMidColor();
    m.getRgbF(&m_midColor_r, &m_midColor_g, &m_midColor_b);

    const QColor& h = m_pColors->getHighColor();
    h.getRgbF(&m_highColor_r, &m_highColor_g, &m_highColor_b);

    const QColor& axes = m_pColors->getAxesColor();
    axes.getRgbF(&m_axesColor_r, &m_axesColor_g, &m_axesColor_b,
                 &m_axesColor_a);

    const QColor& signal = m_pColors->getSignalColor();
    signal.getRgbF(&m_signalColor_r, &m_signalColor_g, &m_signalColor_b);

    onSetup(node);
}

void WaveformRendererSignalBase::getGains(float* pAllGain, float* pLowGain,
                                          float* pMidGain, float* pHighGain) {
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    if (pAllGain != NULL) {
        float allGain = m_waveformRenderer->getGain();
        allGain *= factory->getVisualGain(::WaveformWidgetFactory::All);
        *pAllGain = allGain;
    }

    if (pLowGain || pMidGain || pHighGain) {
        // Per-band gain from the EQ knobs.
        float lowGain(1.0), midGain(1.0), highGain(1.0);

        // Only adjust low/mid/high gains if EQs are enabled.
        if (m_pEQEnabled->get() > 0.0) {
            if (m_pLowFilterControlObject &&
                m_pMidFilterControlObject &&
                m_pHighFilterControlObject) {
                lowGain = m_pLowFilterControlObject->get();
                midGain = m_pMidFilterControlObject->get();
                highGain = m_pHighFilterControlObject->get();
            }

            lowGain *= factory->getVisualGain(WaveformWidgetFactory::Low);
            midGain *= factory->getVisualGain(WaveformWidgetFactory::Mid);
            highGain *= factory->getVisualGain(WaveformWidgetFactory::High);

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

        if (pLowGain != NULL) {
            *pLowGain = lowGain;
        }
        if (pMidGain != NULL) {
            *pMidGain = midGain;
        }
        if (pHighGain != NULL) {
            *pHighGain = highGain;
        }
    }
}
