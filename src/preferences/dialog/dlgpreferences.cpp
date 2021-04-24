#include "preferences/dialog/dlgpreferences.h"

#include <QDesktopServices>
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
#endif // __VINYLCONTROL__

#include "preferences/dialog/dlgprefcolors.h"
#include "preferences/dialog/dlgprefcrossfader.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/dialog/dlgprefeq.h"
#include "preferences/dialog/dlgprefinterface.h"
#include "preferences/dialog/dlgprefwaveform.h"
#ifdef __LILV__
#include "preferences/dialog/dlgpreflv2.h"
#endif // __LILV__
#include "preferences/dialog/dlgprefeffects.h"
#include "preferences/dialog/dlgprefautodj.h"

#ifdef __BROADCAST__
#include "preferences/dialog/dlgprefbroadcast.h"
#endif // __BROADCAST__

#include "preferences/dialog/dlgprefrecord.h"
#include "preferences/dialog/dlgprefbeats.h"
#include "preferences/dialog/dlgprefkey.h"
#include "preferences/dialog/dlgprefreplaygain.h"

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif // __MODPLUG__

#include "controllers/controllermanager.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "skin/skinloader.h"
#include "util/color/color.h"
#include "util/widgethelper.h"

DlgPreferences::DlgPreferences(
        MixxxMainWindow* mixxx,
        std::shared_ptr<SkinLoader> pSkinLoader,
        std::shared_ptr<SoundManager> pSoundManager,
        std::shared_ptr<PlayerManager> pPlayerManager,
        std::shared_ptr<ControllerManager> pControllerManager,
        std::shared_ptr<VinylControlManager> pVCManager,
        LV2Backend* pLV2Backend,
        std::shared_ptr<EffectsManager> pEffectsManager,
        std::shared_ptr<SettingsManager> pSettingsManager,
        std::shared_ptr<Library> pLibrary)
        : m_allPages(),
          m_pConfig(pSettingsManager->settings()),
          m_pageSizeHint(QSize(0, 0)) {
#ifndef __VINYLCONTROL__
    Q_UNUSED(pVCManager);
#endif // __VINYLCONTROL__
#ifndef __LILV__
    Q_UNUSED(pLV2Backend);
#endif // __LILV__
    Q_UNUSED(pPlayerManager);
    setupUi(this);
    contentsTreeWidget->setHeaderHidden(true);

    connect(buttonBox,
            QOverload<QAbstractButton*>::of(&QDialogButtonBox::clicked),
            this,
            &DlgPreferences::slotButtonPressed);

    connect(contentsTreeWidget,
            &QTreeWidget::currentItemChanged,
            this,
            &DlgPreferences::changePage);

    while (pagesWidget->count() > 0) {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }

    // Check the text color of the palette for whether to use dark or light icons
    if (!Color::isDimColor(palette().text().color())) {
        m_iconsPath.setPath(":/images/preferences/light/");
    } else {
        m_iconsPath.setPath(":/images/preferences/dark/");
    }

    // Construct widgets for use in tabs.
    m_soundPage = PreferencesPage(
            new DlgPrefSound(this, pSoundManager, m_pConfig),
            new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type));
    addPageWidget(m_soundPage,
            tr("Sound Hardware"),
            "ic_preferences_soundhardware.svg");

    DlgPrefLibrary* plibraryPage = new DlgPrefLibrary(this, m_pConfig, pLibrary);
    connect(plibraryPage,
            &DlgPrefLibrary::scanLibrary,
            pLibrary->trackCollections(),
            &TrackCollectionManager::startLibraryScan);
    addPageWidget(PreferencesPage(plibraryPage,
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Library"),
            "ic_preferences_library.svg");

    QTreeWidgetItem* pControllerRootItem =
            new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type);
    m_pControllersDlg = new DlgPrefControllers(
            this, m_pConfig, pControllerManager, pControllerRootItem);
    addPageWidget(PreferencesPage(m_pControllersDlg,
                          pControllerRootItem),
            tr("Controllers"),
            "ic_preferences_controllers.svg");

#ifdef __VINYLCONTROL__
    // It's important for this to be before the connect for wsound.
    // TODO(rryan) determine why/if this is still true
    addPageWidget(PreferencesPage(
                          new DlgPrefVinyl(this, pVCManager, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Vinyl Control"),
            "ic_preferences_vinyl.svg");
#endif // __VINYLCONTROL__

    addPageWidget(PreferencesPage(
                          new DlgPrefInterface(this, mixxx, pSkinLoader, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Interface"),
            "ic_preferences_interface.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefWaveform(this, mixxx, m_pConfig, pLibrary),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Waveforms"),
            "ic_preferences_waveforms.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefColors(this, m_pConfig, pLibrary),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Colors"),
            "ic_preferences_colors.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefDeck(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Decks"),
            "ic_preferences_decks.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefEQ(this, pEffectsManager, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Equalizers"),
            "ic_preferences_equalizers.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefCrossfader(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Crossfader"),
            "ic_preferences_crossfader.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefEffects(this, m_pConfig, pEffectsManager),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Effects"),
            "ic_preferences_effects.svg");

