#include "preferences/dialog/dlgpreferences.h"

#include <QDialog>
#include <QEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QtGlobal>

#include "controllers/dlgprefcontrollers.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgpreferences.cpp"
#include "preferences/dialog/dlgpreflibrary.h"
#include "preferences/dialog/dlgprefsound.h"
#include "util/color/color.h"
#include "util/desktophelper.h"
#include "util/widgethelper.h"

#ifdef __VINYLCONTROL__
#include "preferences/dialog/dlgprefvinyl.h"
#endif // __VINYLCONTROL__

#include "preferences/dialog/dlgprefautodj.h"
#include "preferences/dialog/dlgprefcolors.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/dialog/dlgprefeffects.h"
#include "preferences/dialog/dlgprefinterface.h"
#include "preferences/dialog/dlgprefmixer.h"
#include "preferences/dialog/dlgprefwaveform.h"

#ifdef __BROADCAST__
#include "preferences/dialog/dlgprefbroadcast.h"
#endif // __BROADCAST__

#include "preferences/dialog/dlgprefbeats.h"
#include "preferences/dialog/dlgprefkey.h"
#include "preferences/dialog/dlgprefosc.h"
#include "preferences/dialog/dlgprefrecord.h"
#include "preferences/dialog/dlgprefreplaygain.h"

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif // __MODPLUG__

#ifdef Q_OS_MACOS
#include "util/darkappearance.h"
#endif

