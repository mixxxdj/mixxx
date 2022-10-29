#include "effects/effectsmanager.h"

#include <QDir>
#include <QMetaType>

#include "control/controlpotmeter.h"
#include "effects/chains/equalizereffectchain.h"
#include "effects/chains/outputeffectchain.h"
#include "effects/chains/quickeffectchain.h"
#include "effects/chains/standardeffectchain.h"
#include "effects/effectslot.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectpresetmanager.h"
#include "effects/presets/effectxmlelements.h"
#include "effects/visibleeffectslist.h"
#include "engine/effects/engineeffectsmanager.h"
#include "util/assert.h"

namespace {
const unsigned int kEffectMessagePipeFifoSize = 2048;
const QString kEffectsXmlFile = QStringLiteral("effects.xml");
} // anonymous namespace

EffectsManager::EffectsManager(
        UserSettingsPointer pConfig,
        std::shared_ptr<ChannelHandleFactory> pChannelHandleFactory)
        : m_pConfig(pConfig),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_loEqFreq(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040),
          m_hiEqFreq(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040),
          m_eqButtonMode(ConfigKey("[Mixer Profile]", "EQButtonMode"), true, 0) {
    qRegisterMetaType<EffectChainMixMode>("EffectChainMixMode");

    m_pBackendManager = EffectsBackendManagerPointer(new EffectsBackendManager());

    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                    kEffectMessagePipeFifoSize, kEffectMessagePipeFifoSize);
    m_pMessenger = EffectsMessengerPointer(new EffectsMessenger(
            requestPipes.first, requestPipes.second));
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);

    m_pEffectPresetManager = EffectPresetManagerPointer(
            new EffectPresetManager(pConfig, m_pBackendManager));

    m_pChainPresetManager = EffectChainPresetManagerPointer(
            new EffectChainPresetManager(pConfig, m_pBackendManager));

    m_pVisibleEffectsList = VisibleEffectsListPointer(new VisibleEffectsList());
}

EffectsManager::~EffectsManager() {
    m_pMessenger->initiateShutdown();

    saveEffectsXml();

    // The EffectChains must be deleted before the EffectsBackends in case
    // there is an LV2 effect currently loaded.
    // ~LV2GroupState calls lilv_instance_free, which will segfault if called
    // after ~LV2Backend calls lilv_world_free.
    m_equalizerEffectChains.clear();
    m_quickEffectChains.clear();
    m_standardEffectChains.clear();
    m_outputEffectChain.clear();
    m_effectChainSlotsByGroup.clear();

    m_pMessenger->processEffectsResponses();
}

void EffectsManager::setup() {
    // Add postfader effect chain slots
    addStandardEffectChains();
    addOutputEffectChain();
    // EQ and QuickEffect chain slots are initialized when PlayerManager creates decks.
    readEffectsXml();
}

void EffectsManager::registerInputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredInputChannels.contains(handle_group)) {
        return;
    }
    m_registeredInputChannels.insert(handle_group);

    // EqualizerEffectChains, QuickEffectChains, and OutputEffectChains
    // only process one input channel, so they do not need to have new input
    // channels registered.
    for (EffectChainPointer pChainSlot : std::as_const(m_standardEffectChains)) {
        pChainSlot->registerInputChannel(handle_group);
    }
}

void EffectsManager::registerOutputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredOutputChannels.contains(handle_group)) {
        return;
    }
    m_registeredOutputChannels.insert(handle_group);
}

void EffectsManager::addStandardEffectChains() {
    for (int i = 0; i < kNumStandardEffectUnits; ++i) {
        VERIFY_OR_DEBUG_ASSERT(!m_effectChainSlotsByGroup.contains(
                StandardEffectChain::formatEffectChainGroup(i))) {
            continue;
        }

        auto pChainSlot = StandardEffectChainPointer(
                new StandardEffectChain(i, this, m_pMessenger));

        m_standardEffectChains.append(pChainSlot);
        m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
    }
}

void EffectsManager::addOutputEffectChain() {
    m_outputEffectChain = OutputEffectChainPointer(
            new OutputEffectChain(this, m_pMessenger));
    m_effectChainSlotsByGroup.insert(m_outputEffectChain->group(), m_outputEffectChain);
}

