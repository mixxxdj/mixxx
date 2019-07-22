#include <QSqlTableModel>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wanalysislibrarytableview.h"
#include "analyzer/analyzerprogress.h"
#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/dlganalysis.h"
#include "library/library.h"
#include "util/assert.h"

DlgAnalysis::DlgAnalysis(QWidget* parent,
                       UserSettingsPointer pConfig,
                       Library* pLibrary)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(&pLibrary->trackCollection()),
          m_bAnalysisActive(false) {
    setupUi(this);
    m_songsButtonGroup.addButton(radioButtonRecentlyAdded);
    m_songsButtonGroup.addButton(radioButtonAllSongs);

    m_pAnalysisLibraryTableView = new WAnalysisLibraryTableView(this, pConfig, m_pTrackCollection);
    connect(m_pAnalysisLibraryTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pAnalysisLibraryTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    connect(m_pAnalysisLibraryTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pAnalysisLibraryTableView);
    }

    m_pAnalysisLibraryTableModel = new AnalysisLibraryTableModel(this, m_pTrackCollection);
    m_pAnalysisLibraryTableView->loadTrackModel(m_pAnalysisLibraryTableModel);

    connect(radioButtonRecentlyAdded, SIGNAL(clicked()),
            this,  SLOT(showRecentSongs()));
    connect(radioButtonAllSongs, SIGNAL(clicked()),
            this,  SLOT(showAllSongs()));

    // TODO(rryan): This triggers a library search before the UI has even
    // started up. Accounts for 0.2% of skin creation time. Get rid of this!
    radioButtonRecentlyAdded->click();

    connect(pushButtonAnalyze, SIGNAL(clicked()),
            this, SLOT(analyze()));

    connect(pushButtonSelectAll, SIGNAL(clicked()),
            this, SLOT(selectAll()));

    connect(m_pAnalysisLibraryTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));

    connect(pLibrary, SIGNAL(setTrackTableFont(QFont)),
            m_pAnalysisLibraryTableView, SLOT(setTrackTableFont(QFont)));
    connect(pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            m_pAnalysisLibraryTableView, SLOT(setTrackTableRowHeight(int)));
    connect(pLibrary, SIGNAL(setSelectedClick(bool)),
            m_pAnalysisLibraryTableView, SLOT(setSelectedClick(bool)));

    slotAnalysisActive(m_bAnalysisActive);
}

void DlgAnalysis::onShow() {
    // Refresh table
    // There might be new tracks dropped to other views
    m_pAnalysisLibraryTableModel->select();
}

bool DlgAnalysis::hasFocus() const {
    return QWidget::hasFocus();
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

void DlgAnalysis::slotSendToAutoDJBottom() {
    // append to auto DJ
    m_pAnalysisLibraryTableView->slotSendToAutoDJBottom();
}

void DlgAnalysis::slotSendToAutoDJTop() {
    m_pAnalysisLibraryTableView->slotSendToAutoDJTop();
}

void DlgAnalysis::slotSendToAutoDJReplace() {
    m_pAnalysisLibraryTableView->slotSendToAutoDJReplace();
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
        QList<TrackId> trackIds;

        QModelIndexList selectedIndexes = m_pAnalysisLibraryTableView->selectionModel()->selectedRows();
        foreach(QModelIndex selectedIndex, selectedIndexes) {
            TrackId trackId(selectedIndex.sibling(
                selectedIndex.row(),
                m_pAnalysisLibraryTableModel->fieldIndex(LIBRARYTABLE_ID)).data());
            if (trackId.isValid()) {
                trackIds.append(trackId);
            }
        }
        emit(analyzeTracks(trackIds));
    }
}

void DlgAnalysis::slotAnalysisActive(bool bActive) {
    //qDebug() << this << "slotAnalysisActive" << bActive;
    m_bAnalysisActive = bActive;
    if (bActive) {
        pushButtonAnalyze->setEnabled(true);
        pushButtonAnalyze->setText(tr("Stop Analysis"));
        labelProgress->setEnabled(true);
    } else {
        pushButtonAnalyze->setText(tr("Analyze"));
        labelProgress->setText("");
        labelProgress->setEnabled(false);
    }
}

void DlgAnalysis::onTrackAnalysisSchedulerProgress(
        AnalyzerProgress analyzerProgress, int finishedCount, int totalCount) {
    //qDebug() << this << "onTrackAnalysisSchedulerProgress" << analyzerProgress << finishedCount << totalCount;
    if (labelProgress->isEnabled()) {
        QString progressText;
        if (analyzerProgress >= kAnalyzerProgressNone) {
            QString progressPercent = QString::number(
                    analyzerProgressPercent(analyzerProgress));
            progressText = tr("Analyzing %1% %2/%3").arg(
                    progressPercent,
                    QString::number(finishedCount),
                    QString::number(totalCount));
        } else {
            // Omit to display any percentage
            progressText = tr("Analyzing %1/%2").arg(
                    QString::number(finishedCount),
                    QString::number(totalCount));
        }
        labelProgress->setText(progressText);
    }
}

void DlgAnalysis::onTrackAnalysisSchedulerFinished() {
    slotAnalysisActive(false);
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
