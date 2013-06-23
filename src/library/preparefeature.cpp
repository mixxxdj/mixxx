// preparefeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <QtDebug>

#include "library/preparefeature.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "dlgprepare.h"
#include "widget/wlibrary.h"
#include "mixxxkeyboard.h"
#include "analyserqueue.h"

const QString PrepareFeature::m_sPrepareViewName = QString("Prepare");

PrepareFeature::PrepareFeature(QObject* parent,
                               ConfigObject<ConfigValue>* pConfig,
                               TrackCollection* pTrackCollection) :
        LibraryFeature(parent),
        m_pConfig(pConfig),
        m_pTrackCollection(pTrackCollection),
        m_pAnalyserQueue(NULL) {
}

PrepareFeature::~PrepareFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
    cleanupAnalyser();
}

QVariant PrepareFeature::title() {
    return tr("Analyze");
}

QIcon PrepareFeature::getIcon() {
    return QIcon(":/images/library/ic_library_prepare.png");
}

void PrepareFeature::bindWidget(WLibrary* libraryWidget,
                                MixxxKeyboard* keyboard) {
    m_pPrepareView = new DlgPrepare(libraryWidget,
                                    m_pConfig,
                                    m_pTrackCollection);
    connect(m_pPrepareView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pPrepareView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
    connect(m_pPrepareView, SIGNAL(analyzeTracks(QList<int>)),
            this, SLOT(analyzeTracks(QList<int>)));
    connect(m_pPrepareView, SIGNAL(stopAnalysis()),
            this, SLOT(stopAnalysis()));

    connect(this, SIGNAL(analysisActive(bool)),
            m_pPrepareView, SLOT(analysisActive(bool)));

    m_pPrepareView->installEventFilter(keyboard);

    // Let the DlgPrepare know whether or not analysis is active.
    bool bAnalysisActive = m_pAnalyserQueue != NULL;
    emit(analysisActive(bAnalysisActive));

    libraryWidget->registerView(m_sPrepareViewName, m_pPrepareView);
}

TreeItemModel* PrepareFeature::getChildModel() {
    return &m_childModel;
}

void PrepareFeature::refreshLibraryModels() {
    if (m_pPrepareView) {
        m_pPrepareView->onShow();
    }
}

void PrepareFeature::activate() {
    //qDebug() << "PrepareFeature::activate()";
    emit(switchToView(m_sPrepareViewName));
    if (m_pPrepareView) {
        emit(restoreSearch(m_pPrepareView->currentSearch()));
    }
}

void PrepareFeature::analyzeTracks(QList<int> trackIds) {
    if (m_pAnalyserQueue == NULL) {
        // Save the old BPM detection prefs setting (on or off)
        m_iOldBpmEnabled = m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();
        // Force BPM detection to be on.
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(1));
        // Note: this sucks... we should refactor the prefs/analyser to fix this hacky bit ^^^^.

        m_pAnalyserQueue = AnalyserQueue::createPrepareViewAnalyserQueue(m_pConfig, m_pTrackCollection);

        connect(m_pAnalyserQueue, SIGNAL(trackProgress(int)),
                m_pPrepareView, SLOT(trackAnalysisProgress(int)));
        connect(m_pAnalyserQueue, SIGNAL(trackFinished(int)),
                m_pPrepareView, SLOT(trackAnalysisFinished(int)));

        connect(m_pAnalyserQueue, SIGNAL(queueEmpty()),
                this, SLOT(cleanupAnalyser()));
        emit(analysisActive(true));
    }

    foreach(int trackId, trackIds) {
        TrackPointer pTrack = m_pTrackCollection->getTrackDAO().getTrack(trackId);
        if (pTrack) {
            //qDebug() << this << "Queueing track for analysis" << pTrack->getLocation();
            m_pAnalyserQueue->queueAnalyseTrack(pTrack);
        }
    }
}

void PrepareFeature::stopAnalysis() {
    //qDebug() << this << "stopAnalysis()";
    if (m_pAnalyserQueue != NULL) {
        m_pAnalyserQueue->stop();
    }
}

void PrepareFeature::cleanupAnalyser() {
    emit(analysisActive(false));
    if (m_pAnalyserQueue != NULL) {
        m_pAnalyserQueue->stop();
        m_pAnalyserQueue->deleteLater();
        m_pAnalyserQueue = NULL;
        // Restore old BPM detection setting for preferences...
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(m_iOldBpmEnabled));
    }
}
