#include "effects/effectsmanager.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMetaType>
#include <algorithm>

#include "effects/builtin/builtinbackend.h"
#include "effects/lv2/lv2backend.h"
#include "effects/effectslot.h"
#include "effects/effectxmlelements.h"
#include "effects/presets/effectchainpreset.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffectsmanager.h"
#include "util/assert.h"

namespace {
const QString kStandardEffectRackGroup = "[EffectRack1]";
const QString kOutputEffectRackGroup = "[OutputEffectRack]";
const QString kQuickEffectRackGroup = "[QuickEffectRack1]";
const QString kEqualizerEffectRackGroup = "[EqualizerRack1]";
const QString kEffectGroupSeparator = "_";
const QString kGroupClose = "]";
const unsigned int kEffectMessagPipeFifoSize = 2048;
} // anonymous namespace

EffectsManager::EffectsManager(QObject* pParent,
        UserSettingsPointer pConfig,
        ChannelHandleFactory* pChannelHandleFactory)
        : QObject(pParent),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_nextRequestId(0),
          m_loEqFreq(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040),
          m_hiEqFreq(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040),
          m_underDestruction(false),
          m_pConfig(pConfig) {
    qRegisterMetaType<EffectChainMixMode>("EffectChainMixMode");
    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                kEffectMessagPipeFifoSize, kEffectMessagPipeFifoSize);

    m_pRequestPipe.reset(requestPipes.first);
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);

    m_pNumEffectsAvailable = new ControlObject(ConfigKey("[Master]", "num_effectsavailable"));
    m_pNumEffectsAvailable->setReadOnly();

    addEffectsBackend(EffectsBackendPointer(new BuiltInBackend()));
#ifdef __LILV__
    addEffectsBackend(EffectsBackendPointer(new LV2Backend()));
#endif
}

EffectsManager::~EffectsManager() {
    m_underDestruction = true;

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
    processEffectsResponses();

    m_effectsBackends.clear();
    for (QHash<qint64, EffectsRequest*>::iterator it = m_activeRequests.begin();
         it != m_activeRequests.end();) {
        delete it.value();
        it = m_activeRequests.erase(it);
    }

    // delete m_pHiEqFreq;
    // delete m_pLoEqFreq;
    delete m_pNumEffectsAvailable;
    // Safe because the Engine is deleted before EffectsManager. Also, it holds
    // a bare pointer to m_pRequestPipe so it is critical that it does not
    // outlast us.
    delete m_pEngineEffectsManager;
}

bool alphabetizeEffectManifests(EffectManifestPointer pManifest1,
        EffectManifestPointer pManifest2) {
    // Sort built-in effects first before external plugins
    int backendNameComparision = static_cast<int>(pManifest1->backendType()) -
            static_cast<int>(pManifest2->backendType());
    int displayNameComparision = QString::localeAwareCompare(
            pManifest1->displayName(), pManifest2->displayName());
    return (backendNameComparision ? (backendNameComparision < 0) : (displayNameComparision < 0));
}

void EffectsManager::addEffectsBackend(EffectsBackendPointer pBackend) {
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return;
    }
    m_effectsBackends.insert(pBackend->getType(), pBackend);

    QList<QString> backendEffects = pBackend->getEffectIds();
    for (const QString& effectId : backendEffects) {
        m_availableEffectManifests.append(pBackend->getManifest(effectId));
    }

    m_pNumEffectsAvailable->forceSet(m_availableEffectManifests.size());

    std::sort(m_availableEffectManifests.begin(), m_availableEffectManifests.end(),
          alphabetizeEffectManifests);
}

bool EffectsManager::isAdoptMetaknobValueEnabled() const {
    return m_pConfig->getValue(ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
}

void EffectsManager::registerInputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredInputChannels.contains(handle_group)) {
        return;
    }
    m_registeredInputChannels.insert(handle_group);

    foreach (EffectChainSlotPointer pChainSlot, m_standardEffectChainSlots) {
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

void EffectsManager::loadQuickEffect(const QString& deckGroup,
        const int iEffectSlotNumber, const EffectManifestPointer pManifest) {
    auto pChainSlot = m_quickEffectChainSlots.value(deckGroup);
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return;
    }
    loadEffect(pChainSlot, iEffectSlotNumber, pManifest);
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
            createProcessor(pManifest),
            pPreset,
            adoptMetaknobFromPreset);
}