EffectChainPointer EffectsManager::getOutputEffectChain() const {
    return m_outputEffectChain;
}

EffectChainPointer EffectsManager::getStandardEffectChain(int unitNumber) const {
    VERIFY_OR_DEBUG_ASSERT(0 <= unitNumber || unitNumber < m_standardEffectChains.size()) {
        return EffectChainPointer();
    }
    return m_standardEffectChains.at(unitNumber);
}

void EffectsManager::addDeck(const QString& deckGroupName) {
    addEqualizerEffectChain(deckGroupName);
    addQuickEffectChain(deckGroupName);
}

void EffectsManager::addEqualizerEffectChain(const QString& deckGroupName) {
    VERIFY_OR_DEBUG_ASSERT(!m_equalizerEffectChains.contains(
            EqualizerEffectChain::formatEffectChainGroup(deckGroupName))) {
        return;
    }

    auto pChainSlot = EqualizerEffectChainPointer(
            new EqualizerEffectChain(deckGroupName, this, m_pMessenger));

    m_equalizerEffectChains.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

void EffectsManager::addQuickEffectChain(const QString& deckGroupName) {
    VERIFY_OR_DEBUG_ASSERT(!m_quickEffectChains.contains(
            QuickEffectChain::formatEffectChainGroup(deckGroupName))) {
        return;
    }

    auto pChainSlot = QuickEffectChainPointer(
            new QuickEffectChain(deckGroupName, this, m_pMessenger));

    m_quickEffectChains.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

EffectChainPointer EffectsManager::getEffectChain(
        const QString& group) const {
    return m_effectChainSlotsByGroup.value(group);
}

bool EffectsManager::isAdoptMetaknobSettingEnabled() const {
    return m_pConfig->getValue(ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
}

void EffectsManager::readEffectsXml() {
    QDir settingsPath(m_pConfig->getSettingsPath());
    QFile file(settingsPath.absoluteFilePath(kEffectsXmlFile));
    QDomDocument doc;

    if (file.open(QIODevice::ReadOnly)) {
        doc.setContent(&file);
    }
    file.close();

    QStringList deckStrings;
    for (auto it = m_quickEffectChains.begin(); it != m_quickEffectChains.end(); it++) {
        deckStrings << it.key();
    }
    EffectsXmlData data = m_pChainPresetManager->readEffectsXml(doc, deckStrings);

    for (int i = 0; i < data.standardEffectChainPresets.size(); i++) {
        m_standardEffectChains.value(i)->loadChainPreset(data.standardEffectChainPresets.at(i));
    }

    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        auto pChainSlot = m_quickEffectChains.value(it.key());
        if (pChainSlot) {
            pChainSlot->loadChainPreset(it.value());
        }
    }

    m_pVisibleEffectsList->readEffectsXml(doc, m_pBackendManager);
}

void EffectsManager::saveEffectsXml() {
    QDomDocument doc(EffectXml::kRoot);
    doc.setContent(EffectXml::kFileHeader);
    QDomElement rootElement = doc.createElement(EffectXml::kRoot);
    rootElement.setAttribute(
            "schemaVersion", QString::number(EffectXml::kXmlSchemaVersion));
    doc.appendChild(rootElement);

    QHash<QString, EffectChainPresetPointer> quickEffectChainPresets;
    for (auto it = m_quickEffectChains.begin(); it != m_quickEffectChains.end(); it++) {
        auto pPreset = EffectChainPresetPointer::create(it.value().data());
        quickEffectChainPresets.insert(it.key(), pPreset);
    }

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    for (const auto& pChainSlot : std::as_const(m_standardEffectChains)) {
        auto pPreset = EffectChainPresetPointer::create(pChainSlot.data());
        standardEffectChainPresets.append(pPreset);
    }

    m_pChainPresetManager->saveEffectsXml(&doc,
            EffectsXmlData{
                    quickEffectChainPresets, standardEffectChainPresets});
    m_pVisibleEffectsList->saveEffectsXml(&doc);

    QDir settingsPath(m_pConfig->getSettingsPath());
    if (!settingsPath.exists()) {
        return;
    }
    QFile file(settingsPath.absoluteFilePath(kEffectsXmlFile));
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qWarning() << "EffectsManager: could not save effects.xml file";
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}
