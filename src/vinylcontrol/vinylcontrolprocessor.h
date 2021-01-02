#pragma once

#include <QObject>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QWaitCondition>

#include "preferences/usersettings.h"
#include "util/fifo.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "soundio/soundmanagerutil.h"

class VinylControl;
class ControlPushButton;

// VinylControlProcessor is a thread that is in charge of receiving samples from
// the engine callback and feeding those samples to the VinylControl
// classes. The most important thing is that the connection between the engine
// callback and VinylControlProcessor (the receiveBuffer method) is lock-free.
class VinylControlProcessor : public QThread, public AudioDestination {
    Q_OBJECT
  public:
    VinylControlProcessor(QObject* pParent, UserSettingsPointer pConfig);
    virtual ~VinylControlProcessor();

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
    virtual void onInputConfigured(const AudioInput& input);
    virtual void onInputUnconfigured(const AudioInput& input);

    // Called by the engine callback. Must not touch any state in
    // VinylControlProcessor except for m_samplePipes. NOTE:

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, this
    // method is re-entrant since the VinylControlProcessor is registered for
    // multiple AudioDestinations, however it is not re-entrant for a given
    // AudioInput index.
    void receiveBuffer(const AudioInput& input, const CSAMPLE* pBuffer, unsigned int iNumFrames);

  protected:
    void run();

  private slots:
    void toggleDeck(double value);

  private:
    void reloadConfig();

    UserSettingsPointer m_pConfig;
    ControlPushButton* m_pToggle;
    // A pre-allocated array of FIFOs for writing samples from the engine
    // callback to the processor thread. There is a maximum of
    // kMaximumVinylControlInputs pipes.
    FIFO<CSAMPLE>* m_samplePipes[kMaximumVinylControlInputs];
    CSAMPLE* m_pWorkBuffer;
    QWaitCondition m_samplesAvailableSignal;
    QMutex m_waitForSampleMutex;
    QMutex m_processorsLock;
    QVector<VinylControl*> m_processors;
    FIFO<VinylSignalQualityReport> m_signalQualityFifo;
    volatile bool m_bReportSignalQuality;
    volatile bool m_bQuit;
    volatile bool m_bReloadConfig;
};
