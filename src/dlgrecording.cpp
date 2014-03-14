#include <QDesktopServices>
#include <QDateTime>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "controlobject.h"
#include "library/trackcollection.h"

#include "dlgrecording.h"

DlgRecording::DlgRecording(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                           TrackCollection* pTrackCollection,
                           RecordingManager* pRecordingManager, MixxxKeyboard* pKeyboard)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_browseModel(this, m_pTrackCollection, pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_pRecordingManager(pRecordingManager) {
    setupUi(this);
    m_pTrackTableView = new WTrackTableView(this, pConfig, m_pTrackCollection, false); // No sorting
    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));

    connect(m_pRecordingManager, SIGNAL(isRecording(bool)),
            this, SLOT(slotRecordingEnabled(bool)));
    connect(m_pRecordingManager, SIGNAL(bytesRecorded(long)),
            this, SLOT(slotBytesRecorded(long)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_recordingDir = m_pRecordingManager->getRecordingDir();

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

    m_browseModel.setPath(m_recordingDir);
    m_pTrackTableView->loadTrackModel(&m_proxyModel);

    connect(pushButtonRecording, SIGNAL(toggled(bool)),
            this,  SLOT(toggleRecording(bool)));
    label->setText(tr("Start recording here ..."));
    // Read the duration from file
    m_durationStr = "--:--";
    m_timerToReadDuration = new QTimer(this);
    connect(m_timerToReadDuration, SIGNAL(timeout()),
            this, SLOT(slotReadDuration()));
}

DlgRecording::~DlgRecording() {
}

void DlgRecording::onShow() {
    m_recordingDir = m_pRecordingManager->getRecordingDir();
    m_browseModel.setPath(m_recordingDir);
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
        //This will update the recorded track table view
        m_browseModel.setPath(m_recordingDir);
        m_timerToReadDuration->start(50);
    } else {
        pushButtonRecording->setText((tr("Start Recording")));
        label->setText("Start recording here ...");
        m_timerToReadDuration->stop();
    }

}
/** int bytes: the number of recorded bytes within a session **/
void DlgRecording::slotBytesRecorded(long bytes) {
    double megabytes = bytes / 1048575.0;
    m_bytesRecorded = QString::number(megabytes,'f',2);
    refreshLabel();
}
void DlgRecording::slotReadDuration() {
    QString location = m_pRecordingManager->getRecordingLocation();
    if (location == "")
        return;

    TrackPointer pTrack = TrackPointer(new TrackInfoObject(location),
                                       &QObject::deleteLater);
    QString old = m_durationStr;
    m_durationStr = pTrack->getDurationStr().split(".").at(0);
    if (old != m_durationStr)
        refreshLabel();
}
void DlgRecording::refreshLabel() {
    QString message = tr("Recording to file: %1 (%2 MB written in %3)");
    QString text = message.arg(m_pRecordingManager->getRecordingFile(),
                               m_bytesRecorded,
                               m_durationStr);
    label->setText(text);
}
