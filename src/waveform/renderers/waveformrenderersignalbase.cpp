#include "waveformrenderersignalbase.h"

#include <QDomNode>

#include "waveformwidgetrenderer.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer),
      m_pLowFilterControlObject(NULL),
      m_pMidFilterControlObject(NULL),
      m_pHighFilterControlObject(NULL),
      m_pLowKillControlObject(NULL),
      m_pMidKillControlObject(NULL),
      m_pHighKillControlObject(NULL),
      m_alignment(Qt::AlignCenter) {
}

WaveformRendererSignalBase::~WaveformRendererSignalBase() {
    deleteControls();
}

void WaveformRendererSignalBase::deleteControls() {
    if(m_pLowFilterControlObject)
        delete m_pLowFilterControlObject;
    if(m_pMidFilterControlObject)
        delete m_pMidFilterControlObject;
    if(m_pHighFilterControlObject)
        delete m_pHighFilterControlObject;
    if(m_pLowKillControlObject)
        delete m_pLowKillControlObject;
    if(m_pMidKillControlObject)
        delete m_pMidKillControlObject;
    if(m_pHighKillControlObject)
        delete m_pHighKillControlObject;
}

bool WaveformRendererSignalBase::init() {
    deleteControls();

    //create controls
    m_pLowFilterControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterLow");
    m_pMidFilterControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterMid");
    m_pHighFilterControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterHigh");
    m_pLowKillControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterLowKill");
    m_pMidKillControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterMidKill");
    m_pHighKillControlObject = new ControlObjectThread(
            m_waveformRenderer->getGroup(),"filterHighKill");

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
    m_axesColor = m_pColors->getAxesColor();

    onSetup(node);
}
