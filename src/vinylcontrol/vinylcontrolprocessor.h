#pragma once

#include <QMutex>
#include <QRecursiveMutex>
#include <QThread>
#include <QVector>
#include <QWaitCondition>

#include "preferences/usersettings.h"
#include "soundio/soundmanagerutil.h"
#include "util/fifo.h"
#include "vinylcontrol/vinylsignalquality.h"

class VinylControl;
class ControlPushButton;
class QObject;

// VinylControlProcessor is a thread that is in charge of receiving samples from
// the engine callback and feeding those samples to the VinylControl
// classes. The most important thing is that the connection between the engine
// callback and VinylControlProcessor (the receiveBuffer method) is lock-free.
class VinylControlProcessor : public QThread, public AudioDestination {
    Q_OBJECT
  public:
    VinylControlProcessor(QObject* pParent, UserSettingsPointer pConfig);
    ~VinylControlProcessor() override;

    // Called from main thread. Must only touch m_bReportSignalQuality.
    void setSignalQualityReporting(bool enable);

    // Called from the main thread. Must only touch m_bQuit;
    void shutdown();

    // Called from the main thread. Must only touch m_bReload;
    void requestReloadConfig();

    bool deckConfigured(int index) const;

    FIFO<VinylSignalQualityReport>* getSignalQualityFifo() {
        return &m_signalQualityFifo;
    }

  public slots:
    void onInputConfigured(const AudioInput& input) override;
    void onInputUnconfigured(const AudioInput& input) override;

    // Called by the engine callback. Must not touch any state in
    // VinylControlProcessor except for m_samplePipes. NOTE:

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, this
    // method is re-entrant since the VinylControlProcessor is registered for
    // multiple AudioDestinations, however it is not re-entrant for a given
    // AudioInput index.
    void receiveBuffer(const AudioInput& input,
            const CSAMPLE* pBuffer,
            unsigned int iNumFrames) override;

  signals:
    void qualityReportWritten();

  protected:
    void run() override;

  private slots:
    void toggleDeck(double value);

  private:
    void reloadConfig();

    UserSettingsPointer m_pConfig;
    std::unique_ptr<ControlPushButton> m_pToggle;

    // A pre-allocated array of FIFOs for writing samples from the engine
    // callback to the processor thread. There is a maximum of
    // kMaximumVinylControlInputs pipes.
    using VinylControlRingbufferPtr = std::unique_ptr<FIFO<CSAMPLE>>;
    std::array<VinylControlRingbufferPtr, kMaximumVinylControlInputs> m_samplePipes;

    using VinylControlProcessorPtr = std::unique_ptr<VinylControl>;
    std::array<VinylControlProcessorPtr, kMaximumVinylControlInputs> m_processors;

    CSAMPLE* m_pWorkBuffer;
    QWaitCondition m_samplesAvailableSignal;
    QMutex m_waitForSampleMutex;
    QRecursiveMutex m_processorsLock;
    FIFO<VinylSignalQualityReport> m_signalQualityFifo;
    volatile bool m_bReportSignalQuality;
    volatile bool m_bQuit;
    volatile bool m_bReloadConfig;
};
