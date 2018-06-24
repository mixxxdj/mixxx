// analysisfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <widget/wlibrarypane.h>
#include <QtDebug>

#include "analyzer/analyzerqueue.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/analysis/analysisfeature.h"
#include "library/features/analysis/dlganalysis.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wanalysislibrarytableview.h"
#include "widget/wtracktableview.h"

AnalysisFeature::AnalysisFeature(UserSettingsPointer pConfig,
                                 Library* pLibrary, TrackCollection* pTrackCollection,
                                 QObject* parent) :
        LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
        m_pDbConnectionPool(pLibrary->dbConnectionPool()),
        m_pAnalyzerQueue(nullptr),
        m_iOldBpmEnabled(0),
        m_analysisTitleName(tr("Analyze")),
        m_pAnalysisView(nullptr),
        m_analysisLibraryTableModel(this, m_pTrackCollection) {

    m_childModel.setRootItem(std::make_unique<TreeItem>(this));
    setTitleDefault();
}

AnalysisFeature::~AnalysisFeature() {
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

QString AnalysisFeature::getIconPath() {
    return ":/images/library/ic_library_prepare.png";
}

QString AnalysisFeature::getSettingsName() const {
    return "AnalysisFeature";
}

parented_ptr<QWidget> AnalysisFeature::createPaneWidget(KeyboardEventFilter*,
            int paneId, QWidget* parent) {
    auto pTable = createTableWidget(paneId, parent);
    pTable->loadTrackModel(&m_analysisLibraryTableModel);
    connect(pTable->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection&, const QItemSelection&)));

    return pTable;
}

parented_ptr<QWidget> AnalysisFeature::createInnerSidebarWidget(
            KeyboardEventFilter* pKeyboard, QWidget* parent) {
    auto pAnalysisView = make_parented<DlgAnalysis>(parent, this);

    m_pAnalysisView = pAnalysisView.toWeakRef();
    m_pAnalysisView->setTableModel(&m_analysisLibraryTableModel);

    connect(this, SIGNAL(analysisActive(bool)),
            m_pAnalysisView, SLOT(analysisActive(bool)));
    connect(this, SIGNAL(trackAnalysisStarted(int)),
            m_pAnalysisView, SLOT(trackAnalysisStarted(int)));

    m_pAnalysisView->installEventFilter(pKeyboard);

    // Let the DlgAnalysis know whether or not analysis is active.
    bool bAnalysisActive = m_pAnalyzerQueue != nullptr;
    emit(analysisActive(bAnalysisActive));

    m_pAnalysisView->onShow();

    return pAnalysisView;
}

QPointer<TreeItemModel> AnalysisFeature::getChildModel() {
    return &m_childModel;
}

void AnalysisFeature::refreshLibraryModels() {
    if (!m_pAnalysisView.isNull()) {
        m_pAnalysisView->onShow();
    }
}

void AnalysisFeature::selectAll() {
    QPointer<WTrackTableView> pTable = LibraryFeature::getFocusedTable();
    if (!pTable.isNull()) {
        pTable->selectAll();
    }
}

void AnalysisFeature::activate() {
    //qDebug() << "AnalysisFeature::activate()";
    switchToFeature();
    showBreadCrumb();

    if (!m_pAnalysisView.isNull()) {
        restoreSearch(m_pAnalysisView->currentSearch());
    }
}

namespace {
    inline
    AnalyzerQueue::Mode getAnalyzerQueueMode(
            const UserSettingsPointer& pConfig) {
        if (pConfig->getValue<bool>(ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true)) {
            return AnalyzerQueue::Mode::Default;
        } else {
            return AnalyzerQueue::Mode::WithoutWaveform;
        }
    }
} // anonymous namespace

void AnalysisFeature::analyzeTracks(QList<TrackId> trackIds) {
    if (m_pAnalyzerQueue == nullptr) {
        // Save the old BPM detection prefs setting (on or off)
        m_iOldBpmEnabled = m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();
        // Force BPM detection to be on.
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(1));
        // Note: this sucks... we should refactor the prefs/analyzer to fix this hacky bit ^^^^.

        m_pAnalyzerQueue = new AnalyzerQueue(
                m_pDbConnectionPool,
                m_pConfig,
                getAnalyzerQueueMode(m_pConfig));

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
    if (m_pAnalyzerQueue != nullptr) {
        m_pAnalyzerQueue->stop();
    }
}

void AnalysisFeature::cleanupAnalyzer() {
    setTitleDefault();
    emit(analysisActive(false));
    if (m_pAnalyzerQueue != nullptr) {
        m_pAnalyzerQueue->stop();
        m_pAnalyzerQueue->deleteLater();
        m_pAnalyzerQueue = nullptr;
        // Restore old BPM detection setting for preferences...
        m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(m_iOldBpmEnabled));
    }
}

void AnalysisFeature::tableSelectionChanged(const QItemSelection&,
                                            const QItemSelection&) {
    //qDebug() << "AnalysisFeature::tableSelectionChanged" << sender();
    QPointer<WTrackTableView> pTable = LibraryFeature::getFocusedTable();
    if (pTable.isNull()) {
        return;
    }

    QModelIndexList indexes = pTable->selectionModel()->selectedIndexes();
    m_pAnalysisView->setSelectedIndexes(indexes);
}

bool AnalysisFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    Q_UNUSED(pSource);
    auto files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    // Adds track, does not insert duplicates, handles unremoving logic.
    auto trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    analyzeTracks(trackIds);
    return trackIds.size() > 0;
}

bool AnalysisFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url);
}
