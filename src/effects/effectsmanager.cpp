#include "effects/effectsmanager.h"

#include <QDir>
#include <QMetaType>
#include <algorithm>

#include "effects/effectslot.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/effectxmlelements.h"
#include "util/assert.h"

namespace {
const QString kEffectDefaultsDirectory = "/effects/defaults";
const QString kStandardEffectRackGroup = "[EffectRack1]";
const QString kOutputEffectRackGroup = "[OutputEffectRack]";
const QString kQuickEffectRackGroup = "[QuickEffectRack1]";
const QString kEqualizerEffectRackGroup = "[EqualizerRack1]";
const QString kEffectGroupSeparator = "_";
const QString kGroupClose = "]";
const unsigned int kEffectMessagePipeFifoSize = 2048;
} // anonymous namespace

EffectsManager::EffectsManager(QObject* pParent,
        UserSettingsPointer pConfig,
        ChannelHandleFactory* pChannelHandleFactory)
        : QObject(pParent),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_loEqFreq(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040),
          m_hiEqFreq(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040),
          m_pConfig(pConfig) {
    qRegisterMetaType<EffectChainMixMode>("EffectChainMixMode");

    m_pBackendManager = EffectsBackendManagerPointer(new EffectsBackendManager());

    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                    kEffectMessagePipeFifoSize, kEffectMessagePipeFifoSize);
    m_pMessenger = EffectsMessengerPointer(new EffectsMessenger(
            requestPipes.first, requestPipes.second));
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);

    m_pChainPresetManager = EffectChainPresetManagerPointer(
            new EffectChainPresetManager(pConfig, m_pBackendManager));
}