std::unique_ptr<EffectProcessor> EffectsManager::createProcessor(
        const EffectManifestPointer pManifest) {
    if (!pManifest) {
        // This can be a valid request to unload an effect, so do not DEBUG_ASSERT.
        return std::unique_ptr<EffectProcessor>(nullptr);
    }
    EffectsBackendPointer pBackend = m_effectsBackends.value(pManifest->backendType());
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return std::unique_ptr<EffectProcessor>(nullptr);
    }
    return pBackend->createProcessor(pManifest);
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

// This needs to be in EffectsManager rather than EffectChainSlot because it
// needs access to the EffectsBackends.
void EffectsManager::loadEffectChainPreset(EffectChainSlot* pChainSlot,
        EffectChainPresetPointer pPreset) {
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pPreset) {
        return;
    }
    // Set the superknob before loading the effects so it does not change their
    // metaknobs
    pChainSlot->setSuperParameter(pPreset->superKnob());

    int effectSlot = 0;
    for (const auto& pEffectPreset : pPreset->effectPresets()) {
        if (pEffectPreset->isEmpty()) {
            pChainSlot->loadEffect(
                    effectSlot,
                    nullptr,
                    nullptr,
                    nullptr,
                    true);
            effectSlot++;
            continue;
        }
        EffectsBackendPointer pBackend = m_effectsBackends.value(pEffectPreset->backendType());
        VERIFY_OR_DEBUG_ASSERT(pBackend) {
            effectSlot++;
            continue;
        }
        EffectManifestPointer pManifest = pBackend->getManifest(pEffectPreset->id());
        pChainSlot->loadEffect(
                effectSlot,
                pManifest,
                createProcessor(pManifest),
                pEffectPreset,
                true);
        effectSlot++;
    }
    pChainSlot->setMixMode(pPreset->mixMode());
    pChainSlot->setName(pPreset->name());
}

void EffectsManager::loadPresetToStandardChain(int chainNumber, EffectChainPresetPointer pPreset) {
    loadEffectChainPreset(m_standardEffectChainSlots.at(chainNumber).get(), pPreset);
}

void EffectsManager::importChainPreset() {
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
            tr("Import effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + " (*.xml)");

    QString importFailed = tr("Error importing effect chain preset");
    for (int i = 0; i < fileNames.size(); ++i) {
        QString filePath = fileNames.at(i);
        QDomDocument doc;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(nullptr, importFailed, importFailed + " " + filePath);
            continue;
        } else if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::critical(nullptr, importFailed, importFailed + " " + filePath);
            continue;
        }
        file.close();

        EffectChainPresetPointer pChainPreset(new EffectChainPreset(doc.documentElement()));
        if (!pChainPreset->isEmpty()) {
            if (m_effectChainPresets.contains(pChainPreset->name())) {
                bool okay = false;
                QString newName = QInputDialog::getText(nullptr,
                        tr("Rename effect chain preset"),
                        tr("An effect chain preset with the name") + " " + pChainPreset->name() + " " + tr("already exists. Choose a new name for the imported effect chain preset:"),
                        QLineEdit::Normal,
                        QString(),
                        &okay);
                if (!okay) {
                    continue;
                }
                pChainPreset->setName(newName);
            }

            // An imported chain preset might contain an LV2 plugin that the user does not
            // have installed.
            for (const auto pEffectPreset : pChainPreset->effectPresets()) {
                if (pEffectPreset == nullptr || pEffectPreset->isEmpty()) {
                    continue;
                }

                bool effectSupported = false;
                for (EffectManifestPointer pManifest : m_availableEffectManifests) {
                    if (pManifest->id() == pEffectPreset->id() &&
                            pManifest->backendType() == pEffectPreset->backendType()) {
                        effectSupported = true;
                        break;
                    }
                }
                if (!effectSupported) {
                    QMessageBox::critical(nullptr, importFailed, tr("The effect chain imported from") + " " + filePath + " " + tr("contains an effect that Mixxx does not support") + ":\n\n" + pEffectPreset->id() + "\n\n" + tr("If you load this chain preset, the unsupported effect will not be loaded with it."));
                }
            }

            m_effectChainPresets.insert(pChainPreset->name(), pChainPreset);
            m_effectChainPresetsSorted.append(pChainPreset);
        } else {
            QMessageBox::critical(nullptr, importFailed, importFailed + " " + filePath);
        }
    }
}

