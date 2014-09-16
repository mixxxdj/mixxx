#ifndef VINYLCONTROLPROCESSOR_H
#define VINYLCONTROLPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QWaitCondition>

#include "configobject.h"
#include "controlobjectslave.h"
#include "util/fifo.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "soundmanagerutil.h"

class VinylControl;
class ControlPushButton;

// VinylControlProcessor is a thread that is in charge of receiving samples from
// the engine callback and feeding those samples to the VinylControl
// classes. The most important thing is that the connection between the engine
// callback and VinylControlProcessor (the receiveBuffer method) is lock-free.
class VinylControlProcessor : public QObject, public AudioDestination {
    Q_OBJECT
  public:
    VinylControlProcessor(QObject* pParent, ConfigObject<ConfigValue>* pConfig);
    virtual ~VinylControlProcessor();

    // Called from main thread. Must only touch m_bReportSignalQuality.
    void setSignalQualityReporting(bool enable);

    // Called from the main thread. Must only touch m_bReload;
    void requestReloadConfig();

    void deckAdded(QString group);

    bool deckConfigured(int index) const;

    FIFO<VinylSignalQualityReport>* getSignalQualityFifo() {
        return &m_signalQualityFifo;
    }

  public slots:
    virtual void onInputConfigured(AudioInput input);
    virtual void onInputUnconfigured(AudioInput input);

    // Called by the engine callback. Must not touch any state in
    // VinylControlProcessor except for m_samplePipes. NOTE:

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, this
    // method is re-entrant since the VinylControlProcessor is registered for
    // multiple AudioDestinations, however it is not re-entrant for a given
    // AudioInput index.
    void receiveBuffer(AudioInput input, const CSAMPLE* pBuffer,
                       unsigned int iNumFrames);

  private slots:
    void slotGuiTick(double);
    void slotVinylEnabled(double);
    void toggleDeck(double value);

  private:
    // Returns the number of decks that are enabled. Does not hold the lock,
    // so should only be used where the lock is already being held.
    bool enabledDeckExists() const;

    ConfigObject<ConfigValue>* m_pConfig;
    ControlPushButton* m_pToggle;
    QList<ControlObjectSlave*> m_VCEnableds;
    ControlObjectSlave* m_pGuiTick;
    CSAMPLE* m_pWorkBuffer;
    QMutex m_processorsLock;
    QVector<VinylControl*> m_processors;
    bool m_processingActive;
    FIFO<VinylSignalQualityReport> m_signalQualityFifo;
    volatile bool m_bReportSignalQuality;
};


#endif /* VINYLCONTROLPROCESSOR_H */
