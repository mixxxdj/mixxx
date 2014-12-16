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

#include <QDesktopWidget>
#include <QDialog>
#include <QEvent>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>

#ifdef __VINYLCONTROL__
#include "dlgprefvinyl.h"
#else
#include "dlgprefnovinyl.h"
#endif

#ifdef __SHOUTCAST__
#include "dlgprefshoutcast.h"
#endif

#include "dlgprefbeats.h"
#include "dlgprefkey.h"

#ifdef __MODPLUG__
#include "dlgprefmodplug.h"
#endif

#include "dlgpreferences.h"
#include "dlgprefsound.h"
#include "controllers/dlgprefcontrollers.h"
#include "dlgpreflibrary.h"
#include "dlgprefcontrols.h"
#include "dlgprefwaveform.h"
#include "dlgprefautodj.h"
#include "dlgprefeq.h"
#include "dlgprefcrossfader.h"
#include "dlgprefrecord.h"
#include "dlgprefreplaygain.h"
#include "dlgprefeffects.h"
#include "mixxx.h"
#include "controllers/controllermanager.h"
#include "skin/skinloader.h"
#include "library/library.h"

DlgPreferences::DlgPreferences(MixxxMainWindow * mixxx, SkinLoader* pSkinLoader,
                               SoundManager * soundman, PlayerManager* pPlayerManager,
                               ControllerManager * controllers, VinylControlManager *pVCManager,
                               EffectsManager* pEffectsManager,
                               ConfigObject<ConfigValue>* pConfig, Library *pLibrary)
        : m_pConfig(pConfig),
          m_pageSizeHint(QSize(0, 0)),
          m_preferencesUpdated(ConfigKey("[Preferences]", "updated"), false) {
    setupUi(this);
#if QT_VERSION >= 0x040400 //setHeaderHidden is a qt4.4 addition so having it in the .ui file breaks the build on OpenBSD4.4 (FIXME: revisit this when OpenBSD4.5 comes out?)
    contentsTreeWidget->setHeaderHidden(true);
#endif

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(slotButtonPressed(QAbstractButton*)));


    createIcons();

    while (pagesWidget->count() > 0) {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    // Construct widgets for use in tabs.

#ifdef __VINYLCONTROL__
    // It's important for this to be before the connect for wsound.
    // TODO(rryan) determine why/if this is still true
    m_wvinylcontrol = new DlgPrefVinyl(this, pVCManager, m_pConfig);
    addPageWidget(m_wvinylcontrol);
#else
    m_wnovinylcontrol = new DlgPrefNoVinyl(this, soundman, m_pConfig);
    addPageWidget(m_wnovinylcontrol);
#endif
    m_wsound = new DlgPrefSound(this, soundman, pPlayerManager, m_pConfig);
    addPageWidget(m_wsound);
    m_wlibrary = new DlgPrefLibrary(this, m_pConfig, pLibrary);
    addPageWidget(m_wlibrary);
    connect(m_wlibrary, SIGNAL(scanLibrary()),
            mixxx, SLOT(slotScanLibrary()));
    m_wcontrols = new DlgPrefControls(this, mixxx, pSkinLoader, pPlayerManager, m_pConfig);
    addPageWidget(m_wcontrols);
    m_wwaveform = new DlgPrefWaveform(this, mixxx, m_pConfig);
    addPageWidget(m_wwaveform);
    m_wautodj = new DlgPrefAutoDJ(this, m_pConfig);
    addPageWidget(m_wautodj);
    m_weq = new DlgPrefEQ(this, pEffectsManager, m_pConfig);
    addPageWidget(m_weq);
    m_weffects = new DlgPrefEffects(this, m_pConfig, pEffectsManager);
    addPageWidget(m_weffects);
    m_wcrossfader = new DlgPrefCrossfader(this, m_pConfig);
    addPageWidget(m_wcrossfader);

    m_wbeats = new DlgPrefBeats(this, m_pConfig);
    addPageWidget (m_wbeats);
    m_wkey = new DlgPrefKey(this, m_pConfig);
    addPageWidget(m_wkey);
    m_wreplaygain = new DlgPrefReplayGain(this, m_pConfig);
    addPageWidget(m_wreplaygain);
    m_wrecord = new DlgPrefRecord(this, m_pConfig);
    addPageWidget(m_wrecord);
#ifdef __SHOUTCAST__
    m_wshoutcast = new DlgPrefShoutcast(this, m_pConfig);
    addPageWidget(m_wshoutcast);
#endif
#ifdef __MODPLUG__
    m_wmodplug = new DlgPrefModplug(this, m_pConfig);
    addPageWidget(m_wmodplug);
#endif
    m_wcontrollers = new DlgPrefControllers(this, m_pConfig, controllers,
                                            m_pControllerTreeItem);
    addPageWidget(m_wcontrollers);

    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // If we don't call this explicitly, then we default to showing the sound
    // hardware page but the tree item is not selected.
    showSoundHardwarePage();
}

