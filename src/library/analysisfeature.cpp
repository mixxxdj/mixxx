// analysisfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <QtDebug>

#include "library/library.h"
#include "library/analysisfeature.h"

#include "library/library.h"
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

namespace {

// Utilize all available cores for batch analysis of tracks
const int kNumberOfAnalyzerThreads = math_max(1, QThread::idealThreadCount());

inline
AnalyzerMode getAnalyzerMode(
        const UserSettingsPointer& pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true)) {
        return AnalyzerMode::WithWaveform;
    } else {
        return AnalyzerMode::WithoutWaveform;
    }
}

} // anonymous namespace

AnalysisFeature::AnalysisFeature(
        Library* parent,
        UserSettingsPointer pConfig) :
        LibraryFeature(parent),
        m_library(parent),
        m_pConfig(pConfig),
        m_iOldBpmEnabled(0),
        m_analysisTitleName(tr("Analyze")),
        m_pAnalysisView(nullptr) {
    setTitleDefault();
}

AnalysisFeature::~AnalysisFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
}

void AnalysisFeature::setTitleDefault() {
    m_Title = m_analysisTitleName;
    emit(featureIsLoading(this, false));
}

void AnalysisFeature::setTitleProgress(int currentTrack, int totalTracks) {
    m_Title = QString("%1 (%2 / %3)")
            .arg(m_analysisTitleName)
            .arg(QString::number(currentTrack))
            .arg(QString::number(totalTracks));
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
                                      &m_library->trackCollection());
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
            m_pAnalysisView, SLOT(slotAnalysisActive(bool)));

    m_pAnalysisView->installEventFilter(keyboard);

    // Let the DlgAnalysis know whether or not analysis is active.
    emit(analysisActive(static_cast<bool>(m_pAnalyzerQueue)));

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
    if (!m_pAnalyzerQueue) {
        // Save the old BPM detection prefs setting (on or off)
        m_iOldBpmEnabled = m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();
        // Force BPM detection to be on.
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(1));
        // Note: this sucks... we should refactor the prefs/analyzer to fix this hacky bit ^^^^.

        m_pAnalyzerQueue = std::make_unique<AnalyzerQueue>(
                m_library,
                kNumberOfAnalyzerThreads,
                m_pConfig,
                getAnalyzerMode(m_pConfig));

        connect(m_pAnalyzerQueue.get(), SIGNAL(progress(int, int, int)),
                m_pAnalysisView, SLOT(slotAnalyzerQueueProgress(int, int, int)));
        connect(m_pAnalyzerQueue.get(), SIGNAL(progress(int, int, int)),
                this, SLOT(slotAnalyzerQueueProgress(int, int, int)));
        connect(m_pAnalyzerQueue.get(), SIGNAL(empty(int)),
                this, SLOT(slotAnalyzerQueueEmpty(int)));
        connect(m_pAnalyzerQueue.get(), SIGNAL(done()),
                this, SLOT(slotAnalyzerQueueDone()));

        emit(analysisActive(true));
    }

    for (const auto& trackId: trackIds) {
        if (trackId.isValid()) {
            m_pAnalyzerQueue->enqueueTrackId(trackId);
        }
    }
    m_pAnalyzerQueue->resume();
}

void AnalysisFeature::slotAnalyzerQueueProgress(
        int /*analyzerProgress*/,
        int finishedCount,
        int totalCount) {
    if (totalCount > 0) {
        setTitleProgress(finishedCount, totalCount);
    } else {
        setTitleDefault();
    }
}

void AnalysisFeature::slotAnalyzerQueueEmpty(int finishedCount) {
    // Only abandon the queue after all enqueued tracks have been
    // dequeued to avoid a race condition when the worker threads
    // are started and running before all selected tracks have
    // been enqueued.
    if (finishedCount > 0) {
        slotAnalyzerQueueDone();
    }
}

void AnalysisFeature::slotAnalyzerQueueDone() {
    if (m_pAnalyzerQueue) {
        m_pAnalyzerQueue->cancel();
        m_pAnalyzerQueue->deleteLater();
        // Release ownership
        m_pAnalyzerQueue.release();
        DEBUG_ASSERT(!m_pAnalyzerQueue);
        // Restore old BPM detection setting for preferences...
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(m_iOldBpmEnabled));
    }
    setTitleDefault();
    emit(analysisActive(false));
}

void AnalysisFeature::stopAnalysis() {
    //qDebug() << this << "stopAnalysis()";
    if (m_pAnalyzerQueue) {
        m_pAnalyzerQueue->cancel();
    }
}

bool AnalysisFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    Q_UNUSED(pSource);
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    // Adds track, does not insert duplicates, handles unremoving logic.
    QList<TrackId> trackIds = m_library->trackCollection().getTrackDAO().addMultipleTracks(files, true);
    analyzeTracks(trackIds);
    return trackIds.size() > 0;
}

bool AnalysisFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url);
}