void EffectsManager::exportChainPreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return;
    }

    EffectChainPresetPointer pChainPreset = m_effectChainPresets.value(chainPresetName);
    VERIFY_OR_DEBUG_ASSERT(!pChainPreset->isEmpty()) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(nullptr,
            tr("Save effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + " (*.xml)");

    QFile file(fileName);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        file.close();
        QMessageBox::critical(nullptr,
                tr("Error exporting effect chain preset"),
                tr("Could not save effect chain preset") + " " + chainPresetName + tr("to file") + " " + fileName);
        return;
    }

    QDomDocument doc(EffectXml::Chain);
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    doc.appendChild(pChainPreset->toXml(&doc));
    file.write(doc.toString().toUtf8());
    file.close();
}

void EffectsManager::renameChainPreset(const QString& oldName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(oldName)) {
        return;
    }

    bool okay = false;
    QString newName = QInputDialog::getText(nullptr,
            tr("Rename effect chain preset"),
            tr("New name effect chain preset") + " " + oldName,
            QLineEdit::Normal,
            oldName,
            &okay);
    if (!okay) {
        return;
    }

    EffectChainPresetPointer pChainPreset = m_effectChainPresets.take(oldName);
    int index = m_effectChainPresetsSorted.indexOf(pChainPreset);
    pChainPreset->setName(newName);
    m_effectChainPresets.insert(newName, pChainPreset);
    m_effectChainPresetsSorted.removeAt(index);
    m_effectChainPresetsSorted.insert(index, pChainPreset);
}

void EffectsManager::deleteChainPreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return;
    }
    auto pressedButton = QMessageBox::question(nullptr,
            tr("Remove effect chain preset"),
            tr("Are you sure you want to delete the effect chain preset") + " " + chainPresetName + "?");
    if (pressedButton != QMessageBox::Yes) {
        return;
    }

    EffectChainPresetPointer pChainPreset = m_effectChainPresets.take(chainPresetName);
    m_effectChainPresetsSorted.removeAll(pChainPreset);
}

const QList<EffectManifestPointer> EffectsManager::getAvailableEffectManifestsFiltered(
        EffectManifestFilterFnc filter) const {
    if (filter == nullptr) {
        return m_availableEffectManifests;
    }

    QList<EffectManifestPointer> list;
    for (const auto& pManifest : m_availableEffectManifests) {
        if (filter(pManifest.data())) {
            list.append(pManifest);
        }
    }
    return list;
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

void EffectsManager::getEffectManifestAndBackend(
        const QString& effectId,
        EffectManifestPointer* ppManifest, EffectsBackend** ppBackend) const {
    for (const auto& pBackend : m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            *ppManifest = pBackend->getManifest(effectId);
            *ppBackend = pBackend.data();
        }
    }
}

EffectManifestPointer EffectsManager::getManifestFromUniqueId(const QString& uid) const {
    if (kEffectDebugOutput) {
        //qDebug() << "EffectsManager::getManifestFromUniqueId" << uid;
    }
    if (uid.isEmpty()) {
        // Do not DEBUG_ASSERT, this may be a valid request for a nullptr to
        // unload an effect.
        return EffectManifestPointer();
    }
    int delimiterIndex = uid.lastIndexOf(" ");
    EffectBackendType backendType = EffectManifest::backendTypeFromString(
            uid.mid(delimiterIndex+1));
    VERIFY_OR_DEBUG_ASSERT(backendType != EffectBackendType::Unknown) {
        // Mixxx 2.0 - 2.2 did not store the backend type in mixxx.cfg,
        // so this code will be executed once when upgrading to Mixxx 2.3.
        // This debug assertion is safe to ignore in that case. If it is
        // triggered at any later time, there is a bug somewhere.
        // Do not manipulate the string passed to this function, just pass
        // it directly to BuiltInBackend.
        return m_effectsBackends.value(EffectBackendType::BuiltIn)->getManifest(uid);
    }
    return m_effectsBackends.value(backendType)->getManifest(
            uid.mid(-1, delimiterIndex+1));
}

EffectManifestPointer EffectsManager::getManifest(
        const QString& id, EffectBackendType backendType) const {
    return m_effectsBackends.value(backendType)->getManifest(id);
}

