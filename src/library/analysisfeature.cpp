// analysisfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <QtDebug>

#include "library/analysisfeature.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/dlganalysis.h"
#include "widget/wlibrary.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "analyzer/analyzerqueue.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "util/debug.h"

const QString AnalysisFeature::m_sAnalysisViewName = QString("Analysis");

AnalysisFeature::AnalysisFeature(QObject* parent,
                               UserSettingsPointer pConfig,
                               TrackCollection* pTrackCollection) :
        LibraryFeature(parent),
        m_pConfig(pConfig),
        m_pTrackCollection(pTrackCollection),
        m_pAnalyzerQueue(NULL),
        m_iOldBpmEnabled(0),
        m_analysisTitleName(tr("Analyze")),
        m_pAnalysisView(NULL) {
    setTitleDefault();
}

AnalysisFeature::~AnalysisFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
    cleanupAnalyzer();
}


void AnalysisFeature::setTitleDefault() {
    m_Title = m_analysisTitleName;
    emit(featureIsLoading(this, false));
}

void AnalysisFeature::setTitleProgress(int trackNum, int totalNum) {
    m_Title = QString("%1 (%2 / %3)")
            .arg(m_analysisTitleName)
            .arg(QString::number(trackNum))
            .arg(QString::number(totalNum));
    emit(featureIsLoading(this, false));
}

QVariant AnalysisFeature::title() {
    return m_Title;
}

QIcon AnalysisFeature::getIcon() {
    return QIcon(":/images/library/ic_library_prepare.png");
}

void AnalysisFeature::bindWidget(WLibrary* libraryWidget,
                                 KeyboardEventFilter* keyboard) {
    m_pAnalysisView = new DlgAnalysis(libraryWidget,
                                      m_pConfig,
                                      m_pTrackCollection);
    connect(m_pAnalysisView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pAnalysisView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
    connect(m_pAnalysisView, SIGNAL(analyzeTracks(QList<TrackId>)),
            this, SLOT(analyzeTracks(QList<TrackId>)));
    connect(m_pAnalysisView, SIGNAL(stopAnalysis()),
            this, SLOT(stopAnalysis()));

    connect(m_pAnalysisView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));

    connect(this, SIGNAL(analysisActive(bool)),
            m_pAnalysisView, SLOT(analysisActive(bool)));
    connect(this, SIGNAL(trackAnalysisStarted(int)),
            m_pAnalysisView, SLOT(trackAnalysisStarted(int)));

    m_pAnalysisView->installEventFilter(keyboard);

    // Let the DlgAnalysis know whether or not analysis is active.
    bool bAnalysisActive = m_pAnalyzerQueue != NULL;
    emit(analysisActive(bAnalysisActive));

    libraryWidget->registerView(m_sAnalysisViewName, m_pAnalysisView);
}

TreeItemModel* AnalysisFeature::getChildModel() {
    return &m_childModel;
}

void AnalysisFeature::refreshLibraryModels() {
    if (m_pAnalysisView) {
        m_pAnalysisView->onShow();
    }
}

void AnalysisFeature::activate() {
    //qDebug() << "AnalysisFeature::activate()";
    emit(switchToView(m_sAnalysisViewName));
    if (m_pAnalysisView) {
        emit(restoreSearch(m_pAnalysisView->currentSearch()));
    }
    emit(enableCoverArtDisplay(true));
}

void AnalysisFeature::analyzeTracks(QList<TrackId> trackIds) {
    if (m_pAnalyzerQueue == NULL) {
        // Save the old BPM detection prefs setting (on or off)
        m_iOldBpmEnabled = m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();
        // Force BPM detection to be on.
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(1));
        // Note: this sucks... we should refactor the prefs/analyzer to fix this hacky bit ^^^^.

        m_pAnalyzerQueue = AnalyzerQueue::createAnalysisFeatureAnalyzerQueue(m_pConfig, m_pTrackCollection);

        connect(m_pAnalyzerQueue, SIGNAL(trackProgress(int)),
                m_pAnalysisView, SLOT(trackAnalysisProgress(int)));
        connect(m_pAnalyzerQueue, SIGNAL(trackFinished(int)),
                this, SLOT(slotProgressUpdate(int)));
        connect(m_pAnalyzerQueue, SIGNAL(trackFinished(int)),
                m_pAnalysisView, SLOT(trackAnalysisFinished(int)));

        connect(m_pAnalyzerQueue, SIGNAL(queueEmpty()),
                this, SLOT(cleanupAnalyzer()));
        emit(analysisActive(true));
    }

    for (const auto& trackId: trackIds) {
        TrackPointer pTrack = m_pTrackCollection->getTrackDAO().getTrack(trackId);
        if (pTrack) {
            //qDebug() << this << "Queueing track for analysis" << pTrack->getLocation();
            m_pAnalyzerQueue->queueAnalyseTrack(pTrack);
        }
    }
    if (trackIds.size() > 0) {
        setTitleProgress(0, trackIds.size());
    }
    emit(trackAnalysisStarted(trackIds.size()));
}

void AnalysisFeature::slotProgressUpdate(int num_left) {
    int num_tracks = m_pAnalysisView->getNumTracks();
    if (num_left > 0) {
        int currentTrack = num_tracks - num_left + 1;
        setTitleProgress(currentTrack, num_tracks);
    }
}

void AnalysisFeature::stopAnalysis() {
    //qDebug() << this << "stopAnalysis()";
    if (m_pAnalyzerQueue != NULL) {
        m_pAnalyzerQueue->stop();
    }
}

void AnalysisFeature::cleanupAnalyzer() {
    setTitleDefault();
    emit(analysisActive(false));
    if (m_pAnalyzerQueue != NULL) {
        m_pAnalyzerQueue->stop();
        m_pAnalyzerQueue->deleteLater();
        m_pAnalyzerQueue = NULL;
        // Restore old BPM detection setting for preferences...
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(m_iOldBpmEnabled));
    }
}

bool AnalysisFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    Q_UNUSED(pSource);
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    // Adds track, does not insert duplicates, handles unremoving logic.
    QList<TrackId> trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    analyzeTracks(trackIds);
    return trackIds.size() > 0;
}

bool AnalysisFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url);
}
