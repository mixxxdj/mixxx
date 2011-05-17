#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "playerinfo.h"
#include "dlgautodj.h"



DlgAutoDJ::DlgAutoDJ(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard)
     : QWidget(parent), Ui::DlgAutoDJ(), m_playlistDao(pTrackCollection->getPlaylistDAO())
{
    setupUi(this);

    m_pConfig = pConfig;
    m_pTrackCollection = pTrackCollection;
    m_bAutoDJEnabled = false;
	m_bFadeNow = false;
	m_eState = ADJ_IDLE;

	m_posThreshold = 0.95; //95% playback is when we crossfade and do stuff

    m_pTrackTableView = new WTrackTableView(this, pConfig, m_pTrackCollection);
    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_pAutoDJTableModel =  new PlaylistTableModel(this, pTrackCollection);
    int playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    if (playlistId < 0) {
        m_playlistDao.createPlaylist(AUTODJ_TABLE, true);
        playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    }
    m_pAutoDJTableModel->setPlaylist(playlistId);
    m_pTrackTableView->loadTrackModel(m_pAutoDJTableModel);

    //Override some playlist-view properties:
    //Prevent drag and drop to the waveform or elsewhere so you can't preempt the Auto DJ queue...
    m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);
    //Sort by the position column and lock it
    m_pTrackTableView->sortByColumn(0, Qt::AscendingOrder);
    m_pTrackTableView->setSortingEnabled(false);

    connect(pushButtonShuffle, SIGNAL(clicked(bool)),
            this, SLOT(shufflePlaylist(bool)));

    connect(pushButtonSkipNext, SIGNAL(clicked(bool)),
            this, SLOT(skipNext(bool)));

    connect(pushButtonFadeNow, SIGNAL(clicked(bool)),
            this, SLOT(fadeNow(bool)));

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
    m_pCORepeat1 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel1]", "repeat")));
    m_pCORepeat2 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel2]", "repeat")));
    m_pCOCrossfader = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Master]", "crossfader")));
}

DlgAutoDJ::~DlgAutoDJ()
{
    delete m_pCOPlayPos1;
    delete m_pCOPlayPos2;
    delete m_pCOPlay1;
    delete m_pCOPlay2;
	delete m_pCORepeat1;
    delete m_pCORepeat2;
    delete m_pCOCrossfader;
}

