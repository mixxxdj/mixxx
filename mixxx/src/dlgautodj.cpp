#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "wtracktableview.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "dlgautodj.h"


DlgAutoDJ::DlgAutoDJ(QWidget* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
     : QWidget(parent), Ui::DlgAutoDJ(), m_playlistDao(pTrackCollection->getPlaylistDAO())
{
    setupUi(this);

    m_pConfig = pConfig;
    m_pTrackCollection = pTrackCollection;
    m_iNextTrackIndex = 0;
    m_bAutoDJEnabled = false;
    m_pTrackTableView = new WTrackTableView(this, pConfig);

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_pAutoDJTableModel =  new PlaylistTableModel(this, pTrackCollection);
    int playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    if (playlistId < 0) {
        m_playlistDao.createPlaylist(AUTODJ_TABLE);
        playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    }
    m_pAutoDJTableModel->setPlaylist(playlistId);
    m_pTrackTableView->loadTrackModel(m_pAutoDJTableModel);

    //Override some playlist-view properties:
    //Prevent drag and drop to the waveform or elsewhere so you can't preempt the Auto DJ queue...
    m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);

    connect(pushButtonAutoDJ, SIGNAL(toggled(bool)),
            this,  SLOT(toggleAutoDJ(bool))); _blah;

    m_pCOPlayPos1 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel1]", "playposition")));
    m_pCOPlayPos2 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel2]", "playposition")));
    m_pCOPlay1 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel1]", "play")));
    m_pCOPlay2 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel2]", "play")));
    m_pCOTrackEndMode1 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel1]", "TrackEndMode")));
    m_pCOTrackEndMode2 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel2]", "TrackEndMode")));
    m_pCOCrossfader = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Master]", "crossfader")));
}

DlgAutoDJ::~DlgAutoDJ()
{
}

void DlgAutoDJ::onShow()
{
    m_pAutoDJTableModel->select();
}

QWidget* DlgAutoDJ::getWidgetForMIDIControl()
{
    return m_pTrackTableView;
}

void DlgAutoDJ::setup(QDomNode node)
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

    pushButtonAutoDJ->setPalette(pal);
    //m_pTrackTableView->setPalette(pal); //Since we're getting this passed into us already created,
                                          //shouldn't need to set the palette.
}
void DlgAutoDJ::onSearchStarting()
{
}
void DlgAutoDJ::onSearchCleared()
{
}
void DlgAutoDJ::onSearch(const QString& text)
{
    m_pAutoDJTableModel->search(text);
}

void DlgAutoDJ::toggleAutoDJ(bool toggle)
{
    if (toggle) //Enable Auto DJ
    {
        m_iNextTrackIndex = 0;

        if (m_pCOPlay1->get() == 1.0f && m_pCOPlay2->get() == 1.0f) {
            qDebug() << "One player must be stopped before enabling Auto DJ mode";
            pushButtonAutoDJ->setChecked(false);
            return;
        }

        pushButtonAutoDJ->setText(tr("Disable Auto DJ"));
        m_bAutoDJEnabled = true;
        connect(m_pCOPlayPos1, SIGNAL(valueChanged(double)),
        this, SLOT(player1PositionChanged(double)));
        connect(m_pCOPlayPos2, SIGNAL(valueChanged(double)),
        this, SLOT(player2PositionChanged(double)));

        //Load the first song from the queue.
        if (!loadNextTrackFromQueue(false)) {
            //Queue was empty. Disable and return.
            pushButtonAutoDJ->setChecked(false);
            return;
        }

        //If Player 1 is playing and player 2 is stopped...
        if (m_pCOPlay1->get() == 1.0f && m_pCOPlay2->get() == 0.0f) {

        }
        //If Player 2 is playing and player 1 is stopped...
        else if (m_pCOPlay1->get() == 0.0f && m_pCOPlay2->get() == 1.0f) {

        }
        //If both players are stopped, start the first one (which should have just had a track loaded into it)
        else if (m_pCOPlay1->get() == 0.0f && m_pCOPlay2->get() == 0.0f) {
            m_pCOCrossfader->slotSet(-1.0f); //Move crossfader to the left!
            m_pCOTrackEndMode1->slotSet(1.0f); //Turn on NEXT mode to avoid race condition between async load
                                               //and "play" command.
            m_pCOPlay1->slotSet(1.0f); //Play the track in player 1
        }
    }
    else //Disable Auto DJ
    {
        pushButtonAutoDJ->setText(tr("Enable Auto DJ"));
        qDebug() << "Auto DJ disabled";
        m_bAutoDJEnabled = false;
        m_pCOPlayPos1->disconnect(this);
        m_pCOPlayPos2->disconnect(this);
        m_pCOTrackEndMode1->slotSet(0.0f); //Turn off NEXT mode
        m_pCOTrackEndMode2->slotSet(0.0f); //Turn off NEXT mode
    }
}

