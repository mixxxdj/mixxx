#include "library/recording/dlgrecording.h"

#include <QDesktopServices>

#include "control/controlobject.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgrecording.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "widget/wwidget.h"

DlgRecording::DlgRecording(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        RecordingManager* pRecordingManager,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity(),
                          true)),
          m_browseModel(this, pLibrary->trackCollections(), pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_bytesRecordedStr("--"),
          m_durationRecordedStr("--:--"),
          m_pRecordingManager(pRecordingManager) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &DlgRecording::loadTrack);
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

    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { //Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_recordingDir = m_pRecordingManager->getRecordingDir();

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

    m_browseModel.setPath(m_recordingDir);
    m_pTrackTableView->loadTrackModel(&m_proxyModel);

    connect(pushButtonRecording,
            &QPushButton::clicked,
            this,
            &DlgRecording::slotRecButtonClicked);

    labelRecPrefix->hide();
    labelRecFilename->hide();
    labelRecStatistics->hide();
    labelRecPrefix->setText(tr("Recording to file:"));

    // Sync GUI with recording state, also refreshes labels
    slotRecordingStateChanged(m_pRecordingManager->isRecordingActive());
}

DlgRecording::~DlgRecording() {
}

void DlgRecording::onShow() {
    m_recordingDir = m_pRecordingManager->getRecordingDir();
    m_browseModel.setPath(m_recordingDir);
}

bool DlgRecording::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgRecording::refreshBrowseModel() {
     m_browseModel.setPath(m_recordingDir);
}

void DlgRecording::onSearch(const QString& text) {
    m_proxyModel.search(text);
}

void DlgRecording::slotRestoreSearch() {
    emit restoreSearch(currentSearch());
}

void DlgRecording::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgRecording::slotAddToAutoDJBottom() {
    m_pTrackTableView->slotAddToAutoDJBottom();
}

void DlgRecording::slotAddToAutoDJTop() {
    m_pTrackTableView->slotAddToAutoDJTop();
}

void DlgRecording::slotAddToAutoDJReplace() {
    m_pTrackTableView->slotAddToAutoDJReplace();
}

void DlgRecording::loadSelectedTrackToGroup(const QString& group, bool play) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, play);
}

void DlgRecording::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgRecording::slotRecButtonClicked(bool toggle) {
    Q_UNUSED(toggle);
    m_pRecordingManager->slotToggleRecording(1);
}

void DlgRecording::slotRecordingStateChanged(bool isRecording) {
    if (isRecording) {
        pushButtonRecording->setChecked(true);
        pushButtonRecording->setText(tr("Stop Recording"));
        labelRecPrefix->show();
        labelRecFilename->show();
        labelRecStatistics->show();
    } else {
        pushButtonRecording->setChecked(false);
        pushButtonRecording->setText(tr("Start Recording"));
        labelRecPrefix->hide();
        labelRecFilename->hide();
        labelRecStatistics->hide();
    }
    //This will update the recorded track table view
    m_browseModel.setPath(m_recordingDir);
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
