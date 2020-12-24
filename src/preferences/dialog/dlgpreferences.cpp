#include "preferences/dialog/dlgpreferences.h"

#include <QDialog>
#include <QEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>

#include "controllers/dlgprefcontrollers.h"
#include "moc_dlgpreferences.cpp"
#include "preferences/dialog/dlgpreflibrary.h"
#include "preferences/dialog/dlgprefsound.h"

#ifdef __VINYLCONTROL__
#include "preferences/dialog/dlgprefvinyl.h"
#else /* __VINYLCONTROL__ */
#include "preferences/dialog/dlgprefnovinyl.h"
#endif

#include "preferences/dialog/dlgprefcolors.h"
#include "preferences/dialog/dlgprefcrossfader.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/dialog/dlgprefeq.h"
#include "preferences/dialog/dlgprefinterface.h"
#include "preferences/dialog/dlgprefwaveform.h"
#ifdef __LILV__
#include "preferences/dialog/dlgpreflv2.h"
#endif /* __LILV__ */
#include "preferences/dialog/dlgprefeffects.h"
#include "preferences/dialog/dlgprefautodj.h"

#ifdef __BROADCAST__
#include "preferences/dialog/dlgprefbroadcast.h"
#endif /* __BROADCAST__ */

#include "preferences/dialog/dlgprefrecord.h"
#include "preferences/dialog/dlgprefbeats.h"
#include "preferences/dialog/dlgprefkey.h"
#include "preferences/dialog/dlgprefreplaygain.h"

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif /* __MODPLUG__ */

