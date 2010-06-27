#include "samplermanager.h"

#include "trackinfoobject.h"
#include "sampler.h"
#include "analyserqueue.h"
#include "controlobject.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "engine/enginemaster.h"

SamplerManager::SamplerManager(ConfigObject<ConfigValue> *pConfig,
    EngineMaster* pEngine,
    Library* pLibrary)
    : m_pConfig(pConfig),
    m_pEngine(pEngine),
m_pLibrary(pLibrary) {
    m_pAnalyserQueue = AnalyserQueue::createDefaultAnalyserQueue(pConfig);

    connect(m_pLibrary, SIGNAL(loadTrackToSampler(TrackInfoObject*, int)),
        this, SLOT(slotLoadTrackToSampler(TrackInfoObject*, int)));

    // Disabled to remove loading into sampler on double click
    //connect(m_pLibrary, SIGNAL(loadTrack(TrackInfoObject*)),
    //  this, SLOT(slotLoadTrackIntoNextAvailableSampler(TrackInfoObject*)));
}

SamplerManager::~SamplerManager() {
    delete m_pAnalyserQueue;

    QMutableListIterator<Sampler*> it(m_samplers);
    while (it.hasNext()) {
        Sampler* pSampler = it.next();
        it.remove();
        delete pSampler;
    }
}

int SamplerManager::numSamplers() {
    return m_samplers.size();
}

Sampler* SamplerManager::addSampler() {
    Sampler* pSampler;
    int number = numSamplers() + 1;


    //TODO Fix this so it adjusts to the number of players
    pSampler = new Sampler(m_pConfig, m_pEngine, number,
        QString("[Channel%1]").arg(number+2));


    // Connect the player to the library so that when a track is unloaded, it's
    // data (eg. waveform summary) is saved back to the database.
    connect(pSampler, SIGNAL(unloadingTrack(TrackInfoObject*)),
        &(m_pLibrary->getTrackCollection()->getTrackDAO()),
        SLOT(saveTrack(TrackInfoObject*)));

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    connect(pSampler, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pAnalyserQueue, SLOT(queueAnalyseTrack(TrackInfoObject*)));

    m_samplers.append(pSampler);

    return pSampler;
}

Sampler* SamplerManager::getSampler(QString group) {
    QList<Sampler*>::iterator it = m_samplers.begin();
    while (it != m_samplers.end()) {
        Sampler* pSampler = *it;
        if (pSampler->getGroup() == group) {
            return pSampler;
        }
        it++;
    }
    return NULL;
}


Sampler* SamplerManager::getSampler(int sampler) {
    if (sampler < 1 || sampler > numSamplers()) {
        qWarning() << "Warning PlayerManager::getPlayer() called with invalid index: " << sampler;
        return NULL;
    }
    return m_samplers[sampler - 1];
}

void SamplerManager::slotLoadTrackToSampler(TrackInfoObject* pTrack, 
int sampler) {
    Sampler* pSampler = getSampler(sampler);

    if (pSampler == NULL) {
        qWarning() << "Invalid sampler argument " << sampler << " to slotLoadTrackToPlayer.";
        return;
    }

    pSampler->slotLoadTrack(pTrack);
}

void SamplerManager::slotLoadTrackIntoNextAvailableSampler(TrackInfoObject* pTrack)
{
    QList<Sampler*>::iterator it = m_samplers.begin();
    while (it != m_samplers.end()) {
        Sampler* pSampler = *it;
        ControlObject* playControl =
            ControlObject::getControl(ConfigKey(pSampler->getGroup(), "play"));
        if (playControl && playControl->get() != 1.) {
            pSampler->slotLoadTrack(pTrack, false);
            break;
        }
        it++;
    }
}

TrackInfoObject* SamplerManager::lookupTrack(QString location) {
    // Try to get TrackInfoObject* from library, identified by location.
    TrackDAO& trackDao = m_pLibrary->getTrackCollection()->getTrackDAO();
    TrackInfoObject* pTrack = trackDao.getTrack(trackDao.getTrackId(location));
    // If not, create a new TrackInfoObject*
    if (pTrack == NULL)
    {
        pTrack = new TrackInfoObject(location);
    }
    return pTrack;
}

QString SamplerManager::getTrackLocation(int sampler) {
    return m_samplers[sampler-1]->getLoadedTrackLocation();
}

void SamplerManager::slotLoadToSampler(QString location, int sampler) {
    Sampler* pSampler = getSampler(sampler);

    if (pSampler == NULL) {
        qWarning() << "Invalid player argument " << sampler << " to slotLoadToPlayer.";
        return;
    }

    TrackInfoObject *pTrack = lookupTrack(location);

    //Load the track into the Player.
    pSampler->slotLoadTrack(pTrack);
}


void SamplerManager::slotLoadToSampler(QString location, QString group) {
    Sampler* pSampler = getSampler(group);

    if (pSampler == NULL) {
        qWarning() << "Invalid group argument " << group << " to slotLoadToPlayer.";
        return;
    }

    TrackInfoObject *pTrack = lookupTrack(location);

    //Load the track into the Player.
    pSampler->slotLoadTrack(pTrack);
    qDebug("Track Loaded in Sampler.");
}