EffectsManager::~EffectsManager() {
    m_pMessenger->startShutdownProcess();

    saveEffectsXml();
    for (const auto pEffectPreset : m_defaultPresets) {
        saveDefaultForEffect(pEffectPreset);
    }

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

bool EffectsManager::isAdoptMetaknobValueEnabled() const {
    return m_pConfig->getValue(ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
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

void EffectsManager::loadStandardEffect(const int iChainSlotNumber,
        const int iEffectSlotNumber, const EffectManifestPointer pManifest) {
    auto pChainSlot = getStandardEffectChainSlot(iChainSlotNumber);
    if (pChainSlot) {
        loadEffect(pChainSlot, iEffectSlotNumber, pManifest);
    }
}

void EffectsManager::loadOutputEffect(const int iEffectSlotNumber,
    const EffectManifestPointer pManifest) {
    if (m_outputEffectChainSlot) {
        loadEffect(m_outputEffectChainSlot, iEffectSlotNumber, pManifest);
    }
}

void EffectsManager::loadEqualizerEffect(const QString& deckGroup,
        const int iEffectSlotNumber, const EffectManifestPointer pManifest) {
    auto pChainSlot = m_equalizerEffectChainSlots.value(deckGroup);
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return;
    }
    loadEffect(pChainSlot, iEffectSlotNumber, pManifest);
}

void EffectsManager::loadEffect(EffectChainSlotPointer pChainSlot,
        const int iEffectSlotNumber,
        const EffectManifestPointer pManifest,
        EffectPresetPointer pPreset,
        bool adoptMetaknobFromPreset) {
    if (pPreset == nullptr) {
        pPreset = m_defaultPresets.value(pManifest);
    }
    pChainSlot->loadEffect(
            iEffectSlotNumber,
            pManifest,
            m_pBackendManager->createProcessor(pManifest),
            pPreset,
            adoptMetaknobFromPreset);
}

ParameterMap EffectsManager::getLoadedParameters(int chainNumber, int effectNumber) const {
    return m_standardEffectChainSlots.at(chainNumber)->getEffectSlot(effectNumber)->getLoadedParameters();
}

ParameterMap EffectsManager::getHiddenParameters(int chainNumber, int effectNumber) const {
    return m_standardEffectChainSlots.at(chainNumber)->getEffectSlot(effectNumber)->getHiddenParameters();
}

void EffectsManager::hideParameter(int chainNumber, int effectNumber, EffectParameterPointer pParameter) {
    m_standardEffectChainSlots.at(chainNumber)->getEffectSlot(effectNumber)->hideParameter(pParameter);
}

void EffectsManager::showParameter(int chainNumber, int effectNumber, EffectParameterPointer pParameter) {
    m_standardEffectChainSlots.at(chainNumber)->getEffectSlot(effectNumber)->showParameter(pParameter);
}

void EffectsManager::loadPresetToStandardChain(int chainNumber, EffectChainPresetPointer pPreset) {
    m_standardEffectChainSlots.at(chainNumber)->loadChainPreset(pPreset);
}

QString EffectsManager::getNextEffectId(const QString& effectId) {
    if (m_visibleEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_visibleEffectManifests.first()->id();
    }

    int index;
    for (index = 0; index < m_visibleEffectManifests.size(); ++index) {
        if (effectId == m_visibleEffectManifests.at(index)->id()) {
            break;
        }
    }
    if (++index >= m_visibleEffectManifests.size()) {
        index = 0;
    }
    return m_visibleEffectManifests.at(index)->id();
}

QString EffectsManager::getPrevEffectId(const QString& effectId) {
    if (m_visibleEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_visibleEffectManifests.last()->id();
    }

    int index;
    for (index = 0; index < m_visibleEffectManifests.size(); ++index) {
        if (effectId == m_visibleEffectManifests.at(index)->id()) {
            break;
        }
    }
    if (--index < 0) {
        index = m_visibleEffectManifests.size() - 1;
    }
    return m_visibleEffectManifests.at(index)->id();
}

void EffectsManager::addStandardEffectChainSlots() {
    for (int i = 0; i < EffectsManager::kNumStandardEffectChains; ++i) {
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

EffectSlotPointer EffectsManager::getEffectSlot(
        const QString& group) {
    QRegExp intRegEx(".*(\\d+).*");

    QStringList parts = group.split(kEffectGroupSeparator);

    // NOTE(Kshitij) : Assuming the group is valid
    const QString chainGroup = parts.at(0) + kEffectGroupSeparator + parts.at(1) + kGroupClose;
    EffectChainSlotPointer pChainSlot = getEffectChainSlot(chainGroup);
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return EffectSlotPointer();
    }

    intRegEx.indexIn(parts.at(2));
    EffectSlotPointer pEffectSlot =
            pChainSlot->getEffectSlot(intRegEx.cap(1).toInt() - 1);
    return pEffectSlot;
}

EffectParameterSlotBasePointer EffectsManager::getEffectParameterSlot(
        const EffectParameterType parameterType,
        const ConfigKey& configKey) {
    EffectSlotPointer pEffectSlot =
             getEffectSlot(configKey.group);
    VERIFY_OR_DEBUG_ASSERT(pEffectSlot) {
        return EffectParameterSlotBasePointer();
    }

    QRegExp intRegEx(".*(\\d+).*");
    intRegEx.indexIn(configKey.item);
    EffectParameterSlotBasePointer pParameterSlot = pEffectSlot->getEffectParameterSlot(
            parameterType, intRegEx.cap(1).toInt() - 1);
    return pParameterSlot;
}

void EffectsManager::setEffectVisibility(EffectManifestPointer pManifest, bool visible) {
    if (visible && !m_visibleEffectManifests.contains(pManifest)) {
        auto insertion_point = std::lower_bound(m_visibleEffectManifests.begin(),
                m_visibleEffectManifests.end(),
                pManifest,
                EffectManifest::alphabetize);
        m_visibleEffectManifests.insert(insertion_point, pManifest);
        emit visibleEffectsUpdated();
    } else if (!visible) {
        m_visibleEffectManifests.removeOne(pManifest);
        emit visibleEffectsUpdated();
    }
}

bool EffectsManager::getEffectVisibility(EffectManifestPointer pManifest) {
    return m_visibleEffectManifests.contains(pManifest);
}

void EffectsManager::setup() {
    // Add postfader effect chain slots
    addStandardEffectChainSlots();
    addOutputEffectChainSlot();

    loadDefaultEffectPresets();

    readEffectsXml();
}

void EffectsManager::loadDefaultEffectPresets() {
    // Load saved defaults from settings directory
    QString dirPath(m_pConfig->getSettingsPath() + kEffectDefaultsDirectory);
    QDir effectsDefaultsDir(dirPath);
    effectsDefaultsDir.setFilter(QDir::Files | QDir::Readable);
    for (const auto& filePath : effectsDefaultsDir.entryList()) {
        QFile file(dirPath + "/" + filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        QDomDocument doc;
        if (!doc.setContent(&file)) {
            file.close();
            continue;
        }
        EffectPresetPointer pEffectPreset(new EffectPreset(doc.documentElement()));
        if (!pEffectPreset->isEmpty()) {
            EffectManifestPointer pManifest = m_pBackendManager->getManifest(
                    pEffectPreset->id(), pEffectPreset->backendType());
            m_defaultPresets.insert(pManifest, pEffectPreset);
        }
        file.close();
    }

    // If no preset was found, generate one from the manifest
    for (const auto& pManifest : m_pBackendManager->getManifests()) {
        if (!m_defaultPresets.contains(pManifest)) {
            m_defaultPresets.insert(pManifest,
                    EffectPresetPointer(new EffectPreset(pManifest)));
        }
    }
}

void EffectsManager::saveDefaultForEffect(EffectPresetPointer pEffectPreset) {
    if (pEffectPreset->isEmpty()) {
        return;
    }

    const auto pManifest = m_pBackendManager->getManifest(
            pEffectPreset->id(), pEffectPreset->backendType());
    m_defaultPresets.insert(pManifest, pEffectPreset);

    QDomDocument doc(EffectXml::Effect);
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    doc.appendChild(pEffectPreset->toXml(&doc));

    QString path(m_pConfig->getSettingsPath() + kEffectDefaultsDirectory);
    QDir effectsDefaultsDir(path);
    if (!effectsDefaultsDir.exists()) {
        effectsDefaultsDir.mkpath(path);
    }

    // The file name does not matter as long as it is unique. The actual id string
    // is safely stored in the UTF8 document, regardless of what the filesystem
    // supports for file names.
    QString fileName = pEffectPreset->id();
    // LV2 ids are URLs
    fileName.replace("/", "-");
    QStringList forbiddenCharacters;
    forbiddenCharacters << "<"
                        << ">"
                        << ":"
                        << "\""
                        << "\'"
                        << "|"
                        << "?"
                        << "*"
                        << "\\";
    for (const auto& character : forbiddenCharacters) {
        fileName.remove(character);
    }
    QFile file(path + "/" + fileName + ".xml");
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}

void EffectsManager::saveDefaultForEffect(int unitNumber, int effectNumber) {
    auto pSlot = m_standardEffectChainSlots.at(unitNumber)->getEffectSlot(effectNumber);
    EffectPresetPointer pPreset(new EffectPreset(pSlot));
    saveDefaultForEffect(pPreset);
}

void EffectsManager::savePresetFromStandardEffectChain(int chainNumber) {
    StandardEffectChainSlotPointer pStandardChainSlot = m_standardEffectChainSlots.at(chainNumber);
    EffectChainSlot* genericChainSlot = static_cast<EffectChainSlot*>(pStandardChainSlot.get());
    EffectChainPresetPointer pPreset(new EffectChainPreset(genericChainSlot));
    m_pChainPresetManager->savePreset(pPreset);
}

void EffectsManager::readEffectsXml() {
    QStringList deckStrings;
    for (auto it = m_quickEffectChainSlots.begin(); it != m_quickEffectChainSlots.end(); it++) {
        deckStrings << it.key();
    }
    EffectsXmlData data = m_pChainPresetManager->readEffectsXml(deckStrings);

    for (int i = 0; i < data.standardEffectChainPresets.size(); i++) {
        m_standardEffectChainSlots.value(i)->loadChainPreset(data.standardEffectChainPresets.at(i));
    }

    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        m_quickEffectChainSlots.value(it.key())->loadChainPreset(it.value());
    }
}

void EffectsManager::saveEffectsXml() {
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

    m_pChainPresetManager->saveEffectsXml(EffectsXmlData{
            quickEffectChainPresets, standardEffectChainPresets});
}
