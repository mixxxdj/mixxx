/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "mixer/playermanager.h"
#include "soundio/soundmanager.h"
#include "util/timer.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolprocessor.h"
#include "vinylcontrol/vinylcontrolxwax.h"

#include "vinylcontrol/vinylcontrolmanager.h"

VinylControlManager::VinylControlManager(QObject* pParent,
                                         UserSettingsPointer pConfig,
                                         SoundManager* pSoundManager)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pProcessor(new VinylControlProcessor(this, pConfig)),
          m_iTimerId(-1),
          m_pNumDecks(NULL),
          m_iNumConfiguredDecks(0) {
    // Register every possible VC input with SoundManager to route to the
    // VinylControlProcessor.
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, 2, i), m_pProcessor);
    }

    connect(&m_vinylControlEnabledMapper, SIGNAL(mapped(int)),
            this, SLOT(slotVinylControlEnabledChanged(int)));
}

VinylControlManager::~VinylControlManager() {
    delete m_pProcessor;

    // save a bunch of stuff to config
    // turn off vinyl control so it won't be enabled on load (this is redundant to mixxx.cpp)
    for (int i = 0; i < m_iNumConfiguredDecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_pConfig->setValue(ConfigKey(group, "vinylcontrol_enabled"), false);
        m_pConfig->set(ConfigKey(VINYL_PREF_KEY, QString("cueing_ch%1").arg(i + 1)),
            ConfigValue(static_cast<int>(ControlObject::get(
                ConfigKey(group, "vinylcontrol_cueing")))));
        m_pConfig->set(ConfigKey(VINYL_PREF_KEY, QString("mode_ch%1").arg(i + 1)),
            ConfigValue(static_cast<int>(ControlObject::get(
                ConfigKey(group, "vinylcontrol_mode")))));
    }
}

void VinylControlManager::init() {
    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &VinylControlManager::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());
}

void VinylControlManager::toggleVinylControl(int deck) {
    if (deck < 0 || deck >= m_pVcEnabled.size()) {
        return;
    }

    ControlProxy* pEnabled = m_pVcEnabled[deck];
    pEnabled->set(!pEnabled->toBool());
}

void VinylControlManager::slotNumDecksChanged(double dNumDecks) {
    int num_decks = static_cast<int>(dNumDecks);

    // Complain if we try to create more decks than we can handle.
    if (num_decks > kMaxNumberOfDecks) {
        qWarning() << "Number of decks increased to " << num_decks << ", but Mixxx only supports "
                   << kMaxNumberOfDecks << " vinyl inputs.  Decks above the maximum will not have "
                   << " vinyl control";
        num_decks = kMaxNumberOfDecks;
    }

    if (num_decks <= m_iNumConfiguredDecks) {
        // TODO(owilliams): If we implement deck deletion, shrink the size of configured decks.
        return;
    }

    for (int i = m_iNumConfiguredDecks; i < num_decks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        ControlProxy* pEnabled = new ControlProxy(group, "vinylcontrol_enabled", this);
        m_pVcEnabled.push_back(pEnabled);
        pEnabled->connectValueChanged(&m_vinylControlEnabledMapper, QOverload<int>::of(&QSignalMapper::mapped));
        m_vinylControlEnabledMapper.setMapping(pEnabled, i);

        // Default cueing should be off.
        ControlObject::set(ConfigKey(group, "vinylcontrol_cueing"),
                m_pConfig->getValue(
                        ConfigKey(VINYL_PREF_KEY, QString("cueing_ch%1").arg(i + 1)),
                        0.0));
        // Default mode should be relative.
        ControlObject::set(ConfigKey(group, "vinylcontrol_mode"),
                m_pConfig->getValue(
                        ConfigKey(VINYL_PREF_KEY, QString("mode_ch%1").arg(i + 1)),
                        MIXXX_VCMODE_RELATIVE));
    }
    m_iNumConfiguredDecks = num_decks;
}

void VinylControlManager::slotVinylControlEnabledChanged(int deck) {
    if (deck < 0 || deck >= m_pVcEnabled.size()) {
        DEBUG_ASSERT(false);
        return;
    }

    ControlProxy* pEnabled = m_pVcEnabled.at(deck);
    emit(vinylControlDeckEnabled(deck, pEnabled->toBool()));
}

void VinylControlManager::requestReloadConfig() {
    m_pProcessor->requestReloadConfig();
}

bool VinylControlManager::vinylInputConnected(int deck) {
    if (deck < 0 || deck >= m_iNumConfiguredDecks) {
        return false;
    }
    if (deck < 0 || deck >= m_pVcEnabled.length()) {
        qDebug() << "WARNING, tried to get vinyl enabled status for non-existant deck " << deck;
        return false;
    }
    return m_pProcessor->deckConfigured(deck);
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
