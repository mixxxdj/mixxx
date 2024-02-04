#pragma once

#ifndef SOUNDMANAGERCONFIG_FILENAME
#define SOUNDMANAGERCONFIG_FILENAME "soundconfig.xml"
#endif

#include <QString>
#include <QMultiHash>
#include <QFileInfo>

#include "soundio/soundmanagerutil.h"

class SoundManager;

class SoundManagerConfig {
  public:
    explicit SoundManagerConfig(
            SoundManager* pSoundManager);

    enum Defaults {
        API = (1 << 0),
        DEVICES = (1 << 1),
        OTHER = (1 << 2),
        ALL = (API | DEVICES | OTHER),
    };

    // Size1xms presents the first buffer size of 2^X
    // that results in a buffer time above 1 ms
    // It is 1.45 ms @ 44.1 kHz
    // The other values are representing the following 2^X sizes.
    enum class AudioBufferSizeIndex {
        Size1xms = 1,
        Size2xms = 2,
        Size5xms = 3,
        Size10xms = 4,
        Size20xms = 5,
        Size40xms = 6,
        Size80xms = 7,
    };

    // Represents the sample rate independent frame/period
    // index values in case of Jack
    enum class JackAudioBufferSizeIndex {
        SizeAuto = 5,
        Size2048fpp = 6,
        Size4096fpp = 7,
    };

    static constexpr auto kMaxAudioBufferSizeIndex =
            static_cast<unsigned int>(AudioBufferSizeIndex::Size80xms);
    static constexpr auto kDefaultAudioBufferSizeIndex =
            static_cast<unsigned int>(AudioBufferSizeIndex::Size20xms);

    static const QString kDefaultAPI;
    static const QString kEmptyComboBox;
    static const unsigned int kFallbackSampleRate;
    static const unsigned int kDefaultDeckCount;
    static const int kDefaultSyncBuffers;

    bool readFromDisk();
    bool writeToDisk() const;
    QString getAPI() const;
    void setAPI(const QString& api);
    bool checkAPI();
    unsigned int getSampleRate() const;
    void setSampleRate(unsigned int sampleRate);
    bool checkSampleRate(const SoundManager& soundManager);

    // Record the number of decks configured with this setup so they can
    // be created and configured.
    unsigned int getDeckCount() const;
    void setDeckCount(unsigned int deckCount);
    void setCorrectDeckCount(int configuredDeckCount);
    QSet<SoundDeviceId> getDevices() const;

    unsigned int getAudioBufferSizeIndex() const;
    unsigned int getFramesPerBuffer() const;
    void setAudioBufferSizeIndex(unsigned int latency);
    unsigned int getSyncBuffers() const;
    void setSyncBuffers(unsigned int sampleRate);
    bool getForceNetworkClock() const;
    void setForceNetworkClock(bool force);
    void addOutput(const SoundDeviceId& device, const AudioOutput& out);
    void addInput(const SoundDeviceId& device, const AudioInput& in);
    QMultiHash<SoundDeviceId, AudioOutput> getOutputs() const;
    QMultiHash<SoundDeviceId, AudioInput> getInputs() const;
    void clearOutputs();
    void clearInputs();
    bool hasMicInputs();
    bool hasExternalRecordBroadcast();
    void loadDefaults(SoundManager* soundManager, unsigned int flags);

  private:
    QFileInfo m_configFile;
    QString m_api;
    // none of our sample rates are actually decimals, this avoids
    // the weirdness using floating point can introduce
    unsigned int m_sampleRate;
    unsigned int m_deckCount;
    // m_latency is an index > 0, where 1 is a latency of 1ms and
    // higher indices represent subsequently higher latencies (storing
    // latency as milliseconds or frames per buffer is bad because those
    // values vary with sample rate) -- bkgood
    unsigned int m_audioBufferSizeIndex;
    unsigned int m_syncBuffers;
    bool m_forceNetworkClock;
    QMultiHash<SoundDeviceId, AudioOutput> m_outputs;
    QMultiHash<SoundDeviceId, AudioInput> m_inputs;
    int m_iNumMicInputs;
    bool m_bExternalRecordBroadcastConnected;
    SoundManager* m_pSoundManager;
};