void EffectsManager::addStandardEffectChainSlots() {
    for (int i = 0; i < EffectsManager::kNumStandardEffectChains; ++i) {
        VERIFY_OR_DEBUG_ASSERT(!m_effectChainSlotsByGroup.contains(
                StandardEffectChainSlot::formatEffectChainSlotGroup(i))) {
            continue;
        }

        auto pChainSlot = StandardEffectChainSlotPointer(
            new StandardEffectChainSlot(i, this));
        connectChainSlotSignals(pChainSlot);

        m_standardEffectChainSlots.append(pChainSlot);
        m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
    }
}

void EffectsManager::addOutputEffectChainSlot() {
    m_outputEffectChainSlot = OutputEffectChainSlotPointer(new OutputEffectChainSlot(this));
    connectChainSlotSignals(m_outputEffectChainSlot);
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
            new EqualizerEffectChainSlot(deckGroupName, this));
    connectChainSlotSignals(pChainSlot);

    m_equalizerEffectChainSlots.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

void EffectsManager::addQuickEffectChainSlot(const QString& deckGroupName) {
    VERIFY_OR_DEBUG_ASSERT(!m_quickEffectChainSlots.contains(
            QuickEffectChainSlot::formatEffectChainSlotGroup(deckGroupName))) {
        return;
    }

    auto pChainSlot = QuickEffectChainSlotPointer(
        new QuickEffectChainSlot(deckGroupName, this));
    connectChainSlotSignals(pChainSlot);

    m_quickEffectChainSlots.insert(deckGroupName, pChainSlot);
    m_effectChainSlotsByGroup.insert(pChainSlot->group(), pChainSlot);
}

void EffectsManager::connectChainSlotSignals(EffectChainSlotPointer pChainSlot) {
    connect(pChainSlot.get(),
            &EffectChainSlot::loadChainPreset,
            this,
            &EffectsManager::loadChainPresetFromList);
    connect(pChainSlot.get(),
            &EffectChainSlot::selectChainPreset,
            this,
            &EffectsManager::loadChainPresetSelector);
}

void EffectsManager::loadChainPresetFromList(EffectChainSlot* pChainSlot, int listIndex) {
    if (listIndex < 0 || listIndex >= m_effectChainPresets.size()) {
        listIndex = 0;
    }
    loadEffectChainPreset(pChainSlot,
            m_effectChainPresetsSorted.at(listIndex));
}

void EffectsManager::loadChainPresetSelector(EffectChainSlot* pChainSlot, int delta) {
    int listIndex = 0;
    if (m_effectChainPresets.contains(pChainSlot->name())) {
        EffectChainPresetPointer pLoadedPreset = m_effectChainPresets.value(pChainSlot->name());
        listIndex = m_effectChainPresetsSorted.indexOf(pLoadedPreset);
    }

    listIndex += delta;
    if (listIndex >= m_effectChainPresetsSorted.size()) {
        listIndex = 0;
    } else if (listIndex < 0) {
        listIndex = m_effectChainPresetsSorted.size() - 1;
    }

    loadEffectChainPreset(pChainSlot,
            m_effectChainPresetsSorted.at(listIndex));
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
                                                pManifest, alphabetizeEffectManifests);
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
    loadEffectChainPresets();

    readEffectsXml();
}

bool EffectsManager::writeRequest(EffectsRequest* request) {
    if (m_underDestruction) {
        // Catch all delete Messages since the engine is already down
        // and we cannot wait for a communication cycle
        collectGarbage(request);
    }

    if (m_pRequestPipe.isNull()) {
        delete request;
        return false;
    }

    // This is effectively only garbage collection at this point so only deal
    // with responses when writing new requests.
    processEffectsResponses();

    request->request_id = m_nextRequestId++;
    // TODO(XXX) use preallocated requests to avoid delete calls from engine
    if (m_pRequestPipe->writeMessage(request)) {
        m_activeRequests[request->request_id] = request;
        return true;
    }
    delete request;
    return false;
}

