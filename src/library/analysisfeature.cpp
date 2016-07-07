// analysisfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <QtDebug>

#include "analyzer/analyzerqueue.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/analysisfeature.h"
#include "library/dlganalysis.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "sources/soundsourceproxy.h"
#include "util/debug.h"
#include "util/dnd.h"
#include "widget/wanalysislibrarytableview.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

AnalysisFeature::AnalysisFeature(UserSettingsPointer pConfig,
                                 Library* pLibrary, TrackCollection* pTrackCollection,
                                 QObject* parent) :
        LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
        m_pAnalyzerQueue(nullptr),
        m_iOldBpmEnabled(0),
        m_analysisTitleName(tr("Analyze")),
        m_pAnalysisView(nullptr){
    
    m_childModel.setRootItem(new TreeItem("$root", "$root", this, nullptr));
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

QWidget* AnalysisFeature::createPaneWidget(KeyboardEventFilter* pKeyboard,
                                           int paneId) {        
    WTrackTableView* pTable = LibraryFeature::createTableWidget(pKeyboard, paneId);
    pTable->loadTrackModel(getAnalysisTableModel());
    connect(pTable->selectionModel(), 
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, 
            SLOT(tableSelectionChanged(const QItemSelection&, const QItemSelection&)));
    
    return pTable;
}

QWidget* AnalysisFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pAnalysisView = new DlgAnalysis(nullptr, this, m_pTrackCollection);
    
    m_pAnalysisView->setTableModel(getAnalysisTableModel());
    
    connect(this, SIGNAL(analysisActive(bool)),
            m_pAnalysisView, SLOT(analysisActive(bool)));
    connect(this, SIGNAL(trackAnalysisStarted(int)),
            m_pAnalysisView, SLOT(trackAnalysisStarted(int)));

    m_pAnalysisView->installEventFilter(pKeyboard);
    
    // Let the DlgAnalysis know whether or not analysis is active.
    bool bAnalysisActive = m_pAnalyzerQueue != nullptr;
    emit(analysisActive(bAnalysisActive));
    m_pAnalysisView->onShow();
    
    return m_pAnalysisView;
}

TreeItemModel* AnalysisFeature::getChildModel() {
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
    //m_pLibrary->switchToView(m_sAnalysisViewName);
    m_pLibrary->switchToFeature(this);
    m_pLibrary->showBreadCrumb(m_childModel.getItem(QModelIndex()));
    
    if (!m_pAnalysisView.isNull()) {
        m_pLibrary->restoreSearch(m_pAnalysisView->currentSearch());
    }
    emit(enableCoverArtDisplay(true));
}

void AnalysisFeature::analyzeTracks(QList<TrackId> trackIds) {
    if (m_pAnalyzerQueue == nullptr) {
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
    
    const QModelIndexList &indexes = pTable->selectionModel()->selectedIndexes();
    m_pAnalysisView->setSelectedIndexes(indexes);
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

AnalysisLibraryTableModel* AnalysisFeature::getAnalysisTableModel() {
    if (m_pAnalysisLibraryTableModel.isNull()) {
        m_pAnalysisLibraryTableModel = 
                new AnalysisLibraryTableModel(this, m_pTrackCollection);
    }
    
    return m_pAnalysisLibraryTableModel;
}
