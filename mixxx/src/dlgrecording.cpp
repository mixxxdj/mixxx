#include <QDesktopServices>
#include <QDateTime>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
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
    m_pTrackTableView = new WTrackTableView(this, pConfig, m_pTrackCollection);
    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

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

    //Sort by the position column and lock it
    m_pTrackTableView->sortByColumn(0, Qt::AscendingOrder);
    m_pTrackTableView->setSortingEnabled(false);

    connect(pushButtonRecording, SIGNAL(toggled(bool)),
            this,  SLOT(toggleRecording(bool)));
    label->setText(tr("Start recording here ..."));

}

DlgRecording::~DlgRecording()
{

}

void DlgRecording::onShow()
{
    m_recordingDir = m_pRecordingManager->getRecordingDir();
    m_browseModel.setPath(m_recordingDir);
}

void DlgRecording::setup(QDomNode node)
{

    QPalette pal = palette();

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull() &&
        !WWidget::selectNode(node, "BgColorRowUneven").isNull()) {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        r1 = WSkinColor::getCorrectColor(r1);
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        r2 = WSkinColor::getCorrectColor(r2);

        // For now make text the inverse of the background so it's readable In
        // the future this should be configurable from the skin with this as the
        // fallback option
        QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());

        //setAlternatingRowColors ( true );

        QColor fgColor;
        fgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
        fgColor = WSkinColor::getCorrectColor(fgColor);

        pal.setColor(QPalette::Base, r1);
        pal.setColor(QPalette::AlternateBase, r2);
        pal.setColor(QPalette::Text, text);
        pal.setColor(QPalette::WindowText, fgColor);

    }

    setPalette(pal);

    pushButtonRecording->setPalette(pal);
    //m_pTrackTableView->setPalette(pal); //Since we're getting this passed into us already created,
                                          //shouldn't need to set the palette.
}

void DlgRecording::onSearchStarting()
{
}

void DlgRecording::onSearchCleared()
{
}

void DlgRecording::refreshBrowseModel(){
     m_browseModel.setPath(m_recordingDir);
}

void DlgRecording::onSearch(const QString& text)
{
    m_proxyModel.search(text);
}

void DlgRecording::slotRestoreSearch() {
    emit(restoreSearch(currentSearch()));
}

void DlgRecording::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgRecording::loadSelectedTrackToGroup(QString group) {
    m_pTrackTableView->loadSelectedTrackToGroup(group);
}

void DlgRecording::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgRecording::toggleRecording(bool toggle)
{
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
void DlgRecording::slotRecordingEnabled(bool isRecording)
{
    if(isRecording){
        pushButtonRecording->setText((tr("Stop Recording")));
        //This will update the recorded track table view
        m_browseModel.setPath(m_recordingDir);
    }
    else{
        pushButtonRecording->setText((tr("Start Recording")));
        label->setText("Start recording here ...");
    }

}
/** int bytes: the number of recorded bytes within a session **/
void DlgRecording::slotBytesRecorded(long bytes)
{
    double megabytes = bytes / 1048575.0f;
    QString byteStr = QString::number(megabytes,'f',2);
    QString text = (tr("Recording to file: ")) +m_pRecordingManager->getRecordingFile();
    /* TRANSLATOR: The size of the file which has been stored during the current recording in megabytes (MB) */
    text.append(" ( " +byteStr+ (tr("MB written")) +" )");
    label->setText(text);
}
