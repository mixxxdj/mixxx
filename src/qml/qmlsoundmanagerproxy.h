#pragma once

#include <qglobal.h>
#include <qtmetamacros.h>

#include <QDialog>
#include <QObject>
#include <QQmlEngine>
#include <memory>

#include "control/pollingcontrolproxy.h"
#include "engine/enginebuffer.h"
#include "qml_owned_ptr.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;

namespace mixxx {
namespace qml {

class QmlSoundManagerProxy;
class QmlSoundDeviceConnection;
class QmlSoundDeviceProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString displayName READ getDisplayName CONSTANT)
    Q_PROPERTY(uint channelCount READ getChannelCount CONSTANT)
    QML_ANONYMOUS
  public:
    explicit QmlSoundDeviceProxy(SoundDevicePointer pInternal, QObject* parent)
            : QObject(parent),
              m_pInternal(std::move(pInternal)) {
    }

    QString getDisplayName() const {
        return m_pInternal->getDisplayName();
    }

    virtual uint getChannelCount() const = 0;
    SoundDeviceId getDeviceId() const;

    Q_INVOKABLE virtual QList<mixxx::qml::QmlSoundDeviceConnection*> connections(
            mixxx::qml::QmlSoundManagerProxy* manager) = 0;

  protected:
    SoundDevicePointer m_pInternal;
};

class QmlSoundInputDeviceProxy : public QmlSoundDeviceProxy {
    Q_OBJECT
    QML_NAMED_ELEMENT(InputDevice)
    QML_UNCREATABLE("Use Mixxx.SoundManager to get devices")
  public:
    explicit QmlSoundInputDeviceProxy(SoundDevicePointer pInternal, QObject* parent)
            : QmlSoundDeviceProxy(std::move(pInternal), parent) {
    }
    uint getChannelCount() const override;
    Q_INVOKABLE QList<mixxx::qml::QmlSoundDeviceConnection*> connections(
            mixxx::qml::QmlSoundManagerProxy* manager) override;
};

class QmlSoundOutputDeviceProxy : public QmlSoundDeviceProxy {
    Q_OBJECT
    QML_NAMED_ELEMENT(OutputDevice)
    QML_UNCREATABLE("Use Mixxx.SoundManager to get devices")
  public:
    explicit QmlSoundOutputDeviceProxy(SoundDevicePointer pInternal, QObject* parent)
            : QmlSoundDeviceProxy(std::move(pInternal), parent) {
    }
    uint getChannelCount() const override;
    Q_INVOKABLE QList<mixxx::qml::QmlSoundDeviceConnection*> connections(
            mixxx::qml::QmlSoundManagerProxy* manager) override;
};

class QmlSoundDeviceConnection : public QObject {
    Q_OBJECT
    Q_PROPERTY(int type READ getType CONSTANT)
    Q_PROPERTY(uchar channelGroup READ getChannelGroup CONSTANT)
    Q_PROPERTY(uchar index READ getIndex CONSTANT)
    QML_ANONYMOUS
  public:
    QmlSoundDeviceConnection(std::unique_ptr<AudioPath> path, QObject* parent = nullptr)
            : QObject(parent),
              m_audioPath(std::move(path)) {
    }

    int getType() const;
    uchar getChannelGroup() const;
    uchar getIndex() const;

  private:
    std::unique_ptr<AudioPath> m_audioPath;
};

class QmlSoundDeviceInputConnection : public QmlSoundDeviceConnection {
    Q_OBJECT
    QML_NAMED_ELEMENT(InputConnection)
    QML_UNCREATABLE("Use Mixxx.SoundDevice to get connections")
  public:
    QmlSoundDeviceInputConnection(std::unique_ptr<AudioPath> path, QObject* parent = nullptr)
            : QmlSoundDeviceConnection(std::move(path), parent) {
    }
};

class QmlSoundDeviceOutputConnection : public QmlSoundDeviceConnection {
    Q_OBJECT
    QML_NAMED_ELEMENT(OutputConnection)
    QML_UNCREATABLE("Use Mixxx.SoundDevice to get connections")
  public:
    QmlSoundDeviceOutputConnection(std::unique_ptr<AudioPath> path, QObject* parent = nullptr)
            : QmlSoundDeviceConnection(std::move(path), parent) {
    }
};

class QmlSoundManagerProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SoundManager)
    QML_SINGLETON
  public:
    explicit QmlSoundManagerProxy(
            std::shared_ptr<SoundManager> pSoundManager,
            QObject* parent = nullptr);

    Q_INVOKABLE QList<QString> getHostAPIList() const;
    Q_INVOKABLE QList<mixxx::qml::QmlSoundDeviceProxy*> availableInputDevices(
            const QString& filterAPI);
    Q_INVOKABLE QList<mixxx::qml::QmlSoundDeviceProxy*> availableOutputDevices(
            const QString& filterAPI);

    Q_INVOKABLE QList<EngineBuffer::KeylockEngine> getKeylockEngines() const;
    Q_INVOKABLE EngineBuffer::KeylockEngine getKeylockEngine() const;
    Q_INVOKABLE void setKeylockEngine(EngineBuffer::KeylockEngine);
    Q_INVOKABLE QString getAPI() const;
    Q_INVOKABLE void setAPI(const QString& api);
    Q_INVOKABLE unsigned int getSyncBuffers() const;
    Q_INVOKABLE void setSyncBuffers(unsigned int syncBuffers);
    Q_INVOKABLE uint32_t getSampleRate() const;
    Q_INVOKABLE void setSampleRate(uint32_t sampleRate);
    Q_INVOKABLE bool getForceNetworkClock() const;
    Q_INVOKABLE void setForceNetworkClock(bool force);
    Q_INVOKABLE unsigned int getAudioBufferSizeIndex() const;
    Q_INVOKABLE void setAudioBufferSizeIndex(unsigned int latency);
    Q_INVOKABLE QList<uint32_t> getSampleRates(const QString& filterAPI) const;
    Q_INVOKABLE void addOutput(mixxx::qml::QmlSoundOutputDeviceProxy* device,
            int type,
            unsigned char channelGroup,
            unsigned char index);
    Q_INVOKABLE void addInput(mixxx::qml::QmlSoundInputDeviceProxy* device,
            int type,
            unsigned char channelGroup,
            unsigned char index);
    Q_INVOKABLE void clearOutputs();
    Q_INVOKABLE void clearInputs();
    Q_INVOKABLE bool hasMicInputs();

    std::shared_ptr<SoundManager> internal() const;
    Q_INVOKABLE void commit();

    static QmlSoundManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerManager(std::shared_ptr<SoundManager> pManager) {
        s_pSoundManager = std::move(pManager);
    }

  signals:
    void committed(const QString& error = {});

  private:
    static inline std::shared_ptr<SoundManager> s_pSoundManager;

    PollingControlProxy m_keylockEngine;

    std::shared_ptr<SoundManager> m_pSoundManager;
    SoundManagerConfig m_config;
    QAtomicInt m_commitInProgress;
};

} // namespace qml
} // namespace mixxx
