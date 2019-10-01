#include <QDesktopServices>

#include "control/controlobject.h"
#include "library/recording/dlgrecording.h"
#include "library/trackcollection.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgRecording::DlgRecording(QWidget* parent, UserSettingsPointer pConfig,
                           Library* pLibrary, TrackCollection* pTrackCollection,
                           RecordingManager* pRecordingManager, KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_browseModel(this, m_pTrackCollection, pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_bytesRecordedStr("--"),
          m_durationRecordedStr("--:--"),
          m_pRecordingManager(pRecordingManager) {
    setupUi(this);
    m_pTrackTableView = new WTrackTableView(this, pConfig, m_pTrackCollection, true);
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
            &DlgRecording::slotRecordingEnabled);
    connect(m_pRecordingManager,
            &RecordingManager::bytesRecorded,
            this,
            &DlgRecording::slotBytesRecorded);
    connect(m_pRecordingManager,
            &RecordingManager::durationRecorded,
            this,
            &DlgRecording::slotDurationRecorded);

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
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
            &QPushButton::toggled,
            this,
            &DlgRecording::toggleRecording);
    label->setText("");
    label->setEnabled(false);
}

DlgRecording::~DlgRecording() {
}

void DlgRecording::onShow() {
    m_recordingDir = m_pRecordingManager->getRecordingDir();
    m_browseModel.setPath(m_recordingDir);
}

bool DlgRecording::hasFocus() const {
    return QWidget::hasFocus();
}

void DlgRecording::refreshBrowseModel() {
     m_browseModel.setPath(m_recordingDir);
}

void DlgRecording::onSearch(const QString& text) {
    m_proxyModel.search(text);
}

void DlgRecording::slotRestoreSearch() {
    emit(restoreSearch(currentSearch()));
}

void DlgRecording::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgRecording::slotSendToAutoDJBottom() {
    m_pTrackTableView->slotSendToAutoDJBottom();
}

void DlgRecording::slotSendToAutoDJTop() {
    m_pTrackTableView->slotSendToAutoDJTop();
}

void DlgRecording::slotSendToAutoDJReplace() {
    m_pTrackTableView->slotSendToAutoDJReplace();
}

void DlgRecording::loadSelectedTrackToGroup(QString group, bool play) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, play);
}

void DlgRecording::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgRecording::toggleRecording(bool toggle) {
    Q_UNUSED(toggle);
    if (!m_pRecordingManager->isRecordingActive()) //If recording is enabled
    {
        //pushButtonRecording->setText(tr("Stop Recording"));
        m_pRecordingManager->startRecording();
    }
    else if(m_pRecordingManager->isRecordingActive()) //If we disable recording
    {
        //pushButtonRecording->setText(tr("Start Recording"));
        m_pRecordingManager->stopRecording();
    }
}

void DlgRecording::slotRecordingEnabled(bool isRecording) {
    if (isRecording) {
        pushButtonRecording->setText((tr("Stop Recording")));
        label->setEnabled(true);
    } else {
        pushButtonRecording->setText((tr("Start Recording")));
        label->setText("");
        label->setEnabled(false);
    }
    //This will update the recorded track table view
    m_browseModel.setPath(m_recordingDir);
}

// gets number of recorded bytes and update label
void DlgRecording::slotBytesRecorded(int bytes) {
    double megabytes = bytes / 1048576.0;
    m_bytesRecordedStr = QString::number(megabytes,'f',2);
    refreshLabel();
}

// gets recorded duration and update label
void DlgRecording::slotDurationRecorded(QString durationRecorded) {
    m_durationRecordedStr = durationRecorded;
    refreshLabel();
}

// update label besides start/stop button
void DlgRecording::refreshLabel() {
    QString text = tr("Recording to file: %1 (%2 MiB written in %3)")
              .arg(m_pRecordingManager->getRecordingFile())
              .arg(m_bytesRecordedStr)
              .arg(m_durationRecordedStr);
    label->setText(text);
 }