#include "controllers/controllermanager.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "mixxx.h"
#include "skin/skinloader.h"
#include "util/widgethelper.h"

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
    Q_UNUSED(pPlayerManager);
    setupUi(this);
    contentsTreeWidget->setHeaderHidden(true);

    connect(buttonBox,
            QOverload<QAbstractButton*>::of(&QDialogButtonBox::clicked),
            this,
            &DlgPreferences::slotButtonPressed);

    while (pagesWidget->count() > 0) {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    // Construct widgets for use in tabs:
    // * create the tree view button first (important for 'Controllers' page!)
    // * instantiate preferences page
    m_pSoundButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_soundPage = new DlgPrefSound(this, soundman, m_pConfig);
    addPageWidget(m_soundPage,
            m_pSoundButton,
            tr("Sound Hardware"),
            "soundhardware.svg");

    m_pLibraryButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_libraryPage = new DlgPrefLibrary(this, m_pConfig, pLibrary);
    addPageWidget(m_libraryPage,
            m_pLibraryButton,
            tr("Library"),
            "library.svg");
    connect(m_libraryPage,
            &DlgPrefLibrary::scanLibrary,
            pLibrary->trackCollections(),
            &TrackCollectionManager::startLibraryScan);

    m_pControllersRootButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_controllersPage = new DlgPrefControllers(
            this, m_pConfig, controllers, m_pControllersRootButton);
    addPageWidget(m_controllersPage,
            m_pControllersRootButton,
            tr("Controllers"),
            "controllers.svg");

#ifdef __VINYLCONTROL__
    // It's important for this to be before the connect for wsound.
    // TODO(rryan) determine why/if this is still true
    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_vinylControlPage = new DlgPrefVinyl(this, pVCManager, m_pConfig);
    addPageWidget(m_vinylControlPage,
            m_pVinylControlButton,
            tr("Vinyl Control"),
            "vinyl.svg");
#else /* __VINYLCONTROL__ */

    m_pVinylControlButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_noVinylControlPage = new DlgPrefNoVinyl(this, soundman, m_pConfig);
    addPageWidget(m_noVinylControlPage,
            m_pVinylControlButton,
            tr("Vinyl Control"),
            "vinyl.svg");
#endif

    m_pInterfaceButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_interfacePage = new DlgPrefInterface(this, mixxx, pSkinLoader, m_pConfig);
    addPageWidget(m_interfacePage,
            m_pInterfaceButton,
            tr("Interface"),
            "interface.svg");

    m_pWaveformButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_waveformPage = new DlgPrefWaveform(this, mixxx, m_pConfig, pLibrary);
    addPageWidget(m_waveformPage,
            m_pWaveformButton,
            tr("Waveforms"),
            "waveforms.svg");

    m_pDecksButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_deckPage = new DlgPrefDeck(this, m_pConfig);
    addPageWidget(m_deckPage,
            m_pDecksButton,
            tr("Decks"),
            "decks.svg");

    m_pColorsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_colorsPage = new DlgPrefColors(this, m_pConfig, pLibrary);
    addPageWidget(m_colorsPage,
            m_pColorsButton,
            tr("Colors"),
            "colors.svg");

    m_pEqButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_equalizerPage = new DlgPrefEQ(this, pEffectsManager, m_pConfig);
    addPageWidget(m_equalizerPage,
            m_pEqButton,
            tr("Equalizers"),
            "equalizers.svg");

    m_pCrossfaderButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_crossfaderPage = new DlgPrefCrossfader(this, m_pConfig);
    addPageWidget(m_crossfaderPage,
            m_pCrossfaderButton,
            tr("Crossfader"),
            "crossfader.svg");

    m_pEffectsButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_effectsPage = new DlgPrefEffects(this, m_pConfig, pEffectsManager);
    addPageWidget(m_effectsPage,
            m_pEffectsButton,
            tr("Effects"),
            "effects.svg");

#ifdef __LILV__
    m_pLV2Button = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_lv2Page = new DlgPrefLV2(this, pLV2Backend, m_pConfig, pEffectsManager);
    addPageWidget(m_lv2Page,
            m_pLV2Button,
            tr("LV2 Plugins"),
            "lv2.svg");
#endif /* __LILV__ */

    m_pAutoDJButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_autoDjPage = new DlgPrefAutoDJ(this, m_pConfig);
    addPageWidget(m_autoDjPage,
            m_pAutoDJButton,
            tr("Auto DJ"),
            "autodj.svg");

#ifdef __BROADCAST__
    m_pBroadcastButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_broadcastingPage = new DlgPrefBroadcast(this, pSettingsManager->broadcastSettings());
    addPageWidget(m_broadcastingPage,
            m_pBroadcastButton,
            tr("Live Broadcasting"),
            "broadcast.svg");
#endif /* __BROADCAST__ */

    m_pRecordingButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_recordingPage = new DlgPrefRecord(this, m_pConfig);
    addPageWidget(m_recordingPage,
            m_pRecordingButton,
            tr("Recording"),
            "recording.svg");

    m_pBeatDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_beatgridPage = new DlgPrefBeats(this, m_pConfig);
    addPageWidget(m_beatgridPage,
            m_pBeatDetectionButton,
            tr("Beat Detection"),
            "bpmdetect.svg");

    m_pKeyDetectionButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_musicalKeyPage = new DlgPrefKey(this, m_pConfig);
    addPageWidget(m_musicalKeyPage,
            m_pKeyDetectionButton,
            tr("Key Detection"),
            "keydetect.svg");

    m_pReplayGainButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_replayGainPage = new DlgPrefReplayGain(this, m_pConfig);
    addPageWidget(m_replayGainPage,
            m_pReplayGainButton,
            tr("Normalization"),
            "replaygain.svg");

#ifdef __MODPLUG__
    m_pModplugButton = new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_modplugPage = new DlgPrefModplug(this, m_pConfig);
    addPageWidget(m_modplugPage,
            m_pModplugButton,
            tr("Modplug Decoder"),
            "modplug.svg");
#endif /* __MODPLUG__ */

    connect(contentsTreeWidget,
            &QTreeWidget::currentItemChanged,
            this,
            &DlgPreferences::changePage);

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

void DlgPreferences::changePage(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    if (!current) {
        current = previous;
    }

    if (current == m_pSoundButton) {
        switchToPage(m_soundPage);
    } else if (current == m_pLibraryButton) {
        switchToPage(m_libraryPage);
    } else if (m_controllersPage->handleTreeItemClick(current)) {
        // Do nothing. m_controllersPage handled this click.
#ifdef __VINYLCONTROL__
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_vinylControlPage);
#else /* __VINYLCONTROL__ */
    } else if (current == m_pVinylControlButton) {
        switchToPage(m_noVinylControlPage);
#endif
    } else if (current == m_pInterfaceButton) {
        switchToPage(m_interfacePage);
    } else if (current == m_pWaveformButton) {
        switchToPage(m_waveformPage);
    } else if (current == m_pDecksButton) {
        switchToPage(m_deckPage);
    } else if (current == m_pColorsButton) {
        switchToPage(m_colorsPage);
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
#endif /* __BROADCAST__ */
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
#endif /* __MODPLUG__ */
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
    emit closeDlg();
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

    const QScreen* const pScreen = mixxx::widgethelper::getScreen(*this);
    QSize screenSpace;
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        qWarning() << "Assuming screen size of 800x600px.";
        screenSpace = QSize(800, 600);
    }
    else {
        screenSpace = pScreen->size();
    }
    newX = std::max(0, std::min(newX, screenSpace.width() - m_geometry[2].toInt()));
    newY = std::max(0, std::min(newY, screenSpace.height() - m_geometry[3].toInt()));
    m_geometry[0] = QString::number(newX);
    m_geometry[1] = QString::number(newY);

    // Update geometry with last values
#ifdef __WINDOWS__
    resize(m_geometry[2].toInt(), m_geometry[3].toInt());
#else /* __WINDOWS__ */
    // On linux, when the window is opened for the first time by the window manager,
    // QT does not have information about the frame size so the offset is zero.
    // As such, the first time it opens the window does not include the offset,
    // so it is moved from the last position it had.
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
    emit showDlg();
}

