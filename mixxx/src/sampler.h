#ifndef SAMPLER_H
#define SAMPLER_H

#include <QtCore>

#include "configobject.h"

class EngineMaster;
class TrackInfoObject;
class ControlObjectThreadMain;
class ControlObject;

class Sampler : public QObject
{
    Q_OBJECT
    public:
        Sampler(ConfigObject<ConfigValue> *pConfig, EngineMaster* pMixingEngine,
            int samplerNumber, const char* pGroup);
        ~Sampler();
        QString getGroup();
        QString getLoadedTrackLocation();
    public slots:
        void slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos=false);
        void slotFinishLoading(TrackInfoObject* pTrackInfoObject);
        void slotLoadFailed(TrackInfoObject* pTrackInfoObject, QString reason);
    signals:
        void loadTrack(TrackInfoObject* pTrack);
        void newTrackLoaded(TrackInfoObject* pLoadedTrack);
        void unloadingTrack(TrackInfoObject* pAboutToBeUnloaded);
    private:
        ConfigObject<ConfigValue>* m_pConfig;
        int m_iSamplerNumber;
        QString m_strChannel;

        TrackInfoObject* m_pLoadedTrack;

        ControlObjectThreadMain* m_pCuePoint;
        ControlObjectThreadMain* m_pLoopInPoint;
        ControlObjectThreadMain* m_pLoopOutPoint;
        ControlObjectThreadMain* m_pPlayPosition;
        ControlObject* m_pDuration;
        ControlObjectThreadMain* m_pBPM;
    };

#endif
