#ifndef LOOPRECORDINGMANAGER_H
#define LOOPRECORDINGMANAGER_H

#include <QDesktopServices>
#include <QDateTime>
#include <QObject>
#include <sndfile.h>

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "controlobjectthreadmain.h"

class EngineMaster;
class ControlPushButton;
class ControlObject;
class ControlObjectThread;
class LoopLayerTracker;

class LoopRecordingManager : public QObject {
    Q_OBJECT
  public:
    LoopRecordingManager(ConfigObject<ConfigValue>* pConfig, EngineMaster* pEngine);
    virtual ~LoopRecordingManager();

    LoopLayerTracker* getLoopLayerTracker() {
        return m_pLoopLayerTracker;
    }
    // Returns loop source (used in legacy skin parser)
    QString getLoopSource() {
        return m_loopSource;
    }
    QString getLoopDestination() {
        return m_loopDestination;
    }

  public slots:
    void slotClearRecorder();
    void slotCurrentPlayingDeckChanged(int);
    void slotExportLoopToPlayer(QString);
    void slotIsRecording(bool);

  signals:
    void clearWriter();
    void destinationChanged(QString);
    void exportToPlayer(QString, QString);
    void fileOpen(SNDFILE*);
    void sourceChanged(QString);
    void startWriter(int);
    void stopWriter(bool);

  private slots:
    void slotChangeExportDestination(double v);
    void slotChangeLoopLength(double v);
    void slotChangeLoopSource(double v);
    void slotNumDecksChanged(double v);
    void slotNumSamplersChanged(double v);
    void slotToggleClear(double v);
    void slotToggleExport(double v);
    void slotToggleLoopRecording(double v);
    void slotTogglePlayback(double v);

  private:
    void clearLoopDeck();
    QString formatDateTimeForFilename(QDateTime dateTime) const;
    double getCurrentBPM();
    int getLoopLength();
    SNDFILE* openSndFile(QString);
    void playLoopDeck();
    void setRecordingDir();
    void startRecording();
    void stopRecording();
    void stopLoopDeck();

    ConfigObject<ConfigValue>* m_pConfig;

    ControlObject* m_pCOLoopBeats;
    ControlObject* m_pCOLoopPlayReady;

    ControlObjectThread* m_pLoopPlayReady;
    ControlObjectThread* m_pNumDecks;
    ControlObjectThread* m_pNumSamplers;
    ControlObjectThread* m_pRecReady;
    ControlObjectThread* m_pSampleRate;

    ControlPushButton* m_pChangeExportDestination;
    ControlPushButton* m_pChangeLoopLength;
    ControlPushButton* m_pChangeLoopSource;
    ControlPushButton* m_pClearRecorder;
    ControlPushButton* m_pExportLoop;
    ControlPushButton* m_pToggleLoopRecording;
    ControlPushButton* m_pTogglePlayback;

    QList<ControlObjectThread*> m_deckRateControls;

    // Tracks all loop layers recorded.
    LoopLayerTracker* m_pLoopLayerTracker;

    QString m_dateTime;
    QString m_encodingType;
    QString m_loopDestination;
    QString m_loopSource;
    QString m_recordingDir;
    QString m_recordingTempDir;

    //the base file
    //QString m_recording_base_file;
    //Absolute file
    QString m_recordingLocation;

    bool m_bRecording;

    int m_iCurrentPlayingDeck;
    double m_dLoopBPM;
    int m_iLoopLength;
    unsigned int m_iLoopNumber;
    int m_iNumDecks;
    int m_iNumSamplers;
};

#endif // LOOPRECORDINGMANAGER_H
