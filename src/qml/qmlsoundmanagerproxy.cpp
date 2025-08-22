#include "qmlsoundmanagerproxy.h"

#include <qqmlengine.h>

#include <memory>

#include "moc_qmlsoundmanagerproxy.cpp"
#include "qml_owned_ptr.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/assert.h"
#include "util/scopedoverridecursor.h"

namespace mixxx {
namespace qml {

namespace {
const QString kAppGroup = QStringLiteral("[App]");
const ConfigKey kKeylockEngineCfgkey =
        ConfigKey(kAppGroup, QStringLiteral("keylock_engine"));

} // namespace

uint QmlSoundInputDeviceProxy::getChannelCount() const {
    return m_pInternal->getNumInputChannels();
}
uint QmlSoundOutputDeviceProxy::getChannelCount() const {
    return m_pInternal->getNumOutputChannels();
}
SoundDeviceId QmlSoundDeviceProxy::getDeviceId() const {
    return m_pInternal->getDeviceId();
}

QList<QmlSoundDeviceConnection*> QmlSoundInputDeviceProxy::connections(
        mixxx::qml::QmlSoundManagerProxy* manager) {
    DEBUG_ASSERT(qml_owned_ptr<mixxx::qml::QmlSoundManagerProxy>(manager));
    QList<QmlSoundDeviceConnection*> connections;

    auto pManager = manager->internal();
    auto config = pManager->getConfig();

    const auto inputDeviceMap = config.getInputs();
    for (auto it = inputDeviceMap.cbegin(); it != inputDeviceMap.cend(); ++it) {
        if (it.key() == getDeviceId()) {
            connections.push_back(make_qml_owned<QmlSoundDeviceConnection>(
                    std::make_unique<AudioInput>(it.value()), this));
        }
    }
    return connections;
}

QList<QmlSoundDeviceConnection*> QmlSoundOutputDeviceProxy::connections(
        mixxx::qml::QmlSoundManagerProxy* manager) {
    DEBUG_ASSERT(qml_owned_ptr<mixxx::qml::QmlSoundManagerProxy>(manager));
    QList<QmlSoundDeviceConnection*> connections;

    auto pManager = manager->internal();
    auto config = pManager->getConfig();
    const auto ouputDeviceMap = config.getOutputs();
    for (auto it = ouputDeviceMap.cbegin(); it != ouputDeviceMap.cend(); ++it) {
        if (it.key() == getDeviceId()) {
            connections.push_back(make_qml_owned<QmlSoundDeviceConnection>(
                    std::make_unique<AudioOutput>(it.value()), this));
        }
    }
    return connections;
}

int QmlSoundDeviceConnection::getType() const {
    return static_cast<int>(m_audioPath->getType());
}

uchar QmlSoundDeviceConnection::getChannelGroup() const {
    auto group = m_audioPath->getChannelGroup();
    return group.getChannelBase();
}
uchar QmlSoundDeviceConnection::getIndex() const {
    return m_audioPath->getIndex();
}

QmlSoundManagerProxy::QmlSoundManagerProxy(
        std::shared_ptr<SoundManager> pSoundManager,
        QObject* parent)
        : QObject(parent),
          m_pSoundManager(pSoundManager),
          m_keylockEngine(kKeylockEngineCfgkey),
          m_config(m_pSoundManager->getConfig()) {
    connect(m_pSoundManager.get(), &SoundManager::devicesClosed, this, [this]() {
        SoundDeviceStatus status = SoundDeviceStatus::Ok;
        {
            ScopedWaitCursor cursor;

            if (m_commitInProgress.fetchAndStoreRelease(0) != 1) {
                return;
            }

            status = m_pSoundManager->setConfig(m_config);
        }
        if (status != SoundDeviceStatus::Ok) {
            emit committed(m_pSoundManager->getLastErrorMessage(status));
        } else {
            emit committed();
        }
        m_config = m_pSoundManager->getConfig();
    });
}

// static
QmlSoundManagerProxy* QmlSoundManagerProxy::create(
        QQmlEngine* pQmlEngine,
        QJSEngine*) {
    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pSoundManager) {
        qWarning() << "SoundManager hasn't been registered yet";
        return nullptr;
    }
    return make_qml_owned<QmlSoundManagerProxy>(s_pSoundManager, pQmlEngine);
}

QList<QString> QmlSoundManagerProxy::getHostAPIList() const {
    return m_pSoundManager->getHostAPIList();
}

QList<QmlSoundDeviceProxy*> QmlSoundManagerProxy::availableInputDevices(const QString& filterAPI) {
    QList<QmlSoundDeviceProxy*> devices;

    for (const auto& device : m_pSoundManager->getDeviceList(filterAPI, false, true)) {
        devices.push_back(make_qml_owned<QmlSoundInputDeviceProxy>(device, this));
    }

    return devices;
}

QList<QmlSoundDeviceProxy*> QmlSoundManagerProxy::availableOutputDevices(const QString& filterAPI) {
    QList<QmlSoundDeviceProxy*> devices;

    for (const auto& device : m_pSoundManager->getDeviceList(filterAPI, true, false)) {
        devices.push_back(make_qml_owned<QmlSoundOutputDeviceProxy>(device, this));
    }

    return devices;
}

QList<EngineBuffer::KeylockEngine> QmlSoundManagerProxy::getKeylockEngines() const {
    QList<EngineBuffer::KeylockEngine> list;
    for (const auto engine : EngineBuffer::kKeylockEngines) {
        if (EngineBuffer::isKeylockEngineAvailable(engine)) {
            list.append(engine);
        }
    }
    return list;
}

void QmlSoundManagerProxy::setKeylockEngine(EngineBuffer::KeylockEngine keylockEngine) {
    m_keylockEngine.set(static_cast<double>(keylockEngine));
    m_pSoundManager->userSettings()->setValue(kKeylockEngineCfgkey, keylockEngine);
}

EngineBuffer::KeylockEngine QmlSoundManagerProxy::getKeylockEngine() const {
    return m_pSoundManager->userSettings()
            ->getValue<EngineBuffer::KeylockEngine>(
                    kKeylockEngineCfgkey, EngineBuffer::defaultKeylockEngine());
}

QString QmlSoundManagerProxy::getAPI() const {
    return m_config.getAPI();
}
void QmlSoundManagerProxy::setAPI(const QString& api) {
    m_config.setAPI(api);
}

unsigned int QmlSoundManagerProxy::getSyncBuffers() const {
    return m_config.getSyncBuffers();
}

void QmlSoundManagerProxy::setSyncBuffers(unsigned int syncBuffers) {
    m_config.setSyncBuffers(syncBuffers);
}

uint32_t QmlSoundManagerProxy::getSampleRate() const {
    return m_config.getSampleRate();
}

void QmlSoundManagerProxy::setSampleRate(uint32_t sampleRate) {
    m_config.setSampleRate(mixxx::audio::SampleRate(sampleRate));
}

QList<uint32_t> QmlSoundManagerProxy::getSampleRates(const QString& filterAPI) const {
    QList<uint32_t> sampleRates;
    for (const auto& sampleRate : m_pSoundManager->getSampleRates(filterAPI)) {
        if (sampleRate.isValid()) {
            sampleRates.append(sampleRate);
        }
    }
    return sampleRates;
}

bool QmlSoundManagerProxy::getForceNetworkClock() const {
    return m_config.getForceNetworkClock();
}

void QmlSoundManagerProxy::setForceNetworkClock(bool force) {
    m_config.setForceNetworkClock(force);
}

unsigned int QmlSoundManagerProxy::getAudioBufferSizeIndex() const {
    return m_config.getAudioBufferSizeIndex();
}

void QmlSoundManagerProxy::setAudioBufferSizeIndex(unsigned int latency) {
    m_config.setAudioBufferSizeIndex(latency);
}

void QmlSoundManagerProxy::addOutput(QmlSoundOutputDeviceProxy* device,
        int type,
        unsigned char channelGroup,
        unsigned char index) {
    VERIFY_OR_DEBUG_ASSERT(device && qml_owned_ptr<QmlSoundOutputDeviceProxy>(device)) {
        return;
    }
    m_config.addOutput(device->getDeviceId(),
            AudioOutput(static_cast<AudioPathType>(type),
                    channelGroup,
                    mixxx::audio::ChannelCount::stereo(),
                    index));
}

void QmlSoundManagerProxy::addInput(QmlSoundInputDeviceProxy* device,
        int type,
        unsigned char channelGroup,
        unsigned char index) {
    VERIFY_OR_DEBUG_ASSERT(device && qml_owned_ptr<QmlSoundInputDeviceProxy>(device)) {
        return;
    }
    m_config.addInput(device->getDeviceId(),
            AudioInput(static_cast<AudioPathType>(type),
                    channelGroup,
                    mixxx::audio::ChannelCount::stereo(),
                    index));
}

void QmlSoundManagerProxy::clearOutputs() {
    m_config.clearOutputs();
}

void QmlSoundManagerProxy::clearInputs() {
    m_config.clearInputs();
}

bool QmlSoundManagerProxy::hasMicInputs() {
    return m_config.hasMicInputs();
}

std::shared_ptr<SoundManager> QmlSoundManagerProxy::internal() const {
    return m_pSoundManager;
}

void QmlSoundManagerProxy::commit() {
    m_commitInProgress.storeRelease(1);
    m_pSoundManager->closeActiveConfig(true);
}

} // namespace qml
} // namespace mixxx
