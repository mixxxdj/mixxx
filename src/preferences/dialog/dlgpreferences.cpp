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
#include <QMoveEvent>
#include <QResizeEvent>

#include "preferences/dialog/dlgpreferences.h"

#include "preferences/dialog/dlgprefsound.h"
#include "preferences/dialog/dlgpreflibrary.h"
#include "controllers/dlgprefcontrollers.h"

#ifdef __VINYLCONTROL__
#include "preferences/dialog/dlgprefvinyl.h"
#else
#include "preferences/dialog/dlgprefnovinyl.h"
#endif

#include "preferences/dialog/dlgprefinterface.h"
#include "preferences/dialog/dlgprefwaveform.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/dialog/dlgprefeq.h"
#include "preferences/dialog/dlgprefcrossfader.h"
#ifdef __LILV__
#include "preferences/dialog/dlgpreflv2.h"
#endif /* __LILV__ */
#include "preferences/dialog/dlgprefeffects.h"
#include "preferences/dialog/dlgprefautodj.h"

#ifdef __BROADCAST__
#include "preferences/dialog/dlgprefbroadcast.h"
#endif

#include "preferences/dialog/dlgprefrecord.h"
#include "preferences/dialog/dlgprefbeats.h"
#include "preferences/dialog/dlgprefkey.h"
#include "preferences/dialog/dlgprefreplaygain.h"

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif

#include "mixxx.h"
#include "controllers/controllermanager.h"
#include "skin/skinloader.h"
#include "library/library.h"

DlgPreferences::DlgPreferences(MixxxMainWindow * mixxx, SkinLoader* pSkinLoader,
                               SoundManager * soundman, PlayerManager* pPlayerManager,
                               ControllerManager * controllers, VinylControlManager *pVCManager,
                               LV2Backend* pLV2Backend,
                               EffectsManager* pEffectsManager,
                               SettingsManager* pSettingsManager,
                               Library *pLibrary)
        : m_pConfig(pSettingsManager->settings()),
          m_pageSizeHint(QSize(0, 0)) {
#ifndef __LILV__
    Q_UNUSED(pLV2Backend);
#endif /* __LILV__ */
    setupUi(this);
    contentsTreeWidget->setHeaderHidden(true);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(slotButtonPressed(QAbstractButton*)));

    createIcons();

    while (pagesWidget->count() > 0) {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    // Construct widgets for use in tabs.
    m_soundPage = new DlgPrefSound(this, soundman, pPlayerManager, m_pConfig);
    addPageWidget(m_soundPage);
    m_libraryPage = new DlgPrefLibrary(this, m_pConfig, pLibrary);
    addPageWidget(m_libraryPage);
    connect(m_libraryPage, SIGNAL(scanLibrary()),
            pLibrary, SLOT(scan()));
    m_controllersPage = new DlgPrefControllers(this, m_pConfig, controllers,
                                            m_pControllerTreeItem);
    addPageWidget(m_controllersPage);

#ifdef __VINYLCONTROL__
    // It's important for this to be before the connect for wsound.
    // TODO(rryan) determine why/if this is still true
    m_vinylControlPage = new DlgPrefVinyl(this, pVCManager, m_pConfig);
    addPageWidget(m_vinylControlPage);
#else
    m_noVinylControlPage = new DlgPrefNoVinyl(this, soundman, m_pConfig);
    addPageWidget(m_noVinylControlPage);
#endif

    m_interfacePage = new DlgPrefInterface(this, mixxx, pSkinLoader, m_pConfig);
    addPageWidget(m_interfacePage);
    m_waveformPage = new DlgPrefWaveform(this, mixxx, m_pConfig, pLibrary);
    addPageWidget(m_waveformPage);
    m_deckPage = new DlgPrefDeck(this, mixxx, pPlayerManager, m_pConfig);
    addPageWidget(m_deckPage);
    m_equalizerPage = new DlgPrefEQ(this, pEffectsManager, m_pConfig);
    addPageWidget(m_equalizerPage);
    m_crossfaderPage = new DlgPrefCrossfader(this, m_pConfig);
    addPageWidget(m_crossfaderPage);
    m_effectsPage = new DlgPrefEffects(this, m_pConfig, pEffectsManager);
    addPageWidget(m_effectsPage);
#ifdef __LILV__
    m_lv2Page = new DlgPrefLV2(this, pLV2Backend, m_pConfig, pEffectsManager);
    addPageWidget(m_lv2Page);
#endif /* __LILV__ */
    m_autoDjPage = new DlgPrefAutoDJ(this, m_pConfig);
    addPageWidget(m_autoDjPage);

#ifdef __BROADCAST__
    m_broadcastingPage = new DlgPrefBroadcast(this,
        pSettingsManager->broadcastSettings());
    addPageWidget(m_broadcastingPage);
#endif

    m_recordingPage = new DlgPrefRecord(this, m_pConfig);
    addPageWidget(m_recordingPage);

    m_beatgridPage = new DlgPrefBeats(this, m_pConfig);
    addPageWidget (m_beatgridPage);

    m_musicalKeyPage = new DlgPrefKey(this, m_pConfig);
    addPageWidget(m_musicalKeyPage);

    m_replayGainPage = new DlgPrefReplayGain(this, m_pConfig);
    addPageWidget(m_replayGainPage);

#ifdef __MODPLUG__
    m_modplugPage = new DlgPrefModplug(this, m_pConfig);
    addPageWidget(m_modplugPage);
#endif

    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // If we don't call this explicitly, then we default to showing the sound
    // hardware page but the tree item is not selected.
    showSoundHardwarePage();
}

