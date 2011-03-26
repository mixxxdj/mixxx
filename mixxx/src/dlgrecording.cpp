#include <QDesktopServices>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "library/trackcollection.h"

#include "dlgrecording.h"



DlgRecording::DlgRecording(QWidget* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
    : QWidget(parent), Ui::DlgRecording(), m_browseTableModel(this)
{
    setupUi(this);

    // TODO: make browsertable model singleton
    // There are two broser threads at the moment!!!
    m_pConfig = pConfig;
    m_pTrackCollection = pTrackCollection;
    m_pTrackTableView = new WTrackTableView(this, pConfig, m_pTrackCollection);


    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    QString os_music_folder_dir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    os_music_folder_dir.append("/Traktor/Recordings");
    m_browseTableModel.setPath(os_music_folder_dir);
    m_pTrackTableView->loadTrackModel(&m_browseTableModel);

    //Override some playlist-view properties:
    //Prevent drag and drop to the waveform or elsewhere so you can't preempt the Auto DJ queue...
    m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);
    //Sort by the position column and lock it
    m_pTrackTableView->sortByColumn(0, Qt::AscendingOrder);
    m_pTrackTableView->setSortingEnabled(false);

    connect(pushButtonRecording, SIGNAL(toggled(bool)),
            this,  SLOT(toggleRecording(bool)));

    m_pRecordingCO = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Master]", "Record")));

}

DlgRecording::~DlgRecording()
{
}

void DlgRecording::onShow()
{
    //m_pAutoDJTableModel->select();
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

void DlgRecording::onSearch(const QString& text)
{
    //m_pAutoDJTableModel->search(text);
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
    if (toggle) //If recording is enabled
    {
        pushButtonRecording->setText(tr("Stop Recording"));
        qDebug() << "Stop recording";
    }
    else //If recording is disabled
    {
        pushButtonRecording->setText(tr("Start Recording"));
        qDebug() << "Start recording";

    }
}

