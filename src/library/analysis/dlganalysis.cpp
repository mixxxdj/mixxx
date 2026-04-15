#include "library/analysis/dlganalysis.h"

#include <QKeyEvent>
#include <QLineEdit>

#include "analyzer/analyzerprogress.h"
#include "analyzer/analyzerscheduledtrack.h"
#include "control/controlobject.h"
#include "library/analysis/ui_dlganalysis.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "moc_dlganalysis.cpp"
#include "util/assert.h"
#include "widget/wanalysislibrarytableview.h"
#include "widget/wlibrary.h"

namespace {
const char* kPreferenceGroup = "[Analysis]";
const char* kRecentDaysConfigKey = "RecentDays";
} // anonymous namespace

DlgAnalysis::DlgAnalysis(WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_bAnalysisActive(false) {
    setupUi(this);
    m_songsButtonGroup.addButton(radioButtonRecentlyAdded);
    m_songsButtonGroup.addButton(radioButtonAllSongs);

    // Load the recent days value from config
    const unsigned int recentDays = m_pConfig->getValue<unsigned int>(
            ConfigKey(kPreferenceGroup, kRecentDaysConfigKey),
            AnalysisLibraryTableModel::kDefaultRecentDays);
    spinBoxRecentDays->setValue(static_cast<int>(recentDays));
    spinBoxRecentDays->setFocusPolicy(Qt::ClickFocus);

    // work around QLineEdit being protected
    QLineEdit* lineEditRecentDays(spinBoxRecentDays->findChild<QLineEdit*>());
    if (lineEditRecentDays) {
        lineEditRecentDays->setFocusPolicy(Qt::ClickFocus);
    }

    m_pAnalysisLibraryTableView = new WAnalysisLibraryTableView(
            this,
            pConfig,
            pLibrary,
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
    }
    else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pAnalysisLibraryTableView);
    }

    m_pAnalysisLibraryTableModel = new AnalysisLibraryTableModel(
            this, pLibrary->trackCollectionManager());
    // Initialize model with the configured days value
    m_pAnalysisLibraryTableModel->setRecentDays(recentDays);
    m_pAnalysisLibraryTableView->loadTrackModel(m_pAnalysisLibraryTableModel);

    connect(radioButtonRecentlyAdded,
            &QRadioButton::clicked,
            this,
            &DlgAnalysis::slotShowRecentSongs);
    connect(radioButtonAllSongs,
            &QRadioButton::clicked,
            this,
            &DlgAnalysis::slotShowAllSongs);

    connect(spinBoxRecentDays,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgAnalysis::slotRecentDaysChanged);
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

void DlgAnalysis::setFocus() {
    m_pAnalysisLibraryTableView->setFocus();
}

void DlgAnalysis::onSearch(const QString& text) {
    m_pAnalysisLibraryTableModel->searchCurrentTrackSet(
            text, radioButtonRecentlyAdded->isChecked());
}

void DlgAnalysis::tableSelectionChanged(const QItemSelection&,
        const QItemSelection&) {
    bool tracksSelected = m_pAnalysisLibraryTableView->selectionModel()->hasSelection();
    pushButtonAnalyze->setEnabled(tracksSelected || m_bAnalysisActive);
}

void DlgAnalysis::selectAll() {
    m_pAnalysisLibraryTableView->selectAll();
}

void DlgAnalysis::analyze() {
    // qDebug() << this << "analyze()";
    if (m_bAnalysisActive) {
        emit stopAnalysis();
    } else {
        QList<AnalyzerScheduledTrack> tracks;

        QModelIndexList selectedIndexes = m_pAnalysisLibraryTableView->selectionModel()->selectedRows();
        for (const auto& selectedIndex : std::as_const(selectedIndexes)) {
            TrackId trackId(m_pAnalysisLibraryTableModel->getFieldVariant(
                    selectedIndex, ColumnCache::COLUMN_LIBRARYTABLE_ID));
            if (trackId.isValid()) {
                tracks.append(trackId);
            }
        }
        emit analyzeTracks(tracks);
    }
}

void DlgAnalysis::slotAnalysisActive(bool bActive) {
    // qDebug() << this << "slotAnalysisActive" << bActive;
    m_bAnalysisActive = bActive;
    if (bActive) {
        pushButtonAnalyze->setChecked(true);
        pushButtonAnalyze->setText(tr("Stop Analysis"));
        pushButtonAnalyze->setEnabled(true);
        labelProgress->setEnabled(true);
    } else {
        pushButtonAnalyze->setChecked(false);
        pushButtonAnalyze->setText(tr("Analyze"));
        labelProgress->setText("");
        labelProgress->setEnabled(false);
    }
}

void DlgAnalysis::onTrackAnalysisSchedulerProgress(
        AnalyzerProgress, int finishedCount, int totalCount) {
    // qDebug() << this << "onTrackAnalysisSchedulerProgress" <<
    // analyzerProgress << finishedCount << totalCount;
    if (labelProgress->isEnabled()) {
        int totalProgressPercent = 0;
        if (totalCount > 0) {
            totalProgressPercent = (finishedCount * 100) / totalCount;
            if (totalProgressPercent > 100) {
                totalProgressPercent = 100;
            }
        }

        labelProgress->setText(tr("Analyzing %1/%2")
                                       .arg(QString::number(finishedCount),
                                               QString::number(totalCount)) +
                QStringLiteral(" (%3%)").arg(
                        QString::number(totalProgressPercent)));
    }
}

void DlgAnalysis::onTrackAnalysisSchedulerFinished() {
    slotAnalysisActive(false);
}

void DlgAnalysis::slotShowRecentSongs() {
    spinBoxRecentDays->setEnabled(true);
    labelRecentDaysAppendix->setEnabled(true);
    m_pAnalysisLibraryTableModel->showRecentSongs();
}

void DlgAnalysis::slotRecentDaysChanged(int days) {
    // Update the model's days value
    m_pAnalysisLibraryTableModel->setRecentDays(static_cast<unsigned int>(days));

    // Save to config
    m_pConfig->setValue(
            ConfigKey(kPreferenceGroup, kRecentDaysConfigKey),
            static_cast<unsigned int>(days));

    // If "Recently Added" is selected, refresh the view
    if (radioButtonRecentlyAdded->isChecked()) {
        m_pAnalysisLibraryTableModel->showRecentSongs();
    }
}

void DlgAnalysis::slotShowAllSongs() {
    spinBoxRecentDays->setEnabled(false);
    labelRecentDaysAppendix->setEnabled(false);
    m_pAnalysisLibraryTableModel->showAllSongs();
}

void DlgAnalysis::keyPressEvent(QKeyEvent* pEvent) {
    // If we receive key events the spinbox is focused.
    // Return, Enter and Escape move focus back to the previously focused
    // library widget in order to immediately allow keyboard shortcuts again.
    if (pEvent->key() == Qt::Key_Return ||
            pEvent->key() == Qt::Key_Enter ||
            pEvent->key() == Qt::Key_Escape) {
        ControlObject::set(ConfigKey("[Library]", "refocus_prev_widget"), 1);
        return;
    }
    QWidget::keyPressEvent(pEvent);
}

void DlgAnalysis::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pAnalysisLibraryTableView->installEventFilter(pFilter);
}

void DlgAnalysis::saveCurrentViewState() {
    m_pAnalysisLibraryTableView->saveCurrentViewState();
}

bool DlgAnalysis::restoreCurrentViewState() {
    return m_pAnalysisLibraryTableView->restoreCurrentViewState();
}