DlgPreferences::~DlgPreferences() {
    // store last geometry in mixxx.cfg
    if (m_geometry.size() == 4) {
        m_pConfig->set(ConfigKey("[Preferences]","geometry"),
                       m_geometry.join(","));
    }

    // Need to explicitly delete rather than relying on child auto-deletion
    // because otherwise the QStackedWidget will delete the controller
    // preference pages (and DlgPrefControllers dynamically generates and
    // deletes them).
    delete m_controllersPage;
}

void DlgPreferences::createIcons() {
    m_pSoundButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pSoundButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_soundhardware.svg"));
    m_pSoundButton->setText(0, tr("Sound Hardware"));
    m_pSoundButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pSoundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pLibraryButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pLibraryButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_library.svg"));
    m_pLibraryButton->setText(0, tr("Library"));
    m_pLibraryButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pLibraryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pControllerTreeItem = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControllerTreeItem->setIcon(0, QIcon(":/images/preferences/ic_preferences_controllers.svg"));
    m_pControllerTreeItem->setText(0, tr("Controllers"));
    m_pControllerTreeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pControllerTreeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __VINYLCONTROL__
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    //QT screws up my nice vinyl svg for some reason, so we'll use a PNG version
    //instead...
    m_pVinylControlButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_vinyl.svg"));
    m_pVinylControlButton->setText(0, tr("Vinyl Control"));
    m_pVinylControlButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pVinylControlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#else
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    //QT screws up my nice vinyl svg for some reason, so we'll use a PNG version
    //instead...
    m_pVinylControlButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_vinyl.svg"));
    m_pVinylControlButton->setText(0, tr("Vinyl Control"));
    m_pVinylControlButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pVinylControlButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

    m_pInterfaceButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pInterfaceButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_interface.svg"));
    m_pInterfaceButton->setText(0, tr("Interface"));
    m_pInterfaceButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pInterfaceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pWaveformButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pWaveformButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_waveforms.svg"));
    m_pWaveformButton->setText(0, tr("Waveforms"));
    m_pWaveformButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pWaveformButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pDecksButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pDecksButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_decks.svg"));
    m_pDecksButton->setText(0, tr("Decks"));
    m_pDecksButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pDecksButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEqButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEqButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_equalizers.svg"));
    m_pEqButton->setText(0, tr("Equalizers"));
    m_pEqButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEqButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pCrossfaderButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pCrossfaderButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_crossfader.svg"));
    m_pCrossfaderButton->setText(0, tr("Crossfader"));
    m_pCrossfaderButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pCrossfaderButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pEffectsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pEffectsButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_effects.svg"));
    m_pEffectsButton->setText(0, tr("Effects"));
    m_pEffectsButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pEffectsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __LILV__
    m_pLV2Button = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pLV2Button->setIcon(0, QIcon(":/images/preferences/ic_preferences_lv2.svg"));
    m_pLV2Button->setText(0, tr("LV2 Plugins"));
    m_pLV2Button->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pLV2Button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif /* __LILV__ */

    m_pAutoDJButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pAutoDJButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_autodj.svg"));
    m_pAutoDJButton->setText(0, tr("Auto DJ"));
    m_pAutoDJButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pAutoDJButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __BROADCAST__
    m_pBroadcastButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pBroadcastButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_broadcast.svg"));
    m_pBroadcastButton->setText(0, tr("Live Broadcasting"));
    m_pBroadcastButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pBroadcastButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

    m_pRecordingButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pRecordingButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_recording.svg"));
    m_pRecordingButton->setText(0, tr("Recording"));
    m_pRecordingButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pRecordingButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pBeatDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pBeatDetectionButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_bpmdetect.svg"));
    m_pBeatDetectionButton->setText(0, tr("Beat Detection"));
    m_pBeatDetectionButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pBeatDetectionButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pKeyDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pKeyDetectionButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_keydetect.svg"));
    m_pKeyDetectionButton->setText(0, tr("Key Detection"));
    m_pKeyDetectionButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pKeyDetectionButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    m_pReplayGainButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pReplayGainButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_replaygain.svg"));
    m_pReplayGainButton->setText(0, tr("Normalization"));
    m_pReplayGainButton->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    m_pReplayGainButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __MODPLUG__
    m_pModplugButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pModplugButton->setIcon(0, QIcon(":/images/preferences/ic_preferences_modplug.svg"));
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
        switchToPage(m_soundPage);
    } else if (current == m_pLibraryButton) {
        switchToPage(m_libraryPage);
    } else if (m_controllersPage->handleTreeItemClick(current)) {
        // Do nothing. m_controllersPage handled this click.
#ifdef __VINYLCONTROL__
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_vinylControlPage);
#else
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_noVinylControlPage);
#endif
    } else if (current == m_pInterfaceButton) {
        switchToPage(m_interfacePage);
    } else if (current == m_pWaveformButton) {
        switchToPage(m_waveformPage);
    } else if (current == m_pDecksButton) {
        switchToPage(m_deckPage);
    } else if (current == m_pEqButton) {
        switchToPage(m_equalizerPage);
    } else if (current == m_pCrossfaderButton) {
        switchToPage(m_crossfaderPage);
    } else if (current == m_pEffectsButton) {
        switchToPage(m_effectsPage);
#ifdef __LILV__
    } else if (current == m_pLV2Button) {
        switchToPage(m_lv2Page);
#endif /* __LILV__ */
    } else if (current == m_pAutoDJButton) {
        switchToPage(m_autoDjPage);
#ifdef __BROADCAST__
    } else if (current == m_pBroadcastButton) {
        switchToPage(m_broadcastingPage);
#endif
    } else if (current == m_pRecordingButton) {
        switchToPage(m_recordingPage);
    } else if (current == m_pBeatDetectionButton) {
        switchToPage(m_beatgridPage);
    } else if (current == m_pKeyDetectionButton) {
        switchToPage(m_musicalKeyPage);
    } else if (current == m_pReplayGainButton) {
        switchToPage(m_replayGainPage);
#ifdef __MODPLUG__
    } else if (current == m_pModplugButton) {
        switchToPage(m_modplugPage);
#endif
    }
}

