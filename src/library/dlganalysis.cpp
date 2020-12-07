#include "library/dlganalysis.h"

#include <QSqlTableModel>

#include "analyzer/analyzerprogress.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlganalysis.cpp"
#include "util/assert.h"
#include "widget/wanalysislibrarytableview.h"
#include "widget/wlibrary.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

DlgAnalysis::DlgAnalysis(WLibrary* parent,
                       UserSettingsPointer pConfig,
                       Library* pLibrary)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_bAnalysisActive(false) {
    setupUi(this);
    m_songsButtonGroup.addButton(radioButtonRecentlyAdded);
    m_songsButtonGroup.addButton(radioButtonAllSongs);

    m_pAnalysisLibraryTableView = new WAnalysisLibraryTableView(
            this,
            pConfig,
            pLibrary->trackCollections(),
            parent->getTrackTableBackgroundColorOpacity());
    connect(m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::loadTrack,
            this,
            &DlgAnalysis::loadTrack);
    connect(m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::loadTrackToPlayer,
            this,
            &DlgAnalysis::loadTrackToPlayer);

    connect(m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::trackSelected,
            this,
            &DlgAnalysis::trackSelected);

    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pAnalysisLibraryTableView);
    }

    m_pAnalysisLibraryTableModel = new AnalysisLibraryTableModel(this, pLibrary->trackCollections());
    m_pAnalysisLibraryTableView->loadTrackModel(m_pAnalysisLibraryTableModel);

    connect(radioButtonRecentlyAdded,
            &QRadioButton::clicked,
            this,
            &DlgAnalysis::showRecentSongs);
    connect(radioButtonAllSongs,
            &QRadioButton::clicked,
            this,
            &DlgAnalysis::showAllSongs);
    // Don't click those radio buttons now reduce skin loading time.
    // 'RecentlyAdded' is clicked in onShow()

    connect(pushButtonAnalyze,
            &QPushButton::clicked,
            this,
            &DlgAnalysis::analyze);
    pushButtonAnalyze->setEnabled(false);

    connect(pushButtonSelectAll,
            &QPushButton::clicked,
            this,
            &DlgAnalysis::selectAll);

    connect(m_pAnalysisLibraryTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgAnalysis::tableSelectionChanged);

    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pAnalysisLibraryTableView,
            &WAnalysisLibraryTableView::setSelectedClick);

    slotAnalysisActive(m_bAnalysisActive);
}

void DlgAnalysis::onShow() {
    if (!radioButtonRecentlyAdded->isChecked() &&
            !radioButtonAllSongs->isChecked()) {
        radioButtonRecentlyAdded->click();
    }
    // Refresh table
    // There might be new tracks dropped to other views
    m_pAnalysisLibraryTableModel->select();
}

bool DlgAnalysis::hasFocus() const {
    return m_pAnalysisLibraryTableView->hasFocus();
}

void DlgAnalysis::onSearch(const QString& text) {
    m_pAnalysisLibraryTableModel->search(text);
}

void DlgAnalysis::loadSelectedTrack() {
    m_pAnalysisLibraryTableView->loadSelectedTrack();
}

void DlgAnalysis::loadSelectedTrackToGroup(const QString& group, bool play) {
    m_pAnalysisLibraryTableView->loadSelectedTrackToGroup(group, play);
}

void DlgAnalysis::slotAddToAutoDJBottom() {
    // append to auto DJ
    m_pAnalysisLibraryTableView->slotAddToAutoDJBottom();
}

void DlgAnalysis::slotAddToAutoDJTop() {
    m_pAnalysisLibraryTableView->slotAddToAutoDJTop();
}

void DlgAnalysis::slotAddToAutoDJReplace() {
    m_pAnalysisLibraryTableView->slotAddToAutoDJReplace();
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
        emit stopAnalysis();
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
        emit analyzeTracks(trackIds);
    }
}

void DlgAnalysis::slotAnalysisActive(bool bActive) {
    //qDebug() << this << "slotAnalysisActive" << bActive;
    m_bAnalysisActive = bActive;
    if (bActive) {
        pushButtonAnalyze->setChecked(true);
        pushButtonAnalyze->setText(tr("Stop Analysis"));
        labelProgress->setEnabled(true);
    } else {
        pushButtonAnalyze->setChecked(false);
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
