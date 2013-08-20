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
#include "looprecording/defs_looprecording.h"
#include "trackinfoobject.h"

class EngineMaster;
class ControlPushButton;
class ControlObject;
class ControlObjectThread;
class TrackInfoObject;

class LoopRecordingManager : public QObject {
    Q_OBJECT
  public:
    LoopRecordingManager(ConfigObject<ConfigValue>* pConfig, EngineMaster* pEngine);
    virtual ~LoopRecordingManager();
//    bool isLoopRecordingActive();

  public slots:
    void slotClearRecorder();
    void slotIsRecording(bool);
    void slotLoadToLoopDeck();
//    void slotCountSamplesRecorded(int);

  signals:
    void clearWriter();
    void exportToPlayer(QString, QString);
    void fileOpen(SNDFILE*);
//    void isLoopRecording(bool);
    void loadToLoopDeck(TrackPointer, QString, bool);
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
    void exportLoopToPlayer(QString group);
    QString formatDateTimeForFilename(QDateTime dateTime) const;
    unsigned int getLoopLength();
    SNDFILE* openSndFile(QString);
    void playLoopDeck();
    bool saveLoop(QString newFileLocation);
    void setRecordingDir();
    void startRecording();
    void stopRecording();
    void stopLoopDeck();

    ConfigObject<ConfigValue>* m_pConfig;
    
    ControlObject* m_pCOExportDestination;
    ControlObject* m_pCOLoopLength;
    ControlObject* m_pCOLoopPlayReady;

    ControlObjectThread* m_pLoopPlayReady;
    ControlObjectThread* m_pLoopSource;
    ControlObjectThread* m_pMasterBPM;
    ControlObjectThread* m_pNumDecks;
    ControlObjectThread* m_pNumSamplers;
    ControlObjectThread* m_pRecReady;
    ControlObjectThread* m_pSampleRate;

    ControlObjectThread* m_pLoopDeck1Play;
    ControlObjectThread* m_pLoopDeck1Stop;
    ControlObjectThread* m_pLoopDeck1Eject;

    ControlPushButton* m_pChangeExportDestination;
    ControlPushButton* m_pChangeLoopLength;
    ControlPushButton* m_pChangeLoopSource;
    ControlPushButton* m_pClearRecorder;
    ControlPushButton* m_pExportLoop;
    ControlPushButton* m_pToggleLoopRecording;
    ControlPushButton* m_pTogglePlayback;

    // Stores paths of all files recorded.
    QList<QString> m_filesRecorded;

    QString date_time_str;
    QString encodingType;
    QString m_recordingDir;

    //the base file
    QString m_recording_base_file;
    //filename without path
    QString m_recordingFile;
    //Absolute file
    QString m_recordingLocation;
    
    bool m_isRecording;

    unsigned int m_iLoopLength;
    unsigned int m_iLoopNumber;
    unsigned int m_iNumDecks;
    unsigned int m_iNumSamplers;
};

#endif // LOOPRECORDINGMANAGER_H
