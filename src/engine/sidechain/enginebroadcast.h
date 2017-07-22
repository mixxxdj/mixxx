#ifndef ENGINE_SIDECHAIN_ENGINEBROADCAST_H
#define ENGINE_SIDECHAIN_ENGINEBROADCAST_H

#include <QMessageBox>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>
#include <QSemaphore>
#include <QTextCodec>
#include <QThread>
#include <QVector>
#include <QMap>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/sidechain/networkstreamworker.h"
#include "preferences/usersettings.h"
#include "util/fifo.h"
#include "preferences/broadcastsettings.h"
#include "engine/sidechain/shoutoutput.h"

class ControlPushButton;

class EngineBroadcast
        : public QThread, public NetworkStreamWorker {
    Q_OBJECT
  public:
    EngineBroadcast(UserSettingsPointer pConfig,
                    BroadcastSettingsPointer pBroadcastSettings);
    virtual ~EngineBroadcast();

    bool addConnection(BroadcastProfilePtr profile);
    bool removeConnection(BroadcastProfilePtr profile);

    // This is called by the Engine implementation for each sample. Encode and
    // send the stream, as well as check for metadata changes.
    void process(const CSAMPLE* pBuffer, const int iBufferSize);

    void shutdown() {
    }

    virtual void outputAvailable();
    virtual void setOutputFifo(FIFO<CSAMPLE>* pOutputFifo);

    virtual bool threadWaiting();

    virtual void run();

  private slots:
    void slotEnableCO(double v);
    void slotProfileAdded(BroadcastProfilePtr profile);
    void slotProfileRemoved(BroadcastProfilePtr profile);
    void slotProfileRenamed(QString oldName, BroadcastProfilePtr profile);
    void slotProfilesChanged();

  private:
#ifndef __WINDOWS__
    void ignoreSigpipe();
#endif

    QMap<QString,ShoutOutputPtr> m_connections;

    BroadcastSettingsPointer m_settings;
    UserSettingsPointer m_pConfig;
    ControlPushButton* m_pBroadcastEnabled;

    QAtomicInt m_threadWaiting;
    QSemaphore m_readSema;
    FIFO<CSAMPLE>* m_pOutputFifo;

    QMutex m_enabledMutex;
};

#endif // ENGINE_SIDECHAIN_ENGINEBROADCAST_H
