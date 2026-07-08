#include "library/musicbrainzqueue/dlgmusicbrainzqueue.h"

#include <QItemSelection>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/dao/trackfingerprintdao.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/musicbrainzqueue/dlgacoustidsubmit.h"
#include "library/musicbrainzqueue/musicbrainzqueuetablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgmusicbrainzqueue.cpp"
#include "musicbrainz/acoustidworker.h"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

DlgMusicBrainzQueue::DlgMusicBrainzQueue(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          Ui::DlgMusicBrainzQueue(),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity())),
          m_pQueueTableModel(nullptr),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig) {
    setupUi(this);
    m_pTrackTableView->installEventFilter(pKeyboard);

    // Swap the placeholder QTableView defined in the .ui file for a real
    // WTrackTableView — identical to the pattern used in DlgMissing.
    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    }
    else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_pQueueTableModel = new MusicBrainzQueueTableModel(
            this, pLibrary->trackCollectionManager());
    m_pTrackTableView->loadTrackModel(m_pQueueTableModel);

    connect(btnSelectAll,
            &QPushButton::clicked,
            this,
            &DlgMusicBrainzQueue::selectAll);
    connect(btnSubmitSelected,
            &QPushButton::clicked,
            this,
            &DlgMusicBrainzQueue::slotSubmitSelected);
    connect(btnSubmitAll,
            &QPushButton::clicked,
            this,
            &DlgMusicBrainzQueue::slotSubmitAll);
    connect(btnRetryFailed,
            &QPushButton::clicked,
            this,
            &DlgMusicBrainzQueue::slotRetryFailed);
    connect(m_pTrackTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgMusicBrainzQueue::selectionChanged);
    connect(m_pTrackTableView,
            &WTrackTableView::trackSelected,
            this,
            &DlgMusicBrainzQueue::trackSelected);

    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pTrackTableView,
            &WTrackTableView::setSelectedClick);
    connect(pLibrary->trackCollectionManager(),
            &TrackCollectionManager::acoustIdQueueDrained,
            this,
            &DlgMusicBrainzQueue::slotAcoustIdQueueDrained);
}

DlgMusicBrainzQueue::~DlgMusicBrainzQueue() {
    // Delete m_pTrackTableView before the table model. The view saves its
    // header state using the model on destruction — same ordering as DlgMissing.
    delete m_pTrackTableView;
    delete m_pQueueTableModel;
}

void DlgMusicBrainzQueue::onShow() {
    m_pQueueTableModel->select();
    activateButtons(false);
}

void DlgMusicBrainzQueue::onSearch(const QString& text) {
    m_pQueueTableModel->search(text);
}

QString DlgMusicBrainzQueue::currentSearch() {
    return m_pQueueTableModel->currentSearch();
}

void DlgMusicBrainzQueue::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgMusicBrainzQueue::activateButtons(bool enable) {
    btnSubmitSelected->setEnabled(enable);
}

void DlgMusicBrainzQueue::selectionChanged(
        const QItemSelection& selected,
        const QItemSelection& /*deselected*/) {
    activateButtons(!selected.indexes().isEmpty());
}

void DlgMusicBrainzQueue::slotAcoustIdQueueDrained() {
    m_pQueueTableModel->select();
}

bool DlgMusicBrainzQueue::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgMusicBrainzQueue::setFocus() {
    m_pTrackTableView->setFocus();
}

void DlgMusicBrainzQueue::saveCurrentViewState() {
    m_pTrackTableView->saveCurrentViewState();
}

bool DlgMusicBrainzQueue::restoreCurrentViewState() {
    return m_pTrackTableView->restoreCurrentViewState();
}

bool DlgMusicBrainzQueue::confirmSubmitOrShowDialog(int trackCount) {
    using namespace mixxx::library::prefs;

    const QString savedKey = m_pConfig->getValue(
            kAcoustIdUserApiKeyConfigKey, QString());
    const bool autoSubmit = m_pConfig->getValue(
            kAcoustIdAutoSubmitConfigKey, false);

    if (!savedKey.isEmpty() && autoSubmit) {
        return true;
    }

    DlgAcoustIdSubmit dlg(this, m_pConfig, trackCount);
    if (dlg.exec() != QDialog::Accepted) {
        return false;
    }

    if (dlg.rememberKey()) {
        m_pConfig->set(
                kAcoustIdUserApiKeyConfigKey,
                ConfigValue{dlg.apiKey()});
        m_pConfig->set(
                kAcoustIdAutoSubmitConfigKey,
                ConfigValue{true});
    } else {
        m_pConfig->set(
                kAcoustIdUserApiKeyConfigKey,
                ConfigValue{dlg.apiKey()});
    }

    return true;
}

void DlgMusicBrainzQueue::enqueueAndWake(const QList<TrackId>& trackIds) {
    if (trackIds.isEmpty()) {
        return;
    }

    TrackFingerprintDao& dao =
            m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->getTrackFingerprintDAO();

    for (const TrackId& id : trackIds) {
        // INSERT OR IGNORE: safe to call even if the track is already queued.
        dao.enqueueAcoustId(id);
    }

    // Wake the background worker so it picks up the new jobs without waiting
    // for its next natural poll interval.
    auto* pWorker = m_pLibrary->trackCollectionManager()->acoustIdWorker();
    if (pWorker) {
        pWorker->slotWakeUp();
    }

    m_pQueueTableModel->select();
}

void DlgMusicBrainzQueue::slotSubmitSelected() {
    // Collect TrackIds for every selected row — same pattern as DlgAnalysis.
    QList<TrackId> trackIds;
    const QModelIndexList selectedIndexes =
            m_pTrackTableView->selectionModel()->selectedRows();
    for (const auto& selectedIndex : std::as_const(selectedIndexes)) {
        TrackId trackId(m_pQueueTableModel->getFieldVariant(
                selectedIndex, ColumnCache::COLUMN_LIBRARYTABLE_ID));
        if (trackId.isValid()) {
            trackIds.append(trackId);
        }
    }

    if (!confirmSubmitOrShowDialog(trackIds.size())) {
        return;
    }
    enqueueAndWake(trackIds);
}

void DlgMusicBrainzQueue::slotSubmitAll() {
    QList<TrackId> trackIds;
    for (int row = 0; row < m_pQueueTableModel->rowCount(); ++row) {
        TrackId trackId(m_pQueueTableModel->getFieldVariant(
                m_pQueueTableModel->index(row, 0),
                ColumnCache::COLUMN_LIBRARYTABLE_ID));
        if (trackId.isValid()) {
            trackIds.append(trackId);
        }
    }

    if (!confirmSubmitOrShowDialog(trackIds.size())) {
        return;
    }
    enqueueAndWake(trackIds);
}

void DlgMusicBrainzQueue::slotRetryFailed() {
    TrackFingerprintDao& dao =
            m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->getTrackFingerprintDAO();

    // Query the current unmatched set and reset only the failed jobs.
    // reQueueJob() sets status='queued', attempts=0, error_message=NULL.
    const QList<UnmatchedTrackInfo> unmatched = dao.getUnmatchedTracks();
    bool anyRetried = false;
    for (const UnmatchedTrackInfo& info : std::as_const(unmatched)) {
        if (info.queueStatus == QStringLiteral("failed")) {
            if (dao.reQueueJob(info.trackId)) {
                anyRetried = true;
            }
        }
    }

    if (anyRetried) {
        auto* pWorker = m_pLibrary->trackCollectionManager()->acoustIdWorker();
        if (pWorker) {
            pWorker->slotWakeUp();
        }
    }

    m_pQueueTableModel->select();
}