void EffectsManager::processEffectsResponses() {
    if (m_pRequestPipe.isNull()) {
        return;
    }

    EffectsResponse response;
    while (m_pRequestPipe->readMessage(&response)) {
        QHash<qint64, EffectsRequest*>::iterator it =
                m_activeRequests.find(response.request_id);

        VERIFY_OR_DEBUG_ASSERT(it != m_activeRequests.end()) {
            qWarning() << debugString()
                       << "WARNING: EffectsResponse with an inactive request_id:"
                       << response.request_id;
        }

        while (it != m_activeRequests.end() &&
               it.key() == response.request_id) {
            EffectsRequest* pRequest = it.value();

            // Don't check whether the response was successful here because
            // specific errors should be caught with DEBUG_ASSERTs in
            // EngineEffectsManager and functions it calls to handle requests.

            collectGarbage(pRequest);

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }
}

void EffectsManager::collectGarbage(const EffectsRequest* pRequest) {
    if (pRequest->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
        }
        delete pRequest->RemoveEffectFromChain.pEffect;
    } else if (pRequest->type == EffectsRequest::REMOVE_EFFECT_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectChain.pChain;
        }
        delete pRequest->RemoveEffectChain.pChain;
    } else if (pRequest->type == EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "deleting states for input channel"
                     << pRequest->DisableInputChannelForChain.pChannelHandle
                     << "for EngineEffectChain" << pRequest->pTargetChain;
        }
        pRequest->pTargetChain->deleteStatesForInputChannel(
                pRequest->DisableInputChannelForChain.pChannelHandle);
    }
}

void EffectsManager::loadDefaultEffectPresets() {
    // Load saved defaults from settings directory
    QString dirPath(m_pConfig->getSettingsPath() + "/effects/defaults");
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
            EffectManifestPointer pManifest = getManifest(pEffectPreset->id(), pEffectPreset->backendType());
            m_defaultPresets.insert(pManifest, pEffectPreset);
        }
        file.close();
    }

    // If no preset was found, generate one from the manifest
    for (const auto pBackend : m_effectsBackends) {
        for (const auto pManifest : pBackend->getManifests()) {
            if (!m_defaultPresets.contains(pManifest)) {
                m_defaultPresets.insert(pManifest,
                        EffectPresetPointer(new EffectPreset(pManifest)));
            }
        }
    }
}

void EffectsManager::saveDefaultForEffect(EffectPresetPointer pEffectPreset) {
    if (pEffectPreset->isEmpty()) {
        return;
    }

    const auto pBackend = m_effectsBackends.value(pEffectPreset->backendType());
    const auto pManifest = pBackend->getManifest(pEffectPreset->id());
    m_defaultPresets.insert(pManifest, pEffectPreset);

    QDomDocument doc(EffectXml::Effect);
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    doc.appendChild(pEffectPreset->toXml(&doc));

    QString path(m_pConfig->getSettingsPath() + "/effects/defaults");
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

void EffectsManager::loadEffectChainPresets() {
    QString dirPath(m_pConfig->getSettingsPath() + "/effects/chains");
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
        EffectChainPresetPointer pEffectChainPreset(new EffectChainPreset(doc.documentElement()));
        if (!pEffectChainPreset->isEmpty()) {
            m_effectChainPresets.insert(pEffectChainPreset->name(), pEffectChainPreset);
            m_effectChainPresetsSorted.append(pEffectChainPreset);
        }
        file.close();
    }
}

void EffectsManager::setChainPresetOrder(const QStringList& chainPresetList) {
    m_effectChainPresetsSorted.clear();

    for (const auto chainPresetName : chainPresetList) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
            continue;
        }
        m_effectChainPresetsSorted.append(m_effectChainPresets.value(chainPresetName));
    }

    for (const auto pChainPreset : m_effectChainPresets) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresetsSorted.contains(pChainPreset)) {
            m_effectChainPresetsSorted.append(pChainPreset);
        }
    }
}

const QString EffectsManager::getDisplayNameForEffectPreset(EffectPresetPointer pPreset) {
    QString displayName(tr("None"));
    if (pPreset == nullptr || pPreset->isEmpty()) {
        return displayName;
    }

    bool manifestFound = false;
    for (const auto pManifest : m_availableEffectManifests) {
        if (pManifest->id() == pPreset->id() &&
                pManifest->backendType() == pPreset->backendType()) {
            displayName = pManifest->name();
            manifestFound = true;
            break;
        }
    }
    DEBUG_ASSERT(manifestFound);
    return displayName;
}

