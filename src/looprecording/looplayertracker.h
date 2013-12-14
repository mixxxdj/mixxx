// looplayertracker.h
// Created by Carl Pillot on 8/23/13.
// Responsible for tracking the playback status of loop layers and mixing them into
// final loops.

#ifndef __LOOPTRACKER_H__
#define __LOOPTRACKER_H__

#include <QtCore>

#include "configobject.h"
#include "trackinfoobject.h"
#include "looprecording/layerinfo.h"

class ControlObjectThread;
class ControlLogpotmeter;

class LoopLayerTracker : public QObject {
    Q_OBJECT
  public:
    LoopLayerTracker(ConfigObject<ConfigValue>* pConfig);
    virtual ~LoopLayerTracker();

    void addLoopLayer(QString path, int length, int iSampleRate);
    void clear();
    void finalizeLoop(QString newPath, double bpm);
    QString getCurrentPath();
    void play();
    void stop(bool clearDeck);
    void setCurrentLength(int length);

  public slots:
    void slotFileFinished(TrackPointer);
    void slotLoadToLoopDeck(int);
    void slotLoop1Loaded(TrackPointer);

  signals:
    void exportLoop(QString);
    // Connected directly to PlayerManager
    void loadToDeck(TrackPointer, QString, bool);

  private slots:
    void slotChangeLoopPregain(double);

  private:
    int getCurrentLength();

    ConfigObject<ConfigValue>* m_pConfig;
    QList<LayerInfo*> m_layers;
    int m_iCurrentLayer;
    bool m_bIsUndoAvailable;
    bool m_bIsRedoAvailable;

    ControlObjectThread* m_pLoopDeck1Play;
    ControlObjectThread* m_pLoopDeck1Stop;
    ControlObjectThread* m_pLoopDeck1Eject;
    ControlObjectThread* m_pLoopDeck1Pregain;
    ControlObjectThread* m_pLoopDeck1LoopIn;
    ControlObjectThread* m_pLoopDeck1LoopOut;
    ControlObjectThread* m_pLoopDeck1Reloop;
    ControlObjectThread* m_pLoopDeck1LoopEnabled;
    ControlObjectThread* m_pLoopDeck2Play;
    ControlObjectThread* m_pLoopDeck2Stop;
    ControlObjectThread* m_pLoopDeck2Eject;
    ControlObjectThread* m_pLoopDeck2Pregain;
    ControlObjectThread* m_pTogglePlayback;
    ControlLogpotmeter* m_pLoopPregain;
};

#endif
