/***************************************************************************
                      dlgpreferences.cpp  -  description
                         -------------------
   begin                : Sun Jun 30 2002
   copyright            : (C) 2002 by Tue & Ken Haste Andersen
   email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifdef __VINYLCONTROL__
#include "dlgprefvinyl.h"
#endif

#ifdef __SHOUTCAST__
#include "dlgprefshoutcast.h"
#endif

#include "dlgprefbpm.h"
#include "dlgpreferences.h"
#include "dlgprefsound.h"
#include "dlgprefmidibindings.h"
#include "dlgprefplaylist.h"
#include "dlgprefnomidi.h"
#include "dlgprefcontrols.h"
#include "dlgprefeq.h"
#include "dlgprefcrossfader.h"
#include "dlgprefrecord.h"
#include "mixxx.h"
#include "midiobject.h"
#include <QTabWidget>

#include <QTabBar>
#include <QDialog>
#include <QtGui>
#include <QEvent>

DlgPreferences::DlgPreferences(MixxxApp * mixxx, MixxxView * view,
                               SoundManager * soundman, 
                               MidiObject * midi, ConfigObject<ConfigValue> * _config) :  QDialog(), Ui::DlgPreferencesDlg()
{
    m_pMixxx = mixxx;
    m_pMidiObject = midi;

    setupUi(this);
#if QT_VERSION >= 0x040400 //setHeaderHidden is a qt4.4 addition so having it in the .ui file breaks the build on OpenBSD4.4 (FIXME: revisit this when OpenBSD4.5 comes out?)
    contentsTreeWidget->setHeaderHidden(true);
#endif

    setWindowTitle(tr("Preferences"));
    config = _config;

    createIcons();
    //contentsTreeWidget->setCurrentRow(0);

    // Construct widgets for use in tabs
    wsound = new DlgPrefSound(this, soundman, config);
    wplaylist = new DlgPrefPlaylist(this, config);
    wcontrols = new DlgPrefControls(this, view, mixxx, config);
    weq = new DlgPrefEQ(this, config);
    wcrossfader = new DlgPrefCrossfader(this, config);
    wbpm = new DlgPrefBpm(this, config);
    wrecord = new DlgPrefRecord(this, config);
#ifdef __VINYLCONTROL__
    wvinylcontrol = new DlgPrefVinyl(this, soundman, config);
#endif
#ifdef __SHOUTCAST__
    wshoutcast = new DlgPrefShoutcast(this, config);
#endif
    wNoMidi = new DlgPrefNoMidi(this, config);

    while (pagesWidget->count() > 0)
    {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    pagesWidget->addWidget(wsound);
    pagesWidget->addWidget(wplaylist);
    pagesWidget->addWidget(wcontrols);
    pagesWidget->addWidget(weq);
    pagesWidget->addWidget(wcrossfader);
    pagesWidget->addWidget(wrecord);
    pagesWidget->addWidget(wbpm);
#ifdef __VINYLCONTROL__
    pagesWidget->addWidget(wvinylcontrol);
#endif
#ifdef __SHOUTCAST__
    pagesWidget->addWidget(wshoutcast);
#endif

    pagesWidget->addWidget(wNoMidi);
    setupMidiWidgets();


    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // Connections
    connect(this, SIGNAL(showDlg()), this,      SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()), this,      SLOT(slotHide()));
    connect(m_pMidiObject, SIGNAL(devicesChanged()), this, SLOT(rescanMidi()));

    connect(this, SIGNAL(showDlg()), wsound,    SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wplaylist, SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wcontrols, SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), weq,       SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()),wcrossfader,SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wbpm,      SLOT(slotUpdate()));

    connect(this, SIGNAL(showDlg()), wrecord,    SLOT(slotUpdate()));
#ifdef __VINYLCONTROL__
    connect(this, SIGNAL(showDlg()), wvinylcontrol, SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()), wvinylcontrol,SLOT(slotClose()));
    connect(this, SIGNAL(showDlg()), wvinylcontrol,    SLOT(slotUpdate()));
    //connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
    connect(wsound, SIGNAL(apiUpdated()), wvinylcontrol,    SLOT(slotUpdate())); //Update the vinyl control
#endif
#ifdef __SHOUTCAST__
    connect(this, SIGNAL(showDlg()), wshoutcast,SLOT(slotUpdate()));
#endif

#ifdef __VINYLCONTROL__
    connect(buttonBox, SIGNAL(accepted()), wvinylcontrol,    SLOT(slotApply())); //It's important for this to be before the
                                                                                 //connect for wsound...
#endif
    connect(buttonBox, SIGNAL(accepted()), wsound,    SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wplaylist, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wcontrols, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), weq,       SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()),wcrossfader,SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), this,      SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wbpm,    SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wrecord,    SLOT(slotApply()));
#ifdef __SHOUTCAST__
    connect(buttonBox, SIGNAL(accepted()), wshoutcast,SLOT(slotApply()));
#endif

    //Update the library when you change the options
    /*if (m_pTrack && wplaylist)
    {
        connect(wplaylist, SIGNAL(apply()), m_pTrack, SLOT(slotScanLibrary()));
    }*/
    //FIXME: Disabled due to library reworking
}