void EffectsManager::savePresetFromStandardEffectChain(int chainNumber) {
    StandardEffectChainSlotPointer pStandardChainSlot = m_standardEffectChainSlots.at(chainNumber);
    EffectChainSlot* genericChainSlot = static_cast<EffectChainSlot*>(pStandardChainSlot.get());
    EffectChainPresetPointer pChainPreset(new EffectChainPreset(genericChainSlot));

    bool okay = false;
    QString name = QInputDialog::getText(nullptr,
            tr("Save preset for effect chain %1").arg(QString::number(chainNumber)),
            tr("Name for new effect chain preset"),
            QLineEdit::Normal,
            pChainPreset->name(),
            &okay);
    if (!okay) {
        return;
    }

    pChainPreset->setName(name);
    m_effectChainPresets.insert(name, pChainPreset);
    m_effectChainPresetsSorted.append(pChainPreset);

    QString path(m_pConfig->getSettingsPath() + "/effects/chains");
    QDir effectsChainsDir(path);
    if (!effectsChainsDir.exists()) {
        effectsChainsDir.mkpath(path);
    }
    // TODO: sanitize file name?
    QFile file(path + "/" + name + ".xml");
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return;
    }

    QDomDocument doc(EffectXml::Chain);
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    doc.appendChild(pChainPreset->toXml(&doc));
    file.write(doc.toString().toUtf8());
    file.close();
}

const QList<EffectChainPresetPointer> EffectsManager::getAvailableChainPresets() const {
    return m_effectChainPresetsSorted;
}

void EffectsManager::readEffectsXml() {
    QDir settingsPath(m_pConfig->getSettingsPath());
    QFile file(settingsPath.absoluteFilePath("effects.xml"));
    QDomDocument doc;

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    } else if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // Reload state of standard chains
    QDomElement root = doc.documentElement();
    QDomElement rackElement = XmlParse::selectElement(root, EffectXml::Rack);
    QDomElement chainsElement = XmlParse::selectElement(rackElement, EffectXml::ChainsRoot);
    QDomNodeList chainsList = chainsElement.elementsByTagName(EffectXml::Chain);

    for (int i=0; i<chainsList.count(); ++i) {
        QDomNode chainNode = chainsList.at(i);

        if (chainNode.isElement()) {
            QDomElement chainElement = chainNode.toElement();
            EffectChainPresetPointer pPreset(new EffectChainPreset(chainElement));
            loadEffectChainPreset(m_standardEffectChainSlots.value(i).get(), pPreset);
        }
    }

    // Reload order of custom chain presets
    QStringList chainPresetsSorted;
    QDomElement chainPresetsElement = XmlParse::selectElement(root, EffectXml::ChainPresetList);
    QDomNodeList presetNameList = chainPresetsElement.elementsByTagName(EffectXml::ChainPresetName);
    for (int i = 0; i < presetNameList.count(); ++i) {
        QDomNode presetNameNode = presetNameList.at(i);
        if (presetNameNode.isElement()) {
            chainPresetsSorted << presetNameNode.toElement().text();
        }
    }
    setChainPresetOrder(chainPresetsSorted);
}

void EffectsManager::saveEffectsXml() {
    QDomDocument doc("MixxxEffects");
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

    // Save presets for current state of standard chains
    QDomElement rootElement = doc.createElement("MixxxEffects");
    rootElement.setAttribute("schemaVersion", QString::number(EffectXml::kXmlSchemaVersion));
    doc.appendChild(rootElement);
    QDomElement rackElement = doc.createElement(EffectXml::Rack);
    rootElement.appendChild(rackElement);
    QDomElement chainsElement = doc.createElement(EffectXml::ChainsRoot);
    rackElement.appendChild(chainsElement);
    for (const auto pChainSlot : m_standardEffectChainSlots) {
        EffectChainSlot* genericChainSlot = static_cast<EffectChainSlot*>(pChainSlot.get());
        chainsElement.appendChild(EffectChainPreset(genericChainSlot).toXml(&doc));
    }

    // Save order of custom chain presets
    QDomElement chainPresetListElement = doc.createElement(EffectXml::ChainPresetList);
    for (const auto pChainPreset : m_effectChainPresetsSorted) {
        XmlParse::addElement(doc,
                chainPresetListElement,
                EffectXml::ChainPresetName,
                pChainPreset->name());
    }
    doc.appendChild(chainPresetListElement);

    QDir settingsPath(m_pConfig->getSettingsPath());
    if (!settingsPath.exists()) {
        return;
    }
    QFile file(settingsPath.absoluteFilePath("effects.xml"));
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}
