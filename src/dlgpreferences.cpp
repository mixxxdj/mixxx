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
#include <QDesktopWidget>

#ifdef __VINYLCONTROL__
#include "dlgprefvinyl.h"
#else
#include "dlgprefnovinyl.h"
#endif

#ifdef __SHOUTCAST__
#include "dlgprefshoutcast.h"
#endif
#include "dlgprefbeats.h"

#ifdef __MODPLUG__
#include "dlgprefmodplug.h"
#endif

#include "dlgpreferences.h"
#include "dlgprefsound.h"
#include "controllers/dlgprefcontrollers.h"
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
                               ConfigObject<ConfigValue> * pConfig, Library *pLibrary) 
        : m_pageSizeHint(QSize(0, 0)),
          m_preferencesUpdated(ConfigKey("[Preferences]", "updated")) {
    setupUi(this);
#if QT_VERSION >= 0x040400 //setHeaderHidden is a qt4.4 addition so having it in the .ui file breaks the build on OpenBSD4.4 (FIXME: revisit this when OpenBSD4.5 comes out?)
    contentsTreeWidget->setHeaderHidden(true);
#endif

    setWindowTitle(tr("Preferences"));
    createIcons();

    while (pagesWidget->count() > 0) {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    // Construct widgets for use in tabs.

#ifdef __VINYLCONTROL__
    // It's important for this to be before the connect for wsound.
    // TODO(rryan) determine why/if this is still true
    m_wvinylcontrol = new DlgPrefVinyl(this, pVCManager, pConfig);
    addPageWidget(m_wvinylcontrol);
#else
    m_wnovinylcontrol = new DlgPrefNoVinyl(this, soundman, pConfig);
    addPageWidget(m_wnovinylcontrol);
#endif
    m_wsound = new DlgPrefSound(this, soundman, pPlayerManager, pConfig);
    addPageWidget(m_wsound);
    m_wplaylist = new DlgPrefPlaylist(this, pConfig, pLibrary);
    addPageWidget(m_wplaylist);
    m_wcontrols = new DlgPrefControls(this, mixxx, pSkinLoader, pPlayerManager, pConfig);
    addPageWidget(m_wcontrols);
    m_weq = new DlgPrefEQ(this, pConfig);
    addPageWidget(m_weq);
    m_wcrossfader = new DlgPrefCrossfader(this, pConfig);
    addPageWidget(m_wcrossfader);

    m_wbeats = new DlgPrefBeats(this, pConfig);
    addPageWidget (m_wbeats);
    m_wreplaygain = new DlgPrefReplayGain(this, pConfig);
    addPageWidget(m_wreplaygain);
    m_wrecord = new DlgPrefRecord(this, pConfig);
    addPageWidget(m_wrecord);
#ifdef __SHOUTCAST__
    m_wshoutcast = new DlgPrefShoutcast(this, pConfig);
    addPageWidget(m_wshoutcast);
#endif
#ifdef __MODPLUG__
    m_wmodplug = new DlgPrefModplug(this, pConfig);
    addPageWidget(m_wmodplug);
#endif
    m_wcontrollers = new DlgPrefControllers(this, pConfig, controllers,
                                            m_pControllerTreeItem);
    addPageWidget(m_wcontrollers);

    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // If we don't call this explicitly, then we default to showing the sound
    // hardware page but the tree item is not selected.
    showSoundHardwarePage();
}

DlgPreferences::~DlgPreferences() {
    // Need to explicitly delete rather than relying on child auto-deletion
    // because otherwise the QStackedWidget will delete the controller
    // preference pages (and DlgPrefControllers dynamically generates and
    // deletes them).
    delete m_wcontrollers;
}

void DlgPreferences::createIcons() {
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


    m_pAnalysersButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pAnalysersButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
    m_pAnalysersButton->setText(0, tr("Beat Detection"));
    m_pAnalysersButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pAnalysersButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
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

#ifdef __MODPLUG__
    m_pModplugButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pModplugButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_modplug.png"));
    m_pModplugButton->setText(0, tr("Modplug Decoder"));
    m_pModplugButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pModplugButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

    connect(contentsTreeWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(changePage(QTreeWidgetItem*, QTreeWidgetItem*)));
}

void DlgPreferences::changePage(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    if (!current)
        current = previous;

    if (current == m_pSoundButton) {
    	m_wsound->slotUpdate();
      switchToPage(m_wsound);
    } else if (current == m_pPlaylistButton) {
        switchToPage(m_wplaylist);
    } else if (current == m_pControlsButton) {
        switchToPage(m_wcontrols);
    } else if (current == m_pEqButton) {
        switchToPage(m_weq);
    } else if (current == m_pCrossfaderButton) {
        switchToPage(m_wcrossfader);
    } else if (current == m_pRecordingButton) {
        switchToPage(m_wrecord);
    } else if (current == m_pAnalysersButton ) {
        switchToPage(m_wbeats);
    } else if (current == m_pReplayGainButton) {
        switchToPage(m_wreplaygain);
#ifdef __VINYLCONTROL__
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_wvinylcontrol);
#else
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_wnovinylcontrol);
#endif
#ifdef __SHOUTCAST__
    } else if (current == m_pShoutcastButton) {
        switchToPage(m_wshoutcast);
#endif
#ifdef __MODPLUG__
    } else if (current == m_pModplugButton) {
        switchToPage(m_wmodplug);
#endif
    } else if (m_wcontrollers->handleTreeItemClick(current)) {
        // Do nothing. m_wcontrolles handled this click.
    }
}

void DlgPreferences::showSoundHardwarePage() {
    switchToPage(m_wsound);
    contentsTreeWidget->setCurrentItem(m_pSoundButton);
}

bool DlgPreferences::eventFilter(QObject* o, QEvent* e) {
    // Send a close signal if dialog is closing
    if (e->type() == QEvent::Hide) {
        onHide();
    }

    if (e->type() == QEvent::Show) {
        onShow();
    }

    // Standard event processing
    return QWidget::eventFilter(o,e);
}

void DlgPreferences::onHide() {
    // Notify children that we are about to hide.
    emit(closeDlg());

    // Notify other parts of Mixxx that the preferences window just saved and so
    // preferences are likely changed.
    m_preferencesUpdated.set(1);
}

void DlgPreferences::onShow() {
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

    // Notify children that we are about to show.
    emit(showDlg());
}

void DlgPreferences::addPageWidget(DlgPreferencePage* pWidget) {
    connect(this, SIGNAL(showDlg()),
            pWidget, SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()),
            pWidget, SLOT(slotHide()));
    connect(this, SIGNAL(showDlg()),
            pWidget, SLOT(slotUpdate()));
    connect(buttonBox, SIGNAL(accepted()),
            pWidget, SLOT(slotApply()));

    QScrollArea* sa = new QScrollArea(pagesWidget);
    sa->setWidgetResizable(true);

    sa->setWidget(pWidget);
    pagesWidget->addWidget(sa);

    int iframe = 2 * sa->frameWidth();
    m_pageSizeHint = m_pageSizeHint.expandedTo(
            pWidget->sizeHint()+QSize(iframe, iframe));

}

void DlgPreferences::removePageWidget(DlgPreferencePage* pWidget) {
    pagesWidget->removeWidget(pWidget->parentWidget()->parentWidget());
}

void DlgPreferences::switchToPage(DlgPreferencePage* pWidget) {
    pagesWidget->setCurrentWidget(pWidget->parentWidget()->parentWidget());
}