DlgPreferences::~DlgPreferences()
{
  destroyMidiWidgets();
}

void DlgPreferences::createIcons()
{	
    m_pSoundButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pSoundButton->setIcon(0, QIcon(":/images/preferences/soundhardware.png"));
    m_pSoundButton->setText(0, tr("Sound Hardware"));
    m_pSoundButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pSoundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
/*
    QTreeWidgetItem * midiButton = new QTreeWidgetItem(contentsTreeWidget);
    midiButton->setIcon(0, QIcon(":/images/preferences/controllers.png"));
    midiButton->setText(tr("Input Controllers"));
    midiButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    midiButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
*/
    m_pMIDITreeItem = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pMIDITreeItem->setIcon(0, QIcon(":/images/preferences/controllers.png"));
    m_pMIDITreeItem->setText(0, tr("MIDI Controllers"));
    m_pMIDITreeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pMIDITreeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

/*
    QTreeWidgetItem * midiBindingsButton = new QTreeWidgetItem(m_pMIDITreeItem, QTreeWidgetItem::Type);
    midiBindingsButton->setIcon(0, QIcon(":/images/preferences/controllers.png"));
    midiBindingsButton->setText(0, tr("MIDI Bindings"));
    midiBindingsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    midiBindingsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
*/

    m_pPlaylistButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pPlaylistButton->setIcon(0, QIcon(":/images/preferences/library.png"));
    m_pPlaylistButton->setText(0, tr("Library and Playlists"));
    m_pPlaylistButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pPlaylistButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

   	m_pControlsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControlsButton->setIcon(0, QIcon(":/images/preferences/interface.png"));
    m_pControlsButton->setText(0, tr("Interface"));
    m_pControlsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pControlsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEqButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEqButton->setIcon(0, QIcon(":/images/preferences/generic.png"));
    m_pEqButton->setText(0, tr("Equalizers"));
    m_pEqButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEqButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pCrossfaderButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pCrossfaderButton->setIcon(0, QIcon(":/images/preferences/generic.png"));
    m_pCrossfaderButton->setText(0, tr("Crossfader"));
    m_pCrossfaderButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pCrossfaderButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pRecordingButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pRecordingButton->setIcon(0, QIcon(":/images/preferences/recording.png"));
    m_pRecordingButton->setText(0, tr("Recording"));
    m_pRecordingButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pRecordingButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pBPMdetectButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pBPMdetectButton->setIcon(0, QIcon(":/images/preferences/bpmdetect.png"));
    m_pBPMdetectButton->setText(0, tr("BPM Detection"));
    m_pBPMdetectButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pBPMdetectButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __VINYLCONTROL__
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    //QT screws up my nice vinyl svg for some reason, so we'll use a PNG version
    //instead...
    m_pVinylControlButton->setIcon(0, QIcon(":/images/preferences/vinyl.png"));
    m_pVinylControlButton->setText(0, tr("Vinyl Control"));
    m_pVinylControlButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pVinylControlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

#ifdef __SHOUTCAST__
    m_pShoutcastButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pShoutcastButton->setIcon(0, QIcon(":/images/preferences/broadcast.png"));
    m_pShoutcastButton->setText(0, tr("Live Broadcasting"));
    m_pShoutcastButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pShoutcastButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif
    connect(contentsTreeWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changePage(QTreeWidgetItem *, QTreeWidgetItem*)));
}

