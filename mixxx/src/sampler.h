#ifndef SAMPLER_H
#define SAMPLER_H

#include <QtCore>

#include "configobject.h"
#include "trackinfoobject.h"

class EngineMaster;
class ControlObjectThreadMain;
class ControlObject;

class Sampler : public QObject
{
    Q_OBJECT
    public:
        Sampler(ConfigObject<ConfigValue> *pConfig, 
                EngineMaster* pMixingEngine,
                int samplerNumber, const char* pGroup);
        ~Sampler();
        QString getGroup();
        QString getLoadedTrackLocation();
    public slots:
        void slotLoadTrack(TrackPointer track, bool bStartFromEndPos=false);
        void slotFinishLoading(TrackPointer pTrackInfoObject);
        void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
    signals:
        void loadTrack(TrackPointer pTrack);
        void newTrackLoaded(TrackPointer pLoadedTrack);
        void unloadingTrack(TrackPointer pAboutToBeUnloaded);
    private:
        ConfigObject<ConfigValue>* m_pConfig;
        int m_iSamplerNumber;
        QString m_strChannel;

        TrackPointer m_pLoadedTrack;

        ControlObjectThreadMain* m_pCuePoint;
        ControlObjectThreadMain* m_pLoopInPoint;
        ControlObjectThreadMain* m_pLoopOutPoint;
        ControlObjectThreadMain* m_pPlayPosition;
        ControlObject* m_pDuration;
        ControlObjectThreadMain* m_pBPM;
    };

#endif
