#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QDesktopServices>
#include <QDateTime>
#include <QObject>


#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "controlobjectthreadmain.h"
#include "recording/defs_recording.h"

/*
 * The RecordingManager is a central class and manages
 * the recoring feature of Mixxx.
 *
 * There is exactly one instance of this class
 *
 * All methods in this class are thread-safe
 *
 * Note: The RecordingManager lives in the GUI thread
*/

class ControlObjectThreadMain;

class RecordingManager : public QObject
{
    Q_OBJECT
    public:
        RecordingManager(ConfigObject<ConfigValue>* pConfig);
        virtual ~RecordingManager();

        void startRecording();
        void stopRecording();
        bool isRecordingActive();
        QString& getRecordingDir();
    signals:
        void bytesRecorded(int);
        void isRecording(bool);

    public slots:
        void slotIsRecording(bool);
        void slotBytesRecorded(int);

    private:
       ControlObjectThread* m_recReady;
       ControlObject* m_recReadyCO;

       ConfigObject<ConfigValue>* m_pConfig;
       QString m_recordingDir;
       bool m_isRecording;

};

#endif // RECORDINGMANAGER_H
