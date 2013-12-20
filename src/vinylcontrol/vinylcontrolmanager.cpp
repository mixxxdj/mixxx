/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "controlobject.h"
#include "controlobjectslave.h"
#include "playermanager.h"
#include "soundmanager.h"
#include "util/timer.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolprocessor.h"
#include "vinylcontrol/vinylcontrolxwax.h"

#include "vinylcontrol/vinylcontrolmanager.h"

VinylControlManager::VinylControlManager(QObject* pParent,
                                         ConfigObject<ConfigValue>* pConfig,
                                         SoundManager* pSoundManager)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pProcessor(new VinylControlProcessor(this, pConfig)),
          m_iTimerId(-1),
          m_iNumConfiguredDecks(0) {
    // Register every possible VC input with SoundManager to route to the
    // VinylControlProcessor.
    for (int i = 0; i < kMaxNumberOfDecks; ++i) {
        m_pVcEnabled[i] = NULL;
    }
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, i), m_pProcessor);
    }
}

VinylControlManager::~VinylControlManager() {
    delete m_pProcessor;

    // save a bunch of stuff to config
    // turn off vinyl control so it won't be enabled on load (this is redundant to mixxx.cpp)
    for (int i = 0; i < m_iNumConfiguredDecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_pConfig->set(ConfigKey(group, "vinylcontrol_enabled"), false);
        m_pConfig->set(ConfigKey(VINYL_PREF_KEY, QString("cueing_ch%1").arg(i)),
            ConfigValue((int)ControlObject::get(
                ConfigKey(group, "vinylcontrol_cueing"))));
    }
}

void VinylControlManager::init() {
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)), Qt::DirectConnection);
    slotNumDecksChanged(m_pNumDecks->get());
}

void VinylControlManager::slotNumDecksChanged(double dNumDecks) {
    int num_decks = static_cast<int>(dNumDecks);
    if (num_decks <= m_iNumConfiguredDecks) {
        // TODO(owilliams): If we implement deck deletion, shrink the size of configured decks.
        return;
    }

    // Complain very very loudly if we try to break out of the array.
    Q_ASSERT(num_decks <= kMaxNumberOfDecks);

    for (int i = m_iNumConfiguredDecks; i < num_decks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_pVcEnabled[i] = new ControlObjectThread(group, "vinylcontrol_enabled", this);
        m_pVcEnabled[i]->set(0);

        ControlObject::set(ConfigKey(group, "vinylcontrol_mode"),
                           m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY, "mode")).toDouble());

        ControlObject::set(ConfigKey(group, "vinylcontrol_cueing"),
                           m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY,
                                                               "cueing_ch1")).toDouble());
    }
    m_iNumConfiguredDecks = num_decks;
}

void VinylControlManager::requestReloadConfig() {
    m_pProcessor->requestReloadConfig();
}

bool VinylControlManager::vinylInputEnabled(int deck) {
    if (deck < 0 || deck >= m_iNumConfiguredDecks) {
        return false;
    }
    if (m_pVcEnabled[deck] == NULL) {
        qDebug() << "WARNING, tried to get vinyl enabled status for NULL deck " << deck;
        return false;
    }
    return m_pVcEnabled[deck]->get() != 0;
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