void DlgPreferences::slotButtonPressed(QAbstractButton* pButton) {
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(pButton);
    DlgPreferencePage* pCurrentPage = currentPage();
    switch (role) {
        case QDialogButtonBox::ResetRole:
            // Only reset to defaults on the current page.
            if (pCurrentPage) {
                pCurrentPage->slotResetToDefaults();
            }
            break;
        case QDialogButtonBox::ApplyRole:
            // Only apply settings on the current page.
            if (pCurrentPage) {
                pCurrentPage->slotApply();
            }
            break;
        case QDialogButtonBox::AcceptRole:
            emit applyPreferences();
            accept();
            break;
        case QDialogButtonBox::RejectRole:
            emit cancelPreferences();
            reject();
            break;
        case QDialogButtonBox::HelpRole:
            if (pCurrentPage) {
                QUrl helpUrl = pCurrentPage->helpUrl();
                DEBUG_ASSERT(helpUrl.isValid());
                QDesktopServices::openUrl(helpUrl);
            }
            break;
        default:
            break;
    }
}

void DlgPreferences::addPageWidget(DlgPreferencePage* pWidget,
        QTreeWidgetItem* treeItem,
        const QString& pageTitle,
        const QString& iconFile) {
    // Configure the tree button linked to the page
    treeItem->setIcon(0, QIcon(":/images/preferences/ic_preferences_" + iconFile));
    treeItem->setText(0, pageTitle);
    treeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    treeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(this, &DlgPreferences::showDlg, pWidget, &DlgPreferencePage::slotShow);
    connect(this, &DlgPreferences::closeDlg, pWidget, &DlgPreferencePage::slotHide);
    connect(this, &DlgPreferences::showDlg, pWidget, &DlgPreferencePage::slotUpdate);
    connect(this, &DlgPreferences::applyPreferences, pWidget, &DlgPreferencePage::slotApply);
    connect(this, &DlgPreferences::cancelPreferences, pWidget, &DlgPreferencePage::slotCancel);
    connect(this,
            &DlgPreferences::resetToDefaults,
            pWidget,
            &DlgPreferencePage::slotResetToDefaults);

    // Add a new scroll area containing the page to the stacked pages widget
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
        if (pObject == nullptr) {
            return nullptr;
        }
        QObjectList children = pObject->children();
        if (children.isEmpty()) {
            return nullptr;
        }
        pObject = children[0];
    }
    return qobject_cast<DlgPreferencePage*>(pObject);
}

void DlgPreferences::removePageWidget(DlgPreferencePage* pWidget) {
    pagesWidget->removeWidget(pWidget->parentWidget()->parentWidget());
}

void DlgPreferences::expandTreeItem(QTreeWidgetItem* pItem) {
    contentsTreeWidget->expandItem(pItem);
}

void DlgPreferences::switchToPage(DlgPreferencePage* pWidget) {
    pagesWidget->setCurrentWidget(pWidget->parentWidget()->parentWidget());

    QPushButton* pButton = buttonBox->button(QDialogButtonBox::Help);
    VERIFY_OR_DEBUG_ASSERT(pButton) {
        return;
    }

    if (pWidget->helpUrl().isValid()) {
        pButton->show();
    } else {
        pButton->hide();
    }
}

void DlgPreferences::moveEvent(QMoveEvent* e) {
    if (m_geometry.length() == 4) {
#ifdef __WINDOWS__
    Q_UNUSED(e);
        m_geometry[0] = QString::number(frameGeometry().left());
        m_geometry[1] = QString::number(frameGeometry().top());
#else /* __WINDOWS__ */
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
    adjustSize();
    const auto* const pScreen = mixxx::widgethelper::getScreen(*this);
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        return QRect();
    }
    QSize optimumSize = pScreen->size();

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
