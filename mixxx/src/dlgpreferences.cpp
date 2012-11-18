/***************************************************************************
                         dlgpreferences.cpp  -  description
                         ------------------
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

#include <QTabWidget>
#include <QTabBar>
#include <QDialog>
#include <QtGui>
#include <QEvent>
#include <QScrollArea>

#ifdef __VINYLCONTROL__
#include "dlgprefvinyl.h"
#else
#include "dlgprefnovinyl.h"
#endif

#ifdef __SHOUTCAST__
#include "dlgprefshoutcast.h"
#endif
#ifdef __VAMP__
    #include "dlgprefbeats.h"
#else
    #include "dlgprefbpm.h"
#endif

#include "dlgpreferences.h"
#include "dlgprefsound.h"
#include "controllers/dlgprefmappablecontroller.h"
#include "controllers/dlgprefnocontrollers.h"
#include "dlgprefplaylist.h"
#include "dlgprefcontrols.h"
#include "dlgprefeq.h"
#include "dlgprefcrossfader.h"
#include "dlgprefrecord.h"
#include "dlgprefreplaygain.h"
#include "mixxx.h"
#include "controllers/controllermanager.h"
#include "skin/skinloader.h"
#include "library/library.h"

DlgPreferences::DlgPreferences(MixxxApp * mixxx, SkinLoader* pSkinLoader,
                               SoundManager * soundman, PlayerManager* pPlayerManager,
                               ControllerManager * controllers, VinylControlManager *pVCManager,
                               ConfigObject<ConfigValue> * _config) {
    m_pControllerManager = controllers;

    setupUi(this);
#if QT_VERSION >= 0x040400 //setHeaderHidden is a qt4.4 addition so having it in the .ui file breaks the build on OpenBSD4.4 (FIXME: revisit this when OpenBSD4.5 comes out?)
    contentsTreeWidget->setHeaderHidden(true);
#endif

    setWindowTitle(tr("Preferences"));
    config = _config;

    createIcons();
    //contentsTreeWidget->setCurrentRow(0);

    while (pagesWidget->count() > 0)
    {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }
    m_pageSizeHint = QSize(0,0);

    // Construct widgets for use in tabs
    m_wsound = new DlgPrefSound(this, soundman, pPlayerManager, config);
    addPageWidget(m_wsound);
    m_wplaylist = new DlgPrefPlaylist(this, config);
    addPageWidget(m_wplaylist);
    m_wcontrols = new DlgPrefControls(this, mixxx, pSkinLoader, pPlayerManager, config);
    addPageWidget(m_wcontrols);
    m_weq = new DlgPrefEQ(this, config);
    addPageWidget(m_weq);
    m_wcrossfader = new DlgPrefCrossfader(this, config);
    addPageWidget(m_wcrossfader);

#ifdef __VAMP__
    m_wbeats = new DlgPrefBeats(this, config);
    addPageWidget (m_wbeats);
#else
    m_wbpm = new DlgPrefBpm(this, config);
    addPageWidget(m_wbpm);
#endif
    m_wreplaygain = new DlgPrefReplayGain(this, config);
    addPageWidget(m_wreplaygain);
    m_wrecord = new DlgPrefRecord(this, config);
    addPageWidget(m_wrecord);
#ifdef __VINYLCONTROL__
    m_wvinylcontrol = new DlgPrefVinyl(this, pVCManager, config);
    addPageWidget(m_wvinylcontrol);
#else
    m_wnovinylcontrol = new DlgPrefNoVinyl(this, soundman, config);
    addPageWidget(m_wnovinylcontrol);
#endif
#ifdef __SHOUTCAST__
    m_wshoutcast = new DlgPrefShoutcast(this, config);
    addPageWidget(m_wshoutcast);
#endif
    m_wNoControllers = new DlgPrefNoControllers(this, config);
    addPageWidget(m_wNoControllers);
    setupControllerWidgets();

    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // Connections
    connect(this, SIGNAL(showDlg()), this,      SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()), this,      SLOT(slotHide()));

    connect(m_pControllerManager, SIGNAL(devicesChanged()), this, SLOT(rescanControllers()));

    connect(this, SIGNAL(showDlg()), m_wcontrols, SLOT(onShow()));
    connect(this, SIGNAL(closeDlg()), m_wcontrols, SLOT(onHide()));

    connect(this, SIGNAL(showDlg()), m_wsound,     SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), m_wplaylist,  SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), m_wcontrols,  SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), m_weq,        SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), m_wcrossfader, SLOT(slotUpdate()));

#ifdef __VAMP__
    connect(this, SIGNAL(showDlg()),
            m_wbeats, SLOT(slotUpdate()));
#else
    connect(this, SIGNAL(showDlg()),
            m_wbpm, SLOT(slotUpdate()));
#endif

    connect(this, SIGNAL(showDlg()), m_wreplaygain,SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), m_wrecord,    SLOT(slotUpdate()));

#ifdef __VINYLCONTROL__
    connect(this, SIGNAL(showDlg()), m_wvinylcontrol, SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()), m_wvinylcontrol,SLOT(slotClose()));
    connect(this, SIGNAL(showDlg()), m_wvinylcontrol,    SLOT(slotUpdate()));
    //connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
#endif
#ifdef __SHOUTCAST__
    connect(this, SIGNAL(showDlg()), m_wshoutcast,SLOT(slotUpdate()));
#endif

#ifdef __VINYLCONTROL__
    connect(buttonBox, SIGNAL(accepted()), m_wvinylcontrol,    SLOT(slotApply())); //It's important for this to be before the
                                                                                 //connect for wsound...
#endif
    connect(buttonBox, SIGNAL(accepted()), m_wsound,    SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), m_wplaylist, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), m_wcontrols, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), m_weq,       SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), m_wcrossfader,SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), this,      SLOT(slotApply()));

#ifdef __VAMP__
    connect(buttonBox, SIGNAL(accepted()), m_wbeats,      SLOT(slotApply()));
#else
    connect(buttonBox, SIGNAL(accepted()), m_wbpm,      SLOT(slotApply()));
#endif
    connect(buttonBox, SIGNAL(accepted()), m_wreplaygain,SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), m_wrecord,   SLOT(slotApply()));
#ifdef __SHOUTCAST__
    connect(buttonBox, SIGNAL(accepted()), m_wshoutcast,SLOT(slotApply()));
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
    destroyControllerWidgets();
}

void DlgPreferences::createIcons()
{
    m_pSoundButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pSoundButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_soundhardware.png"));
    m_pSoundButton->setText(0, tr("Sound Hardware"));
    m_pSoundButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pSoundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pControllerTreeItem = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControllerTreeItem->setIcon(0, QIcon(":/images/preferences/ic_preferences_controllers.png"));
    m_pControllerTreeItem->setText(0, tr("Controllers"));
    m_pControllerTreeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pControllerTreeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pPlaylistButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pPlaylistButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_library.png"));
    m_pPlaylistButton->setText(0, tr("Library"));
    m_pPlaylistButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pPlaylistButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pControlsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControlsButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_interface.png"));
    m_pControlsButton->setText(0, tr("Interface"));
    m_pControlsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pControlsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEqButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEqButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_equalizers.png"));
    m_pEqButton->setText(0, tr("Equalizers"));
    m_pEqButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEqButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pCrossfaderButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pCrossfaderButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_crossfader.png"));
    m_pCrossfaderButton->setText(0, tr("Crossfader"));
    m_pCrossfaderButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pCrossfaderButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pRecordingButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pRecordingButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_recording.png"));
    m_pRecordingButton->setText(0, tr("Recording"));
    m_pRecordingButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pRecordingButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


#ifdef __VAMP__
    m_pAnalysersButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pAnalysersButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
    m_pAnalysersButton->setText(0, tr("Beat Detection"));
    m_pAnalysersButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pAnalysersButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#else
    m_pBPMdetectButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pBPMdetectButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
    m_pBPMdetectButton->setText(0, tr("BPM Detection"));
    m_pBPMdetectButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pBPMdetectButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif
    m_pReplayGainButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pReplayGainButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_replaygain.png"));
    m_pReplayGainButton->setText(0, tr("Normalization"));
    m_pReplayGainButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pReplayGainButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __VINYLCONTROL__
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    //QT screws up my nice vinyl svg for some reason, so we'll use a PNG version
    //instead...
    m_pVinylControlButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_vinyl.png"));
    m_pVinylControlButton->setText(0, tr("Vinyl Control"));
    m_pVinylControlButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pVinylControlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#else
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    //QT screws up my nice vinyl svg for some reason, so we'll use a PNG version
    //instead...
    m_pVinylControlButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_vinyl.png"));
    m_pVinylControlButton->setText(0, tr("Vinyl Control"));
    m_pVinylControlButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pVinylControlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

#ifdef __SHOUTCAST__
    m_pShoutcastButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pShoutcastButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_broadcast.png"));
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

    if (current == m_pSoundButton) {
    	m_wsound->slotUpdate();
    	pagesWidget->setCurrentWidget(m_wsound->parentWidget()->parentWidget());
    } else if (current == m_pPlaylistButton) {
    	pagesWidget->setCurrentWidget(m_wplaylist->parentWidget()->parentWidget());
    } else if (current == m_pControlsButton) {
    	pagesWidget->setCurrentWidget(m_wcontrols->parentWidget()->parentWidget());
    } else if (current == m_pEqButton) {
    	pagesWidget->setCurrentWidget(m_weq->parentWidget()->parentWidget());
    } else if (current == m_pCrossfaderButton) {
    	pagesWidget->setCurrentWidget(m_wcrossfader->parentWidget()->parentWidget());
    } else if (current == m_pRecordingButton) {
    	pagesWidget->setCurrentWidget(m_wrecord->parentWidget()->parentWidget());

#ifdef __VAMP__
    } else if (current == m_pAnalysersButton ) {
        pagesWidget->setCurrentWidget(m_wbeats->parentWidget()->parentWidget());
#else
    } else if (current == m_pBPMdetectButton) {
        pagesWidget->setCurrentWidget(m_wbpm->parentWidget()->parentWidget());
#endif
    } else if (current == m_pReplayGainButton) {
    	pagesWidget->setCurrentWidget(m_wreplaygain->parentWidget()->parentWidget());

#ifdef __VINYLCONTROL__
    } else if (current == m_pVinylControlButton) {
    	pagesWidget->setCurrentWidget(m_wvinylcontrol->parentWidget()->parentWidget());
#else
    } else if (current == m_pVinylControlButton) {
           pagesWidget->setCurrentWidget(m_wnovinylcontrol->parentWidget()->parentWidget());
#endif
#ifdef __SHOUTCAST__
    } else if (current == m_pShoutcastButton) {
           pagesWidget->setCurrentWidget(m_wshoutcast->parentWidget()->parentWidget());
#endif
    //Handle selection of controller items
    } else if (m_controllerWindowLinks.indexOf(current) >= 0) {
           int index = m_controllerWindowLinks.indexOf(current);
           pagesWidget->setCurrentWidget(m_controllerWindows.value(index)->parentWidget()->parentWidget());
           //Manually fire this slot since it doesn't work right...
           m_controllerWindows.value(index)->slotUpdate();
    }

    else if (current == m_pControllerTreeItem) {
        //If the root "Controllers" item is clicked, select the first Controller instead.
        //If there is no first controller, display a page that says so (just so we don't not change the page)
        if (m_controllerWindows.count() > 0)
        {
            //Expand the Controller subtree
            contentsTreeWidget->setItemExpanded(m_pControllerTreeItem, true);
        }
        else
        {
            pagesWidget->setCurrentWidget(m_wNoControllers->parentWidget()->parentWidget());
        }
    }
}

void DlgPreferences::showSoundHardwarePage()
{
    pagesWidget->setCurrentWidget(m_wsound->parentWidget()->parentWidget());
    contentsTreeWidget->setCurrentItem(m_pSoundButton);
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

void DlgPreferences::slotHide() {
}


void DlgPreferences::slotShow() {
    QSize optimumSize;
    QSize deltaSize;
    QSize pagesSize;
    QSize saSize;

    adjustSize();

    optimumSize = qApp->desktop()->availableGeometry(this).size();

    if (frameSize() == size()) {
        // This code is reached in Gnome 2.3
        qDebug() << "guess the size of the window decoration";
        optimumSize -= QSize(2,30);
    } else {
        optimumSize -= (frameSize() - size());
    }

    QSize staticSize = size() - pagesWidget->size();
    optimumSize = optimumSize.boundedTo(staticSize + m_pageSizeHint);

    QRect optimumRect = geometry();
    optimumRect.setSize(optimumSize);
    setGeometry(optimumRect);
}

void DlgPreferences::rescanControllers()
{
    destroyControllerWidgets();
    setupControllerWidgets();
}

void DlgPreferences::destroyControllerWidgets()
{
    //XXX this, and the corresponding code over in onShow(), is pretty bad and messy; it should be wrapped up in a class so that constructors and destructors can handle this setup/teardown

    m_controllerWindowLinks.clear();

    while (!m_controllerWindows.isEmpty())
    {
        DlgPrefController* controllerDlg = m_controllerWindows.takeLast();
        pagesWidget->removeWidget(controllerDlg);
        delete controllerDlg;
    }

    while(m_pControllerTreeItem->childCount() > 0)
    {
        QTreeWidgetItem* controllerWindowLink = m_pControllerTreeItem->takeChild(0);
        //qDebug() << " Q|T|r\e\eWidgetItem point is " << controllerWindowLink;
        m_pControllerTreeItem->removeChild(controllerWindowLink);
        delete controllerWindowLink;
    }
}

void DlgPreferences::setupControllerWidgets()
{
    //For each controller, create a dialog and put a little link to it in the treepane on the left
    QList<Controller*> controllerList = m_pControllerManager->getControllerList(false, true);
    qSort(
        controllerList.begin(),
        controllerList.end(),
        controllerCompare
    );
    QListIterator<Controller*> ctrlr(controllerList);
    while (ctrlr.hasNext())
    {
        Controller* currentDevice = ctrlr.next();
        QString curDeviceName = currentDevice->getName();
        //qDebug() << "curDeviceName: " << curDeviceName;
        if (currentDevice->isMappable()) {
            DlgPrefMappableController* controllerDlg =
                new DlgPrefMappableController(this, currentDevice,
                                              m_pControllerManager, config);
            connect(controllerDlg, SIGNAL(mappingStarted()),
                    this, SLOT(hide()));
            connect(controllerDlg, SIGNAL(mappingEnded()),
                    this, SLOT(show()));
            m_controllerWindows.append(controllerDlg);
            addPageWidget(controllerDlg);
            connect(this, SIGNAL(showDlg()), controllerDlg, SLOT(enumeratePresets()));
            connect(this, SIGNAL(showDlg()), controllerDlg, SLOT(slotUpdate()));
            connect(buttonBox, SIGNAL(accepted()), controllerDlg, SLOT(slotApply()));
            connect(controllerDlg, SIGNAL(deviceStateChanged(DlgPrefController*,bool)), this, SLOT(slotHighlightDevice(DlgPrefController*,bool)));
        } else {
            DlgPrefController* controllerDlg =
                new DlgPrefController(this, currentDevice, m_pControllerManager,
                                      config);
            m_controllerWindows.append(controllerDlg);
            addPageWidget(controllerDlg);
            connect(this, SIGNAL(showDlg()), controllerDlg, SLOT(enumeratePresets()));
            connect(this, SIGNAL(showDlg()), controllerDlg, SLOT(slotUpdate()));
            connect(buttonBox, SIGNAL(accepted()), controllerDlg, SLOT(slotApply()));
            connect(controllerDlg, SIGNAL(deviceStateChanged(DlgPrefController*,bool)),
                    this, SLOT(slotHighlightDevice(DlgPrefController*,bool)));
        }

        QTreeWidgetItem * controllerWindowLink = new QTreeWidgetItem(QTreeWidgetItem::Type);
        //qDebug() << curDeviceName << " QTreeWidgetItem point is " << controllerWindowLink;
        controllerWindowLink->setIcon(0, QIcon(":/images/preferences/ic_preferences_controllers.png"));
        controllerWindowLink->setText(0, curDeviceName);
        controllerWindowLink->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        controllerWindowLink->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_pControllerTreeItem->addChild(controllerWindowLink);
        m_controllerWindowLinks.append(controllerWindowLink);

        // Set the font correctly
        QFont temp = controllerWindowLink->font(0);
        if (currentDevice->isOpen()) temp.setBold(true);
        else temp.setBold(false);
        controllerWindowLink->setFont(0,temp);
    }
}

void DlgPreferences::slotApply()
{
    m_pControllerManager->savePresets();
}

void DlgPreferences::slotHighlightDevice(DlgPrefController* dialog, bool enabled)
{
    QTreeWidgetItem * controllerWindowLink = m_controllerWindowLinks.at(m_controllerWindows.indexOf(dialog));
    QFont temp = controllerWindowLink->font(0);
    if (enabled) temp.setBold(true);
    else temp.setBold(false);
    controllerWindowLink->setFont(0,temp);
}

int DlgPreferences::addPageWidget(QWidget* w) {
    int iret;

    QScrollArea* sa = new QScrollArea(pagesWidget);
    sa->setWidgetResizable(true);

    sa->setWidget(w);
    iret = pagesWidget->addWidget(sa);

    int iframe = 2 * sa->frameWidth();
    m_pageSizeHint = m_pageSizeHint.expandedTo(w->sizeHint()+QSize(iframe, iframe));

    return iret;
}