void DlgAutoDJ::onShow()
{
    m_pAutoDJTableModel->select();
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

void DlgAutoDJ::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgAutoDJ::loadSelectedTrackToGroup(QString group) {
    m_pTrackTableView->loadSelectedTrackToGroup(group);
}

void DlgAutoDJ::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgAutoDJ::shufflePlaylist(bool buttonChecked)
{
    Q_UNUSED(buttonChecked);
    m_pTrackTableView->sortByColumn(0, Qt::AscendingOrder);
    qDebug() << "Shuffling AutoDJ playlist";
    m_pAutoDJTableModel->shuffleTracks(m_pAutoDJTableModel->index(0, 0));
    qDebug() << "Shuffling done";
}

void DlgAutoDJ::skipNext(bool buttonChecked)
{
    Q_UNUSED(buttonChecked);
    //m_pTrackTableView->sortByColumn(0, Qt::AscendingOrder);
    qDebug() << "Skip Next";
    //Load the next song from the queue.
    if (!loadNextTrackFromQueue()) {
        //Queue was empty. Disable and return.
        return;
    }
}

void DlgAutoDJ::fadeNow(bool buttonChecked)
{
	Q_UNUSED(buttonChecked);
    qDebug() << "Fade Now";
	if(m_eState == ADJ_IDLE && m_bAutoDJEnabled){		
		m_bFadeNow = true;
	}
}

void DlgAutoDJ::toggleAutoDJ(bool toggle)
{
    if (toggle) //Enable Auto DJ
    {
        if (m_pCOPlay1->get() == 1.0f && m_pCOPlay2->get() == 1.0f) {
            qDebug() << "One player must be stopped before enabling Auto DJ mode";
            pushButtonAutoDJ->setChecked(false);
            return;
        }

		//Load the first song from the queue.
        if (!loadNextTrackFromQueue()) {
            //Queue was empty. Disable and return.
            return;
        }

        pushButtonAutoDJ->setText(tr("Disable Auto DJ"));
		qDebug() << "Auto DJ enabled";
        m_bAutoDJEnabled = true;
        connect(m_pCOPlayPos1, SIGNAL(valueChanged(double)),
        this, SLOT(player1PositionChanged(double)));
        connect(m_pCOPlayPos2, SIGNAL(valueChanged(double)),
        this, SLOT(player2PositionChanged(double)));
		
		if (m_pCOPlay1->get() == 0.0f )
		{
			m_eState = ADJ_ENABLE_P1LOADED;
		}
		else
		{
			//m_eState = ADJ_ENABLE_P2LOADED;
			m_eState = ADJ_IDLE;
		}
    }
    else //Disable Auto DJ
    {
        pushButtonAutoDJ->setText(tr("Enable Auto DJ"));
        qDebug() << "Auto DJ disabled";
        m_bAutoDJEnabled = false;
		m_bFadeNow = false;
        m_pCOPlayPos1->disconnect(this);
        m_pCOPlayPos2->disconnect(this);
    }
}

void DlgAutoDJ::player1PositionChanged(double value)
{
    // const float posThreshold = 0.95; //95% playback is when we crossfade and do stuff
    const float fadeDuration = 0.05; // 5% playback is crossfade duration

	// qDebug() << "player1PositionChanged(" << value << ")";

	if( !m_bAutoDJEnabled  )
	{
		//nothing to do 
		return;			
	}


	if ( m_eState == ADJ_ENABLE_P1LOADED )
	{	
		// Auto DJ Start
	    if (m_pCOPlay1->get() == 0.0f && m_pCOPlay2->get() == 0.0f) 
		{		
            m_pCOCrossfader->slotSet(-1.0f); //Move crossfader to the left!
            m_pCOPlay1->slotSet(1.0f); //Play the track in player 1
			removePlayingTrackFromQueue("[Channel1]");
        }
		else if( m_pCOPlay1->get() == 1.0f && m_pCOPlay2->get() == 0.0f)
		{
			//Load the next song from the queue.
            loadNextTrackFromQueue();			
			m_eState = ADJ_IDLE;
		}
		else 
		{
			m_eState = ADJ_IDLE;
		}
	}

	if( m_eState == ADJ_P2FADING )	
	{
		if(	 m_pCOPlay2->get() == 0.0f )
		{
			// End State		
			m_eState = ADJ_IDLE;
			loadNextTrackFromQueue();
		}
		return;
	}

	if( m_eState == ADJ_IDLE )	
	{
		if(m_bFadeNow)
		{
			m_posThreshold = value;
			m_pCORepeat1->slotSet(0.0f); // Repeat is disabled by FadeNow but disables auto Fade
			m_bFadeNow = false;	
		}
		else if( m_pCORepeat1->get() == 1.0f )
		{
			return;
		}
	}

    if (value >= m_posThreshold)
    {
		if( m_eState == ADJ_IDLE )
		{      
			if (m_pCOPlay2->get() == 0.0f)
		    {
		        m_pCOPlay2->slotSet(1.0f);
		    }
			removePlayingTrackFromQueue("[Channel2]");

			m_eState = ADJ_P1FADING;
		}

		float posFadeEnd = m_posThreshold + fadeDuration;
		if( posFadeEnd > 1.0f ) posFadeEnd = 1.0f;
		
        if (value >= posFadeEnd)
        {
			// Pre-EndState			
			m_pCOCrossfader->slotSet(1.0f); //Move crossfader to the right!
            
			m_pCOPlay1->slotSet(0.0f); //Stop the player
            
			m_posThreshold = 1.0f - fadeDuration; // back to default 
								
			// does not work always emediatly after stop
			// loadNextTrackFromQueue();
			// m_eState = ADJ_IDLE; // Fading ready
        }
		else
		{
			//Crossfade!
		    float crossfadeValue = -1.0f + 2*(value-m_posThreshold)/(posFadeEnd-m_posThreshold);
		    // crossfadeValue = -1.0f -> + 1.0f
			m_pCOCrossfader->slotSet(crossfadeValue); //Move crossfader to the right!
		}
    }
}

void DlgAutoDJ::player2PositionChanged(double value)
{
    // const float posThreshold = 0.95; //95% playback is when we crossfade and do stuff
    float fadeDuration = 0.05; // 5% playback is crossfade duration

	// qDebug() << "player2PositionChanged(" << value << ")";

	if( !m_bAutoDJEnabled )
	{
		//nothing to do 
		return;			
	}

	if( m_eState == ADJ_P1FADING )	
	{	
		if(	 m_pCOPlay1->get() == 0.0f )
		{
			// End State		
			m_eState = ADJ_IDLE;
			loadNextTrackFromQueue();
		}
		return;
	}

	if( m_eState == ADJ_IDLE )	
	{	
		if(m_bFadeNow)
		{
			m_posThreshold = value;
			m_pCORepeat2->slotSet(0.0f); // Repeat is disabled by FadeNow but disables auto Fade
			m_bFadeNow = false;	
		}
		else if( m_pCORepeat2->get() == 1.0f )
		{
			return;
		}
	}

    if (value >= m_posThreshold)
    {
		if( m_eState == ADJ_IDLE )
		{      
			if (m_pCOPlay1->get() == 0.0f)
		    {
		        m_pCOPlay1->slotSet(1.0f);
		    }
			removePlayingTrackFromQueue("[Channel1]");
			m_eState = ADJ_P2FADING;
		}

		float posFadeEnd = m_posThreshold + fadeDuration;
		if( posFadeEnd > 1.0f ) posFadeEnd = 1.0f;
		
        if (value >= posFadeEnd)
        {
			//Pre-End State			
			m_pCOCrossfader->slotSet(-1.0f); //Move crossfader to the left!
            
			m_pCOPlay2->slotSet(0.0f); //Stop the player

			m_posThreshold = 1.0f - fadeDuration; // back to default 						
			
			// does not work always emediatly after stop
			// loadNextTrackFromQueue();
			// m_eState = ADJ_IDLE; // Fading ready
        }
		else
		{
			//Crossfade!
		    float crossfadeValue = 1.0f - 2*(value-m_posThreshold)/(posFadeEnd-m_posThreshold);
		    // crossfadeValue = 1.0f -> + -1.0f
			m_pCOCrossfader->slotSet(crossfadeValue); //Move crossfader to the right!
		}
    }
}


bool DlgAutoDJ::loadNextTrackFromQueue()
{
    //Get the track at the top of the playlist...
    TrackPointer nextTrack = m_pAutoDJTableModel->getTrack(m_pAutoDJTableModel->index(0, 0));

    if (!nextTrack) //We ran out of tracks in the queue...
    {
        //Disable auto DJ and return...
        pushButtonAutoDJ->setChecked(false);
        return false;
    }

    //m_bNextTrackAlreadyLoaded = false;

    emit(loadTrack(nextTrack));

    return true;
}

bool DlgAutoDJ::removePlayingTrackFromQueue(QString group)
{
	TrackPointer nextTrack, loadedTrack;
	int nextId = 0, loadedId = 0;
	

	//Get the track at the top of the playlist...
    nextTrack = m_pAutoDJTableModel->getTrack(m_pAutoDJTableModel->index(0, 0));
	if ( nextTrack )
	{
		nextId = nextTrack->getId();
	}	

	//Get loaded track
	loadedTrack = PlayerInfo::Instance().getTrackInfo(group);
	if (loadedTrack )
	{	
		loadedId = loadedTrack->getId();
	}

	//When enable auto DJ and Topmost Song is already on second deck, nothing to do  
	//  BaseTrackPlayer::getLoadedTrack()   
	//  pTrack = PlayerInfo::Instance().getCurrentPlayingTrack(); 

	
	if ( loadedId != nextId )
	{
		// Do not remove when the user has loaded a track manualy
		return false;
    }

    // remove the top track 
	m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));	

    return true;
}