#ifdef __LILV__
    addPageWidget(PreferencesPage(
                          new DlgPrefLV2(this, pLV2Backend, m_pConfig, pEffectsManager),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("LV2 Plugins"),
            "ic_preferences_lv2.svg");
#endif // __LILV__

    addPageWidget(PreferencesPage(
                          new DlgPrefAutoDJ(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Auto DJ"),
            "ic_preferences_autodj.svg");

#ifdef __BROADCAST__
    addPageWidget(PreferencesPage(
                          new DlgPrefBroadcast(this, pSettingsManager->broadcastSettings()),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Live Broadcasting"),
            "ic_preferences_broadcast.svg");
#endif // __BROADCAST__

    addPageWidget(PreferencesPage(
                          new DlgPrefRecord(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Recording"),
            "ic_preferences_recording.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefBeats(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Beat Detection"),
            "ic_preferences_bpmdetect.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefKey(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Key Detection"),
            "ic_preferences_keydetect.svg");
    addPageWidget(PreferencesPage(
                          new DlgPrefReplayGain(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Normalization"),
            "ic_preferences_replaygain.svg");

#ifdef __MODPLUG__
    addPageWidget(PreferencesPage(
                          new DlgPrefModplug(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Modplug Decoder"),
            "ic_preferences_modplug.svg");
#endif // __MODPLUG__

    // Find accept and apply buttons
    const auto buttons = buttonBox->buttons();
    for (QAbstractButton* button : buttons) {
        QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
        if (role == QDialogButtonBox::ButtonRole::ApplyRole) {
            m_pApplyButton = button;
        } else if (role == QDialogButtonBox::ButtonRole::AcceptRole) {
            m_pAcceptButton = button;
        }
    }

    labelWarning->hide();
    labelWarningIcon->hide();
    labelWarning->setText(tr(
            "<font color='#BB0000'><b>Some preferences pages have errors. "
            "To apply the changes please first fix the issues.</b></font>"));
    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    labelWarningIcon->setPixmap(icon.pixmap(16));

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

    // When DlgPrefControllers is deleted it manually deletes the controller tree items,
    // which makes QTreeWidgetItem trigger this signal. If we don't disconnect,
    // &DlgPreferences::changePage iterates on the PreferencesPage instances in m_allPages,
    // but the pDlg objects of the controller items are already destroyed by DlgPrefControllers,
    // which causes a crash when accessed.
    disconnect(contentsTreeWidget, &QTreeWidget::currentItemChanged, this, &DlgPreferences::changePage);
    // Need to explicitly delete rather than relying on child auto-deletion
    // because otherwise the QStackedWidget will delete the controller
    // preference pages (and DlgPrefControllers dynamically generates and
    // deletes them).
    delete m_pControllersDlg;
}

void DlgPreferences::changePage(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious) {
    if (!pCurrent) {
        pCurrent = pPrevious;
    }

    if (m_pControllersDlg->handleTreeItemClick(pCurrent)) {
        // Do nothing. m_controllersPage handled this click.
        return;
    }

    for (PreferencesPage page : qAsConst(m_allPages)) {
        if (pCurrent == page.pTreeItem) {
            switchToPage(page.pDlg);
            break;
        }
    }
}

void DlgPreferences::showSoundHardwarePage() {
    switchToPage(m_soundPage.pDlg);
    contentsTreeWidget->setCurrentItem(m_soundPage.pTreeItem);
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
#else  // __WINDOWS__
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
#endif // __LINUX__ / __MACOS__
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

void DlgPreferences::addPageWidget(PreferencesPage page,
        const QString& pageTitle,
        const QString& iconFile) {
    // Configure the tree button linked to the page
    page.pTreeItem->setIcon(0, QIcon(m_iconsPath.filePath(iconFile)));
    page.pTreeItem->setText(0, pageTitle);
    page.pTreeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    page.pTreeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(this, &DlgPreferences::showDlg, page.pDlg, &DlgPreferencePage::slotShow);
    connect(this, &DlgPreferences::closeDlg, page.pDlg, &DlgPreferencePage::slotHide);
    connect(this, &DlgPreferences::showDlg, page.pDlg, &DlgPreferencePage::slotUpdate);

    connect(this, &DlgPreferences::applyPreferences, page.pDlg, &DlgPreferencePage::slotApply);
    connect(this, &DlgPreferences::cancelPreferences, page.pDlg, &DlgPreferencePage::slotCancel);
    connect(this,
            &DlgPreferences::resetToDefaults,
            page.pDlg,
            &DlgPreferencePage::slotResetToDefaults);

    // Add a new scroll area to the stacked pages widget containing the page
    QScrollArea* sa = new QScrollArea(pagesWidget);
    sa->setWidgetResizable(true);

    sa->setWidget(page.pDlg);
    pagesWidget->addWidget(sa);
    m_allPages.append(page);

    int iframe = 2 * sa->frameWidth();
    m_pageSizeHint = m_pageSizeHint.expandedTo(
            page.pDlg->sizeHint() + QSize(iframe, iframe));
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