DlgPreferences::DlgPreferences(
        std::shared_ptr<mixxx::ScreensaverManager> pScreensaverManager,
        std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
        std::shared_ptr<SoundManager> pSoundManager,
        std::shared_ptr<ControllerManager> pControllerManager,
        std::shared_ptr<VinylControlManager> pVCManager,
        std::shared_ptr<EffectsManager> pEffectsManager,
        std::shared_ptr<SettingsManager> pSettingsManager,
        std::shared_ptr<Library> pLibrary)
        : m_allPages(),
          m_pConfig(pSettingsManager->settings()),
          m_pageSizeHint(QSize(0, 0)) {
    setupUi(this);
    fixSliderStyle();
    contentsTreeWidget->setHeaderHidden(true);

    // Add '&' to default button labels to always have Alt shortcuts, indpependent
    // of operating system.
    //: Preferences standard buttons: consider the other buttons to choose a unique Alt hotkey (&)
    buttonBox->button(QDialogButtonBox::Help)->setText(tr("&Help"));
    //: Preferences standard buttons: consider the other buttons to choose a unique Alt hotkey (&)
    buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("&Restore Defaults"));
    //: Preferences standard buttons: consider the other buttons to choose a unique Alt hotkey (&)
    buttonBox->button(QDialogButtonBox::Apply)->setText(tr("&Apply"));
    //: Preferences standard buttons: consider the other buttons to choose a unique Alt hotkey (&)
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    //: Preferences standard buttons: consider the other buttons to choose a unique Alt hotkey (&)
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Ok"));

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
            pLibrary->trackCollectionManager(),
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

    DlgPrefInterface* pInterfacePage = new DlgPrefInterface(this,
            pScreensaverManager,
            pSkinLoader,
            m_pConfig);
    connect(pInterfacePage,
            &DlgPrefInterface::tooltipModeChanged,
            this,
            &DlgPreferences::tooltipModeChanged);
    connect(pInterfacePage,
            &DlgPrefInterface::reloadUserInterface,
            this,
            &DlgPreferences::reloadUserInterface,
            Qt::DirectConnection);
    connect(pInterfacePage,
            &DlgPrefInterface::menuBarAutoHideChanged,
            this,
            &DlgPreferences::menuBarAutoHideChanged,
            Qt::DirectConnection);
    addPageWidget(PreferencesPage(pInterfacePage,
                          new QTreeWidgetItem(
                                  contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Interface"),
            "ic_preferences_interface.svg");

    // ugly proxy for determining whether this is being instantiated for QML or legacy QWidgets GUI
    if (pSkinLoader) {
        addPageWidget(PreferencesPage(
                              new DlgPrefWaveform(this, m_pConfig, pLibrary),
                              new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
                tr("Waveforms"),
                "ic_preferences_waveforms.svg");
    }

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
                          new DlgPrefMixer(this, pEffectsManager, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Mixer"),
            "ic_preferences_crossfader.svg");

    addPageWidget(PreferencesPage(
                          new DlgPrefEffects(this, m_pConfig, pEffectsManager),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("Effects"),
            "ic_preferences_effects.svg");

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
                          new DlgPrefOsc(this, m_pConfig),
                          new QTreeWidgetItem(contentsTreeWidget, QTreeWidgetItem::Type)),
            tr("OSC"),
            "ic_preferences_broadcast.svg");

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
        m_pConfig->set(ConfigKey("[Preferences]", "geometry"),
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

    for (PreferencesPage page : std::as_const(m_allPages)) {
        if (pCurrent == page.pTreeItem) {
            switchToPage(pCurrent->text(0), page.pDlg);
            break;
        }
    }
}

void DlgPreferences::showSoundHardwarePage() {
    switchToPage(m_soundPage.pTreeItem->text(0), m_soundPage.pDlg);
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
    return QWidget::eventFilter(o, e);
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
                                      ConfigKey("[Preferences]", "geometry"))
                             .split(",");
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
    int newWidth = m_geometry[2].toInt();
    int newHeight = m_geometry[3].toInt();

    const QScreen* const pScreen = mixxx::widgethelper::getScreen(*this);
    QRect screenAvailableGeometry;
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        qWarning() << "Assuming screen size of 800x600px.";
        screenAvailableGeometry = QRect(0, 0, 800, 600);
    }
    else {
        screenAvailableGeometry = pScreen->availableGeometry();
    }

    // Make sure the entire window is visible on screen and is not occluded by taskbar
    // Note: Window geometry excludes window decoration
    int windowDecorationWidth = frameGeometry().width() - geometry().width();
    int windowDecorationHeight = frameGeometry().height() - geometry().height();
    if (windowDecorationWidth <= 0) {
        windowDecorationWidth = 2;
    }
    if (windowDecorationHeight <= 0) {
        windowDecorationHeight = 30;
    }
    int availableWidth = screenAvailableGeometry.width() - windowDecorationWidth;
    int availableHeight = screenAvailableGeometry.height() - windowDecorationHeight;
    newWidth = std::min(newWidth, availableWidth);
    newHeight = std::min(newHeight, availableHeight);
    int minX = screenAvailableGeometry.x();
    int minY = screenAvailableGeometry.y();
    int maxX = screenAvailableGeometry.x() + availableWidth - newWidth;
    int maxY = screenAvailableGeometry.y() + availableHeight - newHeight;
    newX = std::clamp(newX, minX, maxX);
    newY = std::clamp(newY, minY, maxY);
    m_geometry[0] = QString::number(newX);
    m_geometry[1] = QString::number(newY);
    m_geometry[2] = QString::number(newWidth);
    m_geometry[3] = QString::number(newHeight);

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
    setGeometry(newX,   // x position
            newY,       // y position
            newWidth,   // width
            newHeight); // height
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
        emit applyPreferences();
        break;
    case QDialogButtonBox::AcceptRole:
        // Same as Apply but close the dialog
        emit applyPreferences();
        // TODO Unfortunately this will accept() even if DlgPrefSound threw a warning
        // due to inaccessible device(s) or inapplicable samplerate.
        // https://github.com/mixxxdj/mixxx/issues/6077
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
            mixxx::DesktopHelper::openUrl(helpUrl);
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

void DlgPreferences::switchToPage(const QString& pageTitle, DlgPreferencePage* pWidget) {
#ifdef __APPLE__
    // According to Apple's Human Interface Guidelines, settings dialogs have to
    // "Update the window’s title to reflect the currently visible pane."
    // This also solves the problem of the changed in terminology, Settings instead
    // of Preferences, since macOS Ventura.
    setWindowTitle(pageTitle);
#else
    Q_UNUSED(pageTitle);
#endif
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
        optimumSize -= QSize(2, 30);
    } else {
        optimumSize -= (frameSize() - size());
    }

    QSize staticSize = size() - pagesWidget->size();
    optimumSize = optimumSize.boundedTo(staticSize + m_pageSizeHint);

    QRect optimumRect = geometry();
    optimumRect.setSize(optimumSize);

    return optimumRect;
}

void DlgPreferences::fixSliderStyle() {
#ifdef Q_OS_MACOS
    // Only used on macOS where the default slider style has several issues:
    // - the handle is semi-transparent
    // - the slider is higher than the space we give it, which causes that:
    //   - the groove is not correctly centered vertically
    //   - the handle is cut off at the top
    // The style below is based on sliders in the macOS system settings dialogs.
    if (darkAppearance()) {
        setStyleSheet(R"--(
QSlider::handle:horizontal {
    background-color: #8f8c8b; 
    border-radius: 4px;
    width: 8px;
    margin: -8px;
} 
QSlider::handle:horizontal::pressed {
    background-color: #a9a7a7;
}
QSlider::groove:horizontal {
    background: #1e1e1e; 
    height: 4px;
    border-radius: 2px;
    margin-left: 8px; 
    margin-right: 8px;
}
)--");
    } else {
        setStyleSheet(R"--(
QSlider::handle:horizontal {
    background-color: #ffffff;
    border-radius: 4px;
    border: 1px solid #d4d3d3;
    width: 7px;
    margin: -8px;
}
QSlider::handle:horizontal::pressed {
    background-color: #ececec;
}
QSlider::groove:horizontal {
    background: #c6c5c5;
    height: 4px;
    border-radius: 2px;
    margin-left: 8px;
    margin-right: 8px;
}
)--");
    }
#endif // __APPLE__
}
