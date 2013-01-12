#include "waveformrenderersignalbase.h"
#include "waveformwidgetrenderer.h"

#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

#include <QDomNode>


WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_lowFilterControlObject = NULL;
    m_midFilterControlObject = NULL;
    m_highFilterControlObject = NULL;
    m_lowKillControlObject = NULL;
    m_midKillControlObject = NULL;
    m_highKillControlObject = NULL;
    m_alignment = Qt::AlignCenter;
}

WaveformRendererSignalBase::~WaveformRendererSignalBase() {
    deleteControls();
}

void WaveformRendererSignalBase::deleteControls() {
    if(m_lowFilterControlObject)
        delete m_lowFilterControlObject;
    if(m_midFilterControlObject)
        delete m_midFilterControlObject;
    if(m_highFilterControlObject)
        delete m_highFilterControlObject;
    if(m_lowKillControlObject)
        delete m_lowKillControlObject;
    if(m_midKillControlObject)
        delete m_midKillControlObject;
    if(m_highKillControlObject)
        delete m_highKillControlObject;
}

bool WaveformRendererSignalBase::init() {
    deleteControls();

    //create controls
    m_lowFilterControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLow")));
    m_midFilterControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMid")));
    m_highFilterControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHigh")));
    m_lowKillControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLowKill")));
    m_midKillControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMidKill")));
    m_highKillControlObject = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHighKill")));

    return onInit();
}

void WaveformRendererSignalBase::setup(const QDomNode &node) {
    QString alignString = WWidget::selectNodeQString(node, "Align").toLower();
    if (alignString == "bottom") {
        m_alignment = Qt::AlignBottom;
    } else if (alignString == "top") {
        m_alignment = Qt::AlignTop;
    } else {
        m_alignment = Qt::AlignCenter;
    }

    m_colors.setup(node);
    m_axesColor = m_colors.getAxesColor();


    onSetup(node);
}
