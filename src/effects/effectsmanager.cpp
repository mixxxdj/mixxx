#include "effects/effectsmanager.h"

#include <QDir>
#include <QMetaType>

#include "control/controlpotmeter.h"
#include "effects/effectslot.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectpresetmanager.h"
#include "effects/effectxmlelements.h"
#include "effects/visibleeffectslist.h"
#include "engine/effects/engineeffectsmanager.h"
#include "util/assert.h"

namespace {
const QString kStandardEffectRackGroup = QStringLiteral("[EffectRack1]");
const QString kOutputEffectRackGroup = QStringLiteral("[OutputEffectRack]");
const QString kQuickEffectRackGroup = QStringLiteral("[QuickEffectRack1]");
const QString kEqualizerEffectRackGroup = QStringLiteral("[EqualizerRack1]");
const QString kEffectGroupSeparator = QStringLiteral("_");
const QString kGroupClose = QStringLiteral("]");
const unsigned int kEffectMessagePipeFifoSize = 2048;
const QString kEffectsXmlFile = QStringLiteral("effects.xml");
} // anonymous namespace

EffectsManager::EffectsManager(
        UserSettingsPointer pConfig,
        ChannelHandleFactory* pChannelHandleFactory)
        : m_pConfig(pConfig),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_loEqFreq(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040),
          m_hiEqFreq(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040) {
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
    m_pMessenger->startShutdownProcess();

    saveEffectsXml();

    // The EffectChainSlots must be deleted before the EffectsBackends in case
    // there is an LV2 effect currently loaded.
    // ~LV2GroupState calls lilv_instance_free, which will segfault if called
    // after ~LV2Backend calls lilv_world_free.
    m_equalizerEffectChainSlots.clear();
    m_quickEffectChainSlots.clear();
    m_standardEffectChainSlots.clear();
    m_outputEffectChainSlot.clear();
    m_effectChainSlotsByGroup.clear();

    m_pMessenger->processEffectsResponses();
}

void EffectsManager::setup() {
    // Add postfader effect chain slots
    addStandardEffectChainSlots();
    addOutputEffectChainSlot();
    // EQ and QuickEffect chain slots are initialized when PlayerManager creates decks.

    readEffectsXml();
}

void EffectsManager::registerInputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredInputChannels.contains(handle_group)) {
        return;
    }
    m_registeredInputChannels.insert(handle_group);

    // EqualizerEffectChainSlots, QuickEffectChainSlots, and OutputEffectChainSlots
    // only process one input channel, so they do not need to have new input
    // channels registered.
    for (EffectChainSlotPointer pChainSlot : m_standardEffectChainSlots) {
        pChainSlot->registerInputChannel(handle_group);
    }
}

void EffectsManager::registerOutputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredOutputChannels.contains(handle_group)) {
        return;
    }
    m_registeredOutputChannels.insert(handle_group);
}

void EffectsManager::addStandardEffectChainSlots() {
    for (int i = 0; i < kNumStandardEffectUnits; ++i) {
        VERIFY_OR_DEBUG_ASSERT(!m_effectChainSlotsByGroup.contains(
                StandardEffectChainSlot::formatEffectChainSlotGroup(i))) {
            continue;
        }

        auto pChainSlot = StandardEffectChainSlotPointer(
                new StandardEffectChainSlot(i, this, m_pMessenger));

        m_standardEffectChainSlots.append(pChainSlot);
        m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
    }
}

void EffectsManager::addOutputEffectChainSlot() {
    m_outputEffectChainSlot = OutputEffectChainSlotPointer(
            new OutputEffectChainSlot(this, m_pMessenger));
    m_effectChainSlotsByGroup.insert(m_outputEffectChainSlot->group(), m_outputEffectChainSlot);
}

EffectChainSlotPointer EffectsManager::getOutputEffectChainSlot() const {
    return m_outputEffectChainSlot;
}

EffectChainSlotPointer EffectsManager::getStandardEffectChainSlot(int unitNumber) const {
    VERIFY_OR_DEBUG_ASSERT(0 <= unitNumber || unitNumber < m_standardEffectChainSlots.size()) {
        return EffectChainSlotPointer();
    }
    return m_standardEffectChainSlots.at(unitNumber);
}

void EffectsManager::addDeck(const QString& deckGroupName) {
    addEqualizerEffectChainSlot(deckGroupName);
    addQuickEffectChainSlot(deckGroupName);
}

void EffectsManager::addEqualizerEffectChainSlot(const QString& deckGroupName) {
    VERIFY_OR_DEBUG_ASSERT(!m_equalizerEffectChainSlots.contains(
            EqualizerEffectChainSlot::formatEffectChainSlotGroup(deckGroupName))) {
        return;
    }

    auto pChainSlot = EqualizerEffectChainSlotPointer(
            new EqualizerEffectChainSlot(deckGroupName, this, m_pMessenger));

    m_equalizerEffectChainSlots.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

void EffectsManager::addQuickEffectChainSlot(const QString& deckGroupName) {
    VERIFY_OR_DEBUG_ASSERT(!m_quickEffectChainSlots.contains(
            QuickEffectChainSlot::formatEffectChainSlotGroup(deckGroupName))) {
        return;
    }

    auto pChainSlot = QuickEffectChainSlotPointer(
            new QuickEffectChainSlot(deckGroupName, this, m_pMessenger));

    m_quickEffectChainSlots.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

EffectChainSlotPointer EffectsManager::getEffectChainSlot(
        const QString& group) const {
    return m_effectChainSlotsByGroup.value(group);
}

bool EffectsManager::isAdoptMetaknobValueEnabled() const {
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
    for (auto it = m_quickEffectChainSlots.begin(); it != m_quickEffectChainSlots.end(); it++) {
        deckStrings << it.key();
    }
    EffectsXmlData data = m_pChainPresetManager->readEffectsXml(doc, deckStrings);

    for (int i = 0; i < data.standardEffectChainPresets.size(); i++) {
        m_standardEffectChainSlots.value(i)->loadChainPreset(data.standardEffectChainPresets.at(i));
    }

    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        auto pChainSlot = m_quickEffectChainSlots.value(it.key());
        if (pChainSlot) {
            pChainSlot->loadChainPreset(it.value());
        }
    }

    m_pVisibleEffectsList->readEffectsXml(doc, m_pBackendManager);
}

void EffectsManager::saveEffectsXml() {
    QDomDocument doc(EffectXml::Root);
    doc.setContent(EffectXml::FileHeader);
    QDomElement rootElement = doc.createElement(EffectXml::Root);
    rootElement.setAttribute(
            "schemaVersion", QString::number(EffectXml::kXmlSchemaVersion));
    doc.appendChild(rootElement);

    QHash<QString, EffectChainPresetPointer> quickEffectChainPresets;
    for (auto it = m_quickEffectChainSlots.begin(); it != m_quickEffectChainSlots.end(); it++) {
        auto pPreset = EffectChainPresetPointer(new EffectChainPreset(it.value().get()));
        quickEffectChainPresets.insert(it.key(), pPreset);
    }

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    for (const auto& pChainSlot : m_standardEffectChainSlots) {
        auto pPreset = EffectChainPresetPointer(new EffectChainPreset(pChainSlot.get()));
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
