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
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "util/debug.h"

const QString AnalysisFeature::m_sAnalysisViewName = QString("Analysis");

namespace {

// Utilize all available cores for batch analysis of tracks
const int kNumberOfAnalyzerThreads = math_max(1, QThread::idealThreadCount());

inline
AnalyzerModeFlags getAnalyzerModeFlags(
        const UserSettingsPointer& pConfig) {
    // Always enable at least BPM detection for batch analysis, even if disabled
    // in the config for ad-hoc analysis of tracks.
    // NOTE(uklotzde, 2018-12-26): The previous comment just states the status-quo
    // of the existing code. We should rethink the configuration of analyzers when
    // refactoring/redesigning the analyzer framework.
    int modeFlags = AnalyzerModeFlags::WithBeats;
    if (pConfig->getValue<bool>(ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true)) {
        modeFlags |= AnalyzerModeFlags::WithWaveform;
    }
    return static_cast<AnalyzerModeFlags>(modeFlags);
}

} // anonymous namespace

AnalysisFeature::AnalysisFeature(
        Library* parent,
        UserSettingsPointer pConfig)
        : LibraryFeature(parent),
        m_library(parent),
        m_pConfig(pConfig),
        m_pTrackAnalysisScheduler(TrackAnalysisScheduler::NullPointer()),
        m_analysisTitleName(tr("Analyze")),
        m_pAnalysisView(nullptr),
        m_icon(":/images/library/ic_library_prepare.svg") {
    setTitleDefault();
}

void AnalysisFeature::stop() {
    if (m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler->stop();
    }
}

void AnalysisFeature::setTitleDefault() {
    m_Title = m_analysisTitleName;
    emit(featureIsLoading(this, false));
}

void AnalysisFeature::setTitleProgress(int currentTrackNumber, int totalTracksCount) {
    m_Title = QString("%1 (%2 / %3)")
            .arg(m_analysisTitleName)
            .arg(QString::number(currentTrackNumber))
            .arg(QString::number(totalTracksCount));
    emit(featureIsLoading(this, false));
}

QVariant AnalysisFeature::title() {
    return m_Title;
}

QIcon AnalysisFeature::getIcon() {
    return m_icon;
}

void AnalysisFeature::bindWidget(WLibrary* libraryWidget,
                                 KeyboardEventFilter* keyboard) {
    m_pAnalysisView = new DlgAnalysis(libraryWidget,
                                      m_pConfig,
                                      m_library);
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
    emit(analysisActive(static_cast<bool>(m_pTrackAnalysisScheduler)));

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
    if (!m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler = TrackAnalysisScheduler::createInstance(
                m_library,
                kNumberOfAnalyzerThreads,
                m_pConfig,
                getAnalyzerModeFlags(m_pConfig));

        connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::progress,
                m_pAnalysisView, &DlgAnalysis::onTrackAnalysisSchedulerProgress);
        connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::finished,
                m_pAnalysisView, &DlgAnalysis::onTrackAnalysisSchedulerFinished);
        connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::progress,
                this, &AnalysisFeature::onTrackAnalysisSchedulerProgress);
        connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::finished,
                this, &AnalysisFeature::stopAnalysis);

        emit(analysisActive(true));
    }

    if (m_pTrackAnalysisScheduler->scheduleTracksById(trackIds) > 0) {
        m_pTrackAnalysisScheduler->resume();
    }
}

void AnalysisFeature::onTrackAnalysisSchedulerProgress(
        AnalyzerProgress /*currentTrackProgress*/,
        int currentTrackNumber,
        int totalTracksCount) {
    // Ignore any delayed progress updates after the analysis
    // has already been stopped.
    if (m_pTrackAnalysisScheduler) {
        if (totalTracksCount > 0) {
            setTitleProgress(currentTrackNumber, totalTracksCount);
        } else {
            setTitleDefault();
        }
    }
}

void AnalysisFeature::suspendAnalysis() {
    //qDebug() << this << "suspendAnalysis";
    if (m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler->suspend();
    }
}

void AnalysisFeature::resumeAnalysis() {
    //qDebug() << this << "resumeAnalysis";
    if (m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler->resume();
    }
}

void AnalysisFeature::stopAnalysis() {
    //qDebug() << this << "stopAnalysis()";
    if (m_pTrackAnalysisScheduler) {
        // Free resources by abandoning the queue after the batch analysis
        // has completed. Batch analysis are not started very frequently
        // during a session and should be avoided while performing live.
        // If the user decides to start a new batch analysis the setup costs
        // for creating the queue with its worker threads are acceptable.
        m_pTrackAnalysisScheduler.reset();
    }
    setTitleDefault();
    emit(analysisActive(false));
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
