#include <QSqlTableModel>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "transposeproxymodel.h"
#include "widget/wanalysislibrarytableview.h"
#include "library/trackcollection.h"
#include "dlganalysis.h"
#include "util/assert.h"

DlgAnalysis::DlgAnalysis(QWidget* parent,
                       ConfigObject<ConfigValue>* pConfig,
                       TrackCollection* pTrackCollection)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_bAnalysisActive(false),
          m_tracksInQueue(0),
          m_currentTrack(0) {
    setupUi(this);
    m_songsButtonGroup.addButton(radioButtonRecentlyAdded);
    m_songsButtonGroup.addButton(radioButtonAllSongs);

    m_pAnalysisLibraryTableView = new WAnalysisLibraryTableView(this, pConfig, pTrackCollection);
    connect(m_pAnalysisLibraryTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pAnalysisLibraryTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    connect(m_pAnalysisLibraryTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    DEBUG_ASSERT_AND_HANDLE(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pAnalysisLibraryTableView);
    }

    m_pAnalysisLibraryTableModel = new AnalysisLibraryTableModel(this, pTrackCollection);
    m_pAnalysisLibraryTableView->loadTrackModel(m_pAnalysisLibraryTableModel);

    connect(radioButtonRecentlyAdded, SIGNAL(clicked()),
            this,  SLOT(showRecentSongs()));
    connect(radioButtonAllSongs, SIGNAL(clicked()),
            this,  SLOT(showAllSongs()));

    radioButtonRecentlyAdded->click();

    labelProgress->setText("");
    pushButtonAnalyze->setEnabled(false);
    connect(pushButtonAnalyze, SIGNAL(clicked()),
            this, SLOT(analyze()));

    connect(pushButtonSelectAll, SIGNAL(clicked()),
            this, SLOT(selectAll()));

    connect(m_pAnalysisLibraryTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));
}

DlgAnalysis::~DlgAnalysis() {
}

void DlgAnalysis::onShow() {
    // Refresh table
    // There might be new tracks dropped to other views
    m_pAnalysisLibraryTableModel->select();
}

void DlgAnalysis::onSearch(const QString& text) {
    m_pAnalysisLibraryTableModel->search(text);
}

void DlgAnalysis::loadSelectedTrack() {
    m_pAnalysisLibraryTableView->loadSelectedTrack();
}

void DlgAnalysis::loadSelectedTrackToGroup(QString group, bool play) {
    m_pAnalysisLibraryTableView->loadSelectedTrackToGroup(group, play);
}

void DlgAnalysis::moveSelection(int delta) {
    m_pAnalysisLibraryTableView->moveSelection(delta);
}

void DlgAnalysis::tableSelectionChanged(const QItemSelection& selected,
                                       const QItemSelection& deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    bool tracksSelected = m_pAnalysisLibraryTableView->selectionModel()->hasSelection();
    pushButtonAnalyze->setEnabled(tracksSelected || m_bAnalysisActive);
}

void DlgAnalysis::selectAll() {
    m_pAnalysisLibraryTableView->selectAll();
}

void DlgAnalysis::analyze() {
    //qDebug() << this << "analyze()";
    if (m_bAnalysisActive) {
        emit(stopAnalysis());
    } else {
        QList<int> trackIds;

        QModelIndexList selectedIndexes = m_pAnalysisLibraryTableView->selectionModel()->selectedRows();
        foreach(QModelIndex selectedIndex, selectedIndexes) {
            bool ok;
            int trackId = selectedIndex.sibling(
                selectedIndex.row(),
                m_pAnalysisLibraryTableModel->fieldIndex(LIBRARYTABLE_ID)).data().toInt(&ok);
            if (ok) {
                trackIds.append(trackId);
            }
        }
        m_currentTrack = 1;
        emit(analyzeTracks(trackIds));
    }
}

void DlgAnalysis::analysisActive(bool bActive) {
    qDebug() << this << "analysisActive" << bActive;
    m_bAnalysisActive = bActive;
    if (bActive) {
        pushButtonAnalyze->setEnabled(true);
        pushButtonAnalyze->setText(tr("Stop Analysis"));
    } else {
        pushButtonAnalyze->setText(tr("Analyze"));
        labelProgress->setText("");
    }
}

// slot
void DlgAnalysis::trackAnalysisFinished(int size) {
    qDebug() << "Analysis finished" << size << "tracks left";
    if (size > 0) {
        m_currentTrack = m_tracksInQueue - size + 1;
    }
}

// slot
void DlgAnalysis::trackAnalysisProgress(int progress) {
    if (m_bAnalysisActive) {
        QString text = tr("Analyzing %1/%2 %3%").arg(
                QString::number(m_currentTrack),
                QString::number(m_tracksInQueue),
                QString::number(progress));
        labelProgress->setText(text);
    }
}

int DlgAnalysis::getNumTracks() {
	return m_tracksInQueue;
}

void DlgAnalysis::trackAnalysisStarted(int size) {
    m_tracksInQueue = size;
}

void DlgAnalysis::showRecentSongs() {
    m_pAnalysisLibraryTableModel->showRecentSongs();
}

void DlgAnalysis::showAllSongs() {
    m_pAnalysisLibraryTableModel->showAllSongs();
}

void DlgAnalysis::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pAnalysisLibraryTableView->installEventFilter(pFilter);
}