void DlgPreferences::changePage(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
    if (!current)
        current = previous;

	if (current == m_pSoundButton)
   		pagesWidget->setCurrentWidget(wsound);
   	else if (current == m_pPlaylistButton)
   		pagesWidget->setCurrentWidget(wplaylist);
   	else if (current == m_pControlsButton)
   		pagesWidget->setCurrentWidget(wcontrols);
   	else if (current == m_pEqButton)
   		pagesWidget->setCurrentWidget(weq);
   	else if (current == m_pCrossfaderButton)
   		pagesWidget->setCurrentWidget(wcrossfader);
   	else if (current == m_pRecordingButton)
   		pagesWidget->setCurrentWidget(wrecord);
   	else if (current == m_pBPMdetectButton)
   		pagesWidget->setCurrentWidget(wbpm);
#ifdef __VINYLCONTROL__
   	else if (current == m_pVinylControlButton)
   		pagesWidget->setCurrentWidget(wvinylcontrol);
#endif
#ifdef __SHOUTCAST__
   	else if (current == m_pShoutcastButton)
   		pagesWidget->setCurrentWidget(wshoutcast);
#endif

   	//Handle selection of midi device items
   	else if (m_midiBindingsButtons.indexOf(current) >= 0)
   	{
   		int index = m_midiBindingsButtons.indexOf(current);
   		pagesWidget->setCurrentWidget(wmidiBindingsForDevice.value(index));
   		//Manually fire this slot since it doesn't work right...
   		wmidiBindingsForDevice.value(index)->slotUpdate();
   	}
   	
   	//If the root "MIDI Device" item is clicked, select the first MIDI device instead.
   	//If there is no first MIDI device, display a page that says so (just so we don't not change the page)
   	else if (current == m_pMIDITreeItem)
   	{
   		if (wmidiBindingsForDevice.count() > 0)
   		{
   			//Expand the MIDI subtree
   			contentsTreeWidget->setItemExpanded(m_pMIDITreeItem, true);
   			
   			/*
   			* FIXME: None of the following works right, for some reason. - Albert Feb 9/09 
   			*/
   			
   			//Select the first MIDI device
   			//contentsTreeWidget->setItemSelected(m_pMIDITreeItem, false);
   			/*
   			foreach(QTreeWidgetItem* item, contentsTreeWidget->selectedItems())
   			{
   				contentsTreeWidget->setItemSelected(item, false);
   			}*/
   			//contentsTreeWidget->setItemSelected(m_midiBindingsButtons.value(0), true);
   			
   		}
   		else
   		{
   			pagesWidget->setCurrentWidget(wNoMidi);
   		}
   	}
   
}

void DlgPreferences::showVinylControlPage()
{
#ifdef __VINYLCONTROL__
    pagesWidget->setCurrentWidget(wvinylcontrol);
	contentsTreeWidget->setCurrentItem(m_pVinylControlButton);
#endif
}

bool DlgPreferences::eventFilter(QObject * o, QEvent * e)
{
    // Send a close signal if dialog is closing
    if (e->type() == QEvent::Hide)
        emit(closeDlg());

    if (e->type() == QEvent::Show)
        emit(showDlg());

    // Standard event processing
    return QWidget::eventFilter(o,e);
}

void DlgPreferences::slotHide()
{
}


void DlgPreferences::slotShow()
{
  //m_pMixxx->releaseKeyboard();
}

void DlgPreferences::rescanMidi()
{
  destroyMidiWidgets();
  setupMidiWidgets();
}

void DlgPreferences::destroyMidiWidgets()
{
//XXX this, and the corresponding code over in onShow(), is pretty bad and messy; it should be wrapped up in a class so that constructors and destructors can handle this setup/teardown

  m_midiBindingsButtons.clear();
  
  while (!wmidiBindingsForDevice.isEmpty())
    {
      DlgPrefMidiBindings* midiDlg = wmidiBindingsForDevice.takeLast();
      pagesWidget->removeWidget(midiDlg);
      delete midiDlg;
    }

  while(m_pMIDITreeItem->childCount() > 0) 
    {
      QTreeWidgetItem* midiBindingsButton = m_pMIDITreeItem->takeChild(0);
      //qDebug() << " Q|T|r\e\eWidgetItem point is " << midiBindingsButton;
      m_pMIDITreeItem->removeChild(midiBindingsButton);
      delete midiBindingsButton;
    }
}

void DlgPreferences::setupMidiWidgets()
{

	//TODO: For each MIDI device, create a MIDI dialog and put a little link to it in the treepane on the left
	QList<QString>* deviceList = m_pMidiObject->getDeviceList();
	QListIterator<QString> it(*deviceList);
	
	while (it.hasNext())
	  {
	    QString curDeviceName = QString(it.next()); //make a copy of it.next() so that the original list getting freed doesn't kill us
	    //qDebug() << "curDeviceName: " << curDeviceName;
	    DlgPrefMidiBindings* midiDlg = new DlgPrefMidiBindings(this, *m_pMidiObject, curDeviceName, config);
	    wmidiBindingsForDevice.append(midiDlg);
	    pagesWidget->addWidget(midiDlg);
	    connect(this, SIGNAL(showDlg()), midiDlg, SLOT(slotUpdate()));
	    connect(buttonBox, SIGNAL(accepted()), midiDlg, SLOT(slotApply()));
	    
	    QTreeWidgetItem * midiBindingsButton = new QTreeWidgetItem(QTreeWidgetItem::Type);
	    //qDebug() << curDeviceName << " QTreeWidgetItem point is " << midiBindingsButton;
	    midiBindingsButton->setIcon(0, QIcon(":/images/preferences/controllers.png"));
	    midiBindingsButton->setText(0, curDeviceName);
	    midiBindingsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
	    midiBindingsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	    m_pMIDITreeItem->addChild(midiBindingsButton);
	    m_midiBindingsButtons.append(midiBindingsButton);	    
	  }
}

void DlgPreferences::slotApply()
{
//    m_pMixxx->grabKeyboard();
}

