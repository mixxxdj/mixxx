#include "library/recording/dlgrecording.h"

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/browse/browselibrarytablemodel.h"
#include "library/browse/browsetablemodel.h"
#include "library/library.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgrecording.cpp"
#include "recording/recordingmanager.h"
#include "track/track.h"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

DlgRecording::DlgRecording(
        WLibrary* pLibraryWidget,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        RecordingManager* pRecordingManager,
        KeyboardEventFilter* pKeyboard)
        : QWidget(pLibraryWidget),
          m_pConfig(pConfig),
          m_pTrackCollection(
                  pLibrary->trackCollectionManager()->internalCollection()),
          m_pTrackTableView(make_parented<WTrackTableView>(
                  this,
                  pConfig,
                  pLibrary,
                  pLibraryWidget->getTrackTableBackgroundColorOpacity())),
          m_pBrowseModel(make_parented<BrowseTableModel>(
                  this, pLibrary->trackCollectionManager(), pRecordingManager)),
          m_pProxyModel(new ProxyTrackModel(m_pBrowseModel, true /* handle search */)),
          m_pLibraryTableModel(make_parented<BrowseLibraryTableModel>(this,
                  pLibrary->trackCollectionManager(),
                  "mixxx.db.model.libraryRecording")),
          m_pCurrentTrackModel(nullptr),
          m_bytesRecordedStr("--"),
          m_durationRecordedStr("--:--"),
          m_pRecordingManager(pRecordingManager) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &DlgRecording::slotLoadTrack);
    connect(m_pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &DlgRecording::loadTrackToPlayer);
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

    connect(m_pRecordingManager,
            &RecordingManager::isRecording,
            this,
            &DlgRecording::slotRecordingStateChanged);

    connect(m_pRecordingManager,
            &RecordingManager::bytesRecorded,
            this,
            &DlgRecording::slotBytesRecorded);
    connect(m_pRecordingManager,
            &RecordingManager::durationRecorded,
            this,
            &DlgRecording::slotDurationRecorded);
    connect(m_pBrowseModel,
            &BrowseTableModel::restoreModelState,
            m_pTrackTableView,
            &WTrackTableView::restoreCurrentViewState);

    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { //Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_pProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    connect(pushButtonRecording,
            &QPushButton::clicked,
            this,
            &DlgRecording::slotRecButtonClicked);

    labelRecPrefix->hide();
    labelRecFilename->hide();
    labelRecStatistics->hide();
    labelRecPrefix->setText(tr("Recording to file:"));

    // Sync GUI with recording state:
    // refresh labels, switch to file or library view
    slotRecordingStateChanged(m_pRecordingManager->isRecordingActive());
}

DlgRecording::~DlgRecording() {
}

bool DlgRecording::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgRecording::setFocus() {
    m_pTrackTableView->setFocus();
}

void DlgRecording::refreshBrowseModel() {
    qWarning() << "     .";
    qWarning() << "     DlgRec::refresh";
    qWarning() << "     .";
    saveCurrentViewState();
    const QString recordingDir = m_pRecordingManager->getRecordingDir();
    auto dirInfo = mixxx::FileInfo(recordingDir);
    bool isWatched = false;
    std::tie(isWatched, dirInfo) =
            m_pTrackCollection->getDirectoryDAO().isDirectoryWatched(dirInfo);
    if (isWatched) {
        m_pCurrentTrackModel = m_pLibraryTableModel;
        m_pLibraryTableModel->setPath(dirInfo.location());
        m_pTrackTableView->loadTrackModel(m_pLibraryTableModel);
    } else {
        m_pCurrentTrackModel = m_pProxyModel;
        m_pBrowseModel->setPath(mixxx::FileAccess(dirInfo));
        m_pTrackTableView->loadTrackModel(m_pProxyModel);
    }
    // emit restoreSearch(currentSearch());
    onSearch(currentSearch());

    restoreCurrentViewState();

    // Switching models is only necessary when the rec directory is added to or
    // removed from the library, so we don't take care of adopting the previous search here.
}

void DlgRecording::onSearch(const QString& text) {
    m_pCurrentTrackModel->search(text);
}

void DlgRecording::slotRestoreSearch() {
    emit restoreSearch(currentSearch());
}

void DlgRecording::slotRecButtonClicked(bool toggle) {
    Q_UNUSED(toggle);
    m_pRecordingManager->slotToggleRecording(1);
}

void DlgRecording::slotRecordingStateChanged(bool isRecording) {
    qWarning() << "     .";
    qWarning() << "     DlgRec::recChanged" << QString(isRecording ? "ON" : "OFF");
    if (isRecording) {
        pushButtonRecording->setChecked(true);
        pushButtonRecording->setText(tr("Stop Recording"));
        labelRecPrefix->show();
        labelRecFilename->show();
        labelRecStatistics->show();
        if (m_pCurrentTrackModel == m_pLibraryTableModel) {
            // Add the new recording file to the view
            // Add track to library (saves a rescan which would scan ALL directories)
            const QString recFileLoc = m_pRecordingManager->getRecordingLocation();
            m_pLibraryTableModel->addTracks(QModelIndex(), QStringList{recFileLoc});
        }
    } else {
        pushButtonRecording->setChecked(false);
        pushButtonRecording->setText(tr("Start Recording"));
        labelRecPrefix->hide();
        labelRecFilename->hide();
        labelRecStatistics->hide();
    }
    // This will update the recorded track table view
    refreshBrowseModel();
}

// gets number of recorded bytes and update label
void DlgRecording::slotBytesRecorded(int bytes) {
    double megabytes = bytes / 1048576.0;
    m_bytesRecordedStr = QString::number(megabytes,'f',2);
    refreshLabels();
}

// gets recorded duration and update label
void DlgRecording::slotDurationRecorded(const QString& durationRecorded) {
    m_durationRecordedStr = durationRecorded;
    refreshLabels();
}

// update label besides start/stop button
void DlgRecording::refreshLabels() {
    QString recFile = m_pRecordingManager->getRecordingFile();
    QString recData = QString(QStringLiteral("(") + tr("%1 MiB written in %2") +
            QStringLiteral(")"))
                              .arg(m_bytesRecordedStr, m_durationRecordedStr);
    labelRecFilename->setText(recFile);
    labelRecStatistics->setText(recData);
}

void DlgRecording::slotLoadTrack(TrackPointer pTrack) {
    // TODO Try to adopt block implementation from BrowseTableModel::getTrackByref
    if (!pTrack) {
        return;
    }
    const QString recFileLoc = m_pRecordingManager->getRecordingLocation();
    const QString trackLoc = pTrack->getLocation();
    if (trackLoc == recFileLoc) {
        // Prevent loading the rec track
        return;
    }
    emit loadTrack(pTrack);
}

void DlgRecording::saveCurrentViewState() {
    m_pTrackTableView->saveCurrentViewState();
}

bool DlgRecording::restoreCurrentViewState() {
    return m_pTrackTableView->restoreCurrentViewState();
}

QString DlgRecording::currentSearch() const {
    return m_pCurrentTrackModel->currentSearch();
}
