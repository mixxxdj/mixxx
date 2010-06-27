#ifndef SAMPLERMANAGER_H
#define SAMPLERMANAGER_H

#include <QList>

#include "configobject.h"

class Sampler;
class Library;
class EngineMaster;
class AnalyserQueue;
class TrackInfoObject;

class SamplerManager : public QObject {
    Q_OBJECT
    public:
        SamplerManager(ConfigObject<ConfigValue> *pConfig,
            EngineMaster* pEngine,
            Library* pLibrary);
        virtual ~SamplerManager();
        
        Sampler* addSampler();
        
        int numSamplers();
        
        Sampler* getSampler(int Sampler);
        Sampler* getSampler(QString group);
        
        QString getTrackLocation(int sampler);

    public slots:
        void slotLoadTrackToSampler(TrackInfoObject* pTrack, int sampler);
        void slotLoadTrackIntoNextAvailableSampler(TrackInfoObject* pTrack);
        void slotLoadToSampler(QString location, int sampler);
        void slotLoadToSampler(QString location, QString group);

    private:
        TrackInfoObject* lookupTrack(QString location);
        ConfigObject<ConfigValue>* m_pConfig;
        EngineMaster* m_pEngine;
        Library* m_pLibrary;
        AnalyserQueue* m_pAnalyserQueue;
        QList<Sampler*> m_samplers;
    };

#endif /* SAMPLERMANAGER_H */