DlgPreferences::~DlgPreferences() {
    // store last geometry in mixxx.cfg
    m_pConfig->set(ConfigKey("[Preferences]","geometry"),
                   m_geometry.join(","));

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

    m_pLibraryButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pLibraryButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_library.png"));
    m_pLibraryButton->setText(0, tr("Library"));
    m_pLibraryButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pLibraryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pControlsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControlsButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_interface.png"));
    m_pControlsButton->setText(0, tr("Interface"));
    m_pControlsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pControlsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pWaveformButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pWaveformButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_waveforms.png"));
    m_pWaveformButton->setText(0, tr("Waveforms"));
    m_pWaveformButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pWaveformButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pAutoDJButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pAutoDJButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_autodj.png"));
    m_pAutoDJButton->setText(0, tr("Auto DJ"));
    m_pAutoDJButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pAutoDJButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEqButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEqButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_equalizers.png"));
    m_pEqButton->setText(0, tr("Equalizers"));
    m_pEqButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEqButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEffectsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEffectsButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_effects.png"));
    m_pEffectsButton->setText(0, tr("Effects"));
    m_pEffectsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEffectsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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


    m_pBeatDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pBeatDetectionButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
    m_pBeatDetectionButton->setText(0, tr("Beat Detection"));
    m_pBeatDetectionButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pBeatDetectionButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pKeyDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pKeyDetectionButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_keydetect.png"));
    m_pKeyDetectionButton->setText(0, tr("Key Detection"));
    m_pKeyDetectionButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pKeyDetectionButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
    } else if (current == m_pLibraryButton) {
        switchToPage(m_wlibrary);
    } else if (current == m_pControlsButton) {
        switchToPage(m_wcontrols);
    } else if (current == m_pWaveformButton) {
        switchToPage(m_wwaveform);
    } else if (current == m_pAutoDJButton) {
        switchToPage(m_wautodj);
    } else if (current == m_pEqButton) {
        switchToPage(m_weq);
    } else if (current == m_pEffectsButton) {
        switchToPage(m_weffects);
    } else if (current == m_pCrossfaderButton) {
        switchToPage(m_wcrossfader);
    } else if (current == m_pRecordingButton) {
        switchToPage(m_wrecord);
    } else if (current == m_pBeatDetectionButton) {
        switchToPage(m_wbeats);
    } else if (current == m_pKeyDetectionButton) {
        switchToPage(m_wkey);
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
        // Do nothing. m_wcontrollers handled this click.
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
    //
    // Read last geometry (size and position) of preferences panel
    // Bug#1299949
    //
    // init m_geometry
    if (m_geometry.length() < 4) {
        // load default values (optimum size)
        QRect defaultGeometry = getDefaultGeometry();
        QString defaultGeometryStr = QString("%1,%2,%3,%4")
                                          .arg(defaultGeometry.left())
                                            .arg(defaultGeometry.top())
                                            .arg(defaultGeometry.width())
                                            .arg(defaultGeometry.height());

        // get last geometry OR use default values from
        m_geometry = m_pConfig->getValueString(
                    ConfigKey("[Preferences]", "geometry"),
                    defaultGeometryStr).split(",");
    }

    // Update geometry with last values
    setGeometry(m_geometry[0].toInt(),  // x position
                m_geometry[1].toInt(),  // y position
                m_geometry[2].toInt(),  // width
                m_geometry[3].toInt()); // heigth

    // Notify children that we are about to show.
    emit(showDlg());
}