void DlgAutoDJ::player1PositionChanged(double value)
{
    const float posThreshold = 0.95; //95% playback is when we crossfade and do stuff
    if (value > posThreshold)
    {
        //Crossfade!
        float crossfadeValue = -1.0f + 2*(value-posThreshold)/(1.0f-posThreshold);
        m_pCOCrossfader->slotSet(crossfadeValue); //Move crossfader to the right!

        //If the second player is stopped, load a track into it and start
        //playing it!
        if (m_pCOPlay2->get() == 0.0f)
        {
            //Turn on STOP mode to tell Player 1 to stop at the end
            m_pCOTrackEndMode1->slotSet(0.0f);

            //Load the next track into Player 2
            if (!loadNextTrackFromQueue(true))
                return;

            //Turn on NEXT mode to tell Player 2 to start playing when the new track is loaded.
            //This helps us get around the fact that it takes time for the track to be loaded
            //and that is executed asynchronously (so we get around the race condition).
            m_pCOTrackEndMode2->slotSet(1.0f);
            m_pCOPlay2->slotSet(1.0f);
        }

        if (value == 1.0f)
        {
            m_pCOPlay1->slotSet(0.0f); //Stop the player
        }
    }
}

void DlgAutoDJ::player2PositionChanged(double value)
{
    const float posThreshold = 0.95; //95% playback is when we crossfade and do stuff
    if (value > posThreshold)
    {
        //Crossfade!
        float crossfadeValue = 1.0f - 2*(value-posThreshold)/(1.0f-posThreshold);
        m_pCOCrossfader->slotSet(crossfadeValue); //Move crossfader to the right!

        //If the first player is stopped, load a track into it and start
        //playing it!
        if (m_pCOPlay1->get() == 0.0f)
        {
            //Turn on STOP mode to tell Player 2 to stop at the end
            m_pCOTrackEndMode2->slotSet(0.0f);

            //Load the next track into player 1
            if (!loadNextTrackFromQueue(true))
                return;

            //Turn on NEXT mode to tell Player 1 to start playing when the new track is loaded.
            //This helps us get around the fact that it takes time for the track to be loaded
            //and that is executed asynchronously (so we get around the race condition).
            m_pCOTrackEndMode1->slotSet(1.0f);
            m_pCOPlay1->slotSet(1.0f);
        }

        if (value == 1.0f)
        {
            m_pCOPlay2->slotSet(0.0f); //Stop the player
        }
    }
}


bool DlgAutoDJ::loadNextTrackFromQueue(bool removeTopMostBeforeLoading)
{
    if (removeTopMostBeforeLoading) {
        //Only remove the top track if this isn't the start of Auto DJ mode.
        m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));
    }

    //Get the track at the top of the playlist...
    TrackInfoObject* nextTrack = m_pAutoDJTableModel->getTrack(m_pAutoDJTableModel->index(0, 0));

    if (!nextTrack) //We ran out of tracks in the queue...
    {
        //Disable auto DJ and return...
        pushButtonAutoDJ->setChecked(false);
        return false;
    }

    emit(loadTrack(nextTrack));

    return true;
}

