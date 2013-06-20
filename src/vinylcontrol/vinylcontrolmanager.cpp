/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrol/vinylcontrolmanager.h"

#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolprocessor.h"
#include "vinylcontrol/vinylcontrolxwax.h"
#include "controlobject.h"
#include "util/timer.h"
#include "soundmanager.h"

const int kNumberOfDecks = 4; // set to 4 because it will ideally not be more
// or less than the number of vinyl-controlled decks but will probably be
// forgotten in any 2->4 deck switchover. Only real consequence is
// sizeof(void*)*2 bytes of wasted memory if we're only using 2 decks -bkgood

VinylControlManager::VinylControlManager(QObject *pParent,
                                         ConfigObject<ConfigValue> *pConfig,
                                         SoundManager* pSoundManager)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pProcessor(new VinylControlProcessor(this, pConfig)),
          m_iTimerId(-1) {
    // Register every possible VC input with SoundManager to route to the
    // VinylControlProcessor.
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, i), m_pProcessor);
    }
}

VinylControlManager::~VinylControlManager() {
    delete m_pProcessor;

    // save a bunch of stuff to config
    // turn off vinyl control so it won't be enabled on load (this is redundant to mixxx.cpp)
    m_pConfig->set(ConfigKey("[Channel1]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[Channel2]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey(VINYL_PREF_KEY,"cueing_ch1"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel1]","vinylcontrol_cueing"))->get()));
    m_pConfig->set(ConfigKey(VINYL_PREF_KEY,"cueing_ch2"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel2]","vinylcontrol_cueing"))->get()));
}

void VinylControlManager::init() {
    // Load saved preferences now that the objects exist
    ControlObjectThread cotVcEnable1("[Channel1]","vinylcontrol_enabled");
    cotVcEnable1.set(0);
    ControlObjectThread cotVcEnable2("[Channel2]","vinylcontrol_enabled");
    cotVcEnable2.set(0);

    ControlObjectThread cotVcMode1("[Channel1]","vinylcontrol_mode");
    cotVcMode1.set(m_pConfig->getValueString(
            ConfigKey(VINYL_PREF_KEY,"mode")).toDouble());
    ControlObjectThread cotVcMode2("[Channel2]","vinylcontrol_mode");
    cotVcMode2.set(m_pConfig->getValueString(
            ConfigKey(VINYL_PREF_KEY,"mode")).toDouble());

    ControlObjectThread cotVcCueing1("[Channel1]","vinylcontrol_cueing");
    cotVcCueing1.set(m_pConfig->getValueString(
            ConfigKey(VINYL_PREF_KEY,"cueing_ch1")).toDouble());
    ControlObjectThread cotVcCueing2("[Channel2]","vinylcontrol_cueing");
    cotVcCueing2.set(m_pConfig->getValueString(
            ConfigKey(VINYL_PREF_KEY,"cueing_ch2")).toDouble());
}

void VinylControlManager::requestReloadConfig() {
    m_pProcessor->requestReloadConfig();
}

bool VinylControlManager::vinylInputEnabled(int deck) {
    ControlObjectThread input_enabled(kVCGroup.arg(deck+1), "vinylcontrol_enabled");
    return input_enabled.get() > 0;
}

int VinylControlManager::vinylInputFromGroup(const QString& group) {
    QRegExp channelMatcher("\\[Channel([1-9]\\d*)\\]");
    if (channelMatcher.exactMatch(group)) {
        bool ok = false;
        int input = channelMatcher.cap(1).toInt(&ok);
        return ok ? input - 1 : -1;
    }
    return -1;
}

void VinylControlManager::addSignalQualityListener(VinylSignalQualityListener* pListener) {
    m_listeners.insert(pListener);
    m_pProcessor->setSignalQualityReporting(true);

    if (m_iTimerId == -1) {
        m_iTimerId = startTimer(MIXXX_VINYL_SCOPE_UPDATE_LATENCY_MS);
    }
}

void VinylControlManager::removeSignalQualityListener(VinylSignalQualityListener* pListener) {
    m_listeners.remove(pListener);
    if (m_listeners.empty()) {
        m_pProcessor->setSignalQualityReporting(false);
        if (m_iTimerId != -1) {
            killTimer(m_iTimerId);
            m_iTimerId = -1;
        }
    }
}

void VinylControlManager::updateSignalQualityListeners() {
    FIFO<VinylSignalQualityReport>* signalQualityFifo = m_pProcessor->getSignalQualityFifo();
    if (signalQualityFifo == NULL) {
        return;
    }

    VinylSignalQualityReport report;
    while (signalQualityFifo->read(&report, 1) == 1) {
        foreach (VinylSignalQualityListener* pListener, m_listeners) {
            pListener->onVinylSignalQualityUpdate(report);
        }
    }
}

void VinylControlManager::timerEvent(QTimerEvent*) {
    updateSignalQualityListeners();
}