void DlgPreferences::showSoundHardwarePage() {
    switchToPage(m_soundPage);
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
}

void DlgPreferences::onShow() {
    // init m_geometry
    if (m_geometry.length() < 4) {
        // load default values (optimum size)
        m_geometry = m_pConfig->getValue(
                    ConfigKey("[Preferences]", "geometry")).split(",");
        if (m_geometry.length() < 4) {
            // Warning! geometry does NOT include the frame/title.
            QRect defaultGeometry = getDefaultGeometry();
            m_geometry.clear();
            m_geometry.append(QString::number(defaultGeometry.left()));
            m_geometry.append(QString::number(defaultGeometry.top()));
            m_geometry.append(QString::number(defaultGeometry.width()));
            m_geometry.append(QString::number(defaultGeometry.height()));
        }
    }
    int newX = m_geometry[0].toInt();
    int newY = m_geometry[1].toInt();
    newX = std::max(0, std::min(newX, QApplication::desktop()->screenGeometry().width()- m_geometry[2].toInt()));
    newY = std::max(0, std::min(newY, QApplication::desktop()->screenGeometry().height() - m_geometry[3].toInt()));
    m_geometry[0] = QString::number(newX);
    m_geometry[1] = QString::number(newY);

    // Update geometry with last values
#ifdef __WINDOWS__
    resize(m_geometry[2].toInt(), m_geometry[3].toInt());
#else
    // On linux, when the window is opened for the first time by the window manager, QT does not have
    // information about the frame size so the offset is zero. As such, the first time it opens the window
    // does not include the offset, so it is moved from the last position it had.
    // Excluding the offset from the saved value tries to fix that.
    int offsetX = geometry().left() - frameGeometry().left();
    int offsetY = geometry().top() - frameGeometry().top();
    newX += offsetX;
    newY += offsetY;
    setGeometry(newX,  // x position
                newY,  // y position
                m_geometry[2].toInt(),  // width
                m_geometry[3].toInt()); // height
#endif
    // Move is also needed on linux.
    move(newX, newY);

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
#ifdef __WINDOWS__
    Q_UNUSED(e);
        m_geometry[0] = QString::number(frameGeometry().left());
        m_geometry[1] = QString::number(frameGeometry().top());
#else
        // Warning! geometry does NOT include the frame/title.
        int offsetX = geometry().left() - frameGeometry().left();
        int offsetY = geometry().top() - frameGeometry().top();
        m_geometry[0] = QString::number(e->pos().x() - offsetX);
        m_geometry[1] = QString::number(e->pos().y() - offsetY);
#endif
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