void DlgPreferences::slotButtonPressed(QAbstractButton* pButton) {
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(pButton);
    DlgPreferencePage* pCurrentPage = currentPage();
    switch (role) {
        case QDialogButtonBox::ResetRole:
            // Only reset to defaults on the current page.
            if (pCurrentPage != NULL) {
                pCurrentPage->slotResetToDefaults();
            }
            break;
        case QDialogButtonBox::ApplyRole:
            // Only apply settings on the current page.
            if (pCurrentPage != NULL) {
                pCurrentPage->slotApply();
            }
            break;
        case QDialogButtonBox::AcceptRole:
            emit(applyPreferences());
            accept();
            break;
        case QDialogButtonBox::RejectRole:
            emit(cancelPreferences());
            reject();
            break;
        default:
            break;
    }
}

void DlgPreferences::addPageWidget(DlgPreferencePage* pWidget) {
    connect(this, SIGNAL(showDlg()),
            pWidget, SLOT(slotShow()));
    connect(this, SIGNAL(closeDlg()),
            pWidget, SLOT(slotHide()));
    connect(this, SIGNAL(showDlg()),
            pWidget, SLOT(slotUpdate()));

    connect(this, SIGNAL(applyPreferences()),
            pWidget, SLOT(slotApply()));
    connect(this, SIGNAL(cancelPreferences()),
            pWidget, SLOT(slotCancel()));
    connect(this, SIGNAL(resetToDefaults()),
            pWidget, SLOT(slotResetToDefaults()));

    QScrollArea* sa = new QScrollArea(pagesWidget);
    sa->setWidgetResizable(true);

    sa->setWidget(pWidget);
    pagesWidget->addWidget(sa);

    int iframe = 2 * sa->frameWidth();
    m_pageSizeHint = m_pageSizeHint.expandedTo(
            pWidget->sizeHint()+QSize(iframe, iframe));

}

DlgPreferencePage* DlgPreferences::currentPage() {
    QObject* pObject = pagesWidget->currentWidget();
    for (int i = 0; i < 2; ++i) {
        if (pObject == NULL) {
            return NULL;
        }
        QObjectList children = pObject->children();
        if (children.isEmpty()) {
            return NULL;
        }
        pObject = children[0];
    }
    return dynamic_cast<DlgPreferencePage*>(pObject);
}

void DlgPreferences::removePageWidget(DlgPreferencePage* pWidget) {
    pagesWidget->removeWidget(pWidget->parentWidget()->parentWidget());
}

void DlgPreferences::expandTreeItem(QTreeWidgetItem* pItem) {
    contentsTreeWidget->expandItem(pItem);
}

void DlgPreferences::switchToPage(DlgPreferencePage* pWidget) {
    pagesWidget->setCurrentWidget(pWidget->parentWidget()->parentWidget());
}

void DlgPreferences::moveEvent(QMoveEvent* e) {
    if (m_geometry.length() == 4) {
        m_geometry[0] = QString::number(e->pos().x());
        m_geometry[1] = QString::number(e->pos().y());
    }
}

void DlgPreferences::resizeEvent(QResizeEvent* e) {
    if (m_geometry.length() == 4) {
        m_geometry[2] = QString::number(e->size().width());
        m_geometry[3] = QString::number(e->size().height());
    }
}

QRect DlgPreferences::getDefaultGeometry() {
    QSize optimumSize;
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

    return optimumRect;
}
